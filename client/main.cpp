#include <Socket.h>
#include <SocketInterface.h>

#include <boost/asio.hpp>
#include <functional>
#include <stdexcept>
#include <string>

#include "Client.h"
#include "spdlog/spdlog.h"
#include "tun_interface.h"

namespace {
std::function<void(int)> shutdown_handler;
void signal_handler(int signal) {
  switch (signal) {
    case SIGINT:
      shutdown_handler(signal);
      break;
  }
}
}  // namespace

void tunWorker(TunInterface& tun, std::atomic_bool& isShutdown, Client& c) {
  tun.bringUp(true);

  std::vector<uint8_t> buffer(tun.getMtu());

  while (!isShutdown.load(std::memory_order_consume)) {
    // try{
    ssize_t nread = tun.read(buffer);
    if (nread == 0)  // Timeout from select occured.
      continue;
    if (nread < 0) {
      spdlog::error("Error reading from interface");
      tun.close();
    }
    buffer.resize(nread);
    c.onTunPacket(std::move(buffer));
    buffer.resize(tun.getMtu());
    spdlog::info("Read {} bytes", nread);
  }
  tun.bringUp(false);
}

int main(int argc, char* argv[]) {
  std::atomic_bool isShutdown;
  signal(SIGINT, signal_handler);
  shutdown_handler = [&](int signal) {
    spdlog::info("Shutdown :)");
    isShutdown.store(true, std::memory_order_release);
  };

  TunInterface tun("tunc0");
  tun.setPersist(0);
  // tun.setMac("02:23:45:67:89:ab");
  tun.setIp("172.16.0.2");
  tun.setNetmask("255.255.0.0");

  Client c(tun);

  std::thread t(tunWorker, std::ref(tun), std::ref(isShutdown), std::ref(c));
  try {
    boost::system::error_code ec;
    size_t port = 1234;
    auto ip_address_ =
        boost::asio::ip::address::from_string("192.168.0.111", ec);
    if (ec) {
      spdlog::error(
          "Failed to parse the IP address. Error code = {}. Message: {}",
          ec.value(), ec.message());
      throw std::runtime_error("Failed to parse IP address.");
    }

    boost::asio::io_context io_context;
    auto rawSocketPtr =
        std::make_unique<boost::asio::ip::tcp::socket>(io_context);

    auto endpoint = boost::asio::ip::tcp::endpoint(ip_address_, port);
    std::shared_ptr<Socket> s =
        std::make_shared<Socket>(std::move(rawSocketPtr), c);
    s->connect(endpoint);

    io_context.run();

    // std::this_thread::sleep_for(std::chrono::seconds(100));
  } catch (std::exception& e) {
    spdlog::error("Exception: {}", e.what());
  }

  return 0;
}