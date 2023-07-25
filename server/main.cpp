#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "Server.h"
#include "Socket.h"
#include "SocketInterface.h"
#include "spdlog/spdlog.h"
#include "tun_interface.h"

using boost::asio::ip::tcp;

class ServerAcceptor {
 public:
  ServerAcceptor(boost::asio::io_service& io_service, short port, Server& srv)
      : io_service_(io_service),
        acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
        srv_(srv) {}

  void doAccept(boost::asio::ip::tcp::socket* rawSocket,
                const boost::system::error_code& ec) {
    auto s = std::make_shared<Socket>(
        std::unique_ptr<boost::asio::ip::tcp::socket>(rawSocket), srv_);
    s->handleConnect(ec);
    start_accept();
  }

  void start_accept() {
    spdlog::info("Start accept");
    auto sock = new boost::asio::ip::tcp::socket(io_service_);
    acceptor_.async_accept(*sock,
                           boost::bind(&ServerAcceptor::doAccept, this, sock,
                                       boost::asio::placeholders::error));
  }

 private:
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  Server& srv_;
};

void tunWorker(TunInterface& tun, Server& s) {
  tun.bringUp(true);

  std::vector<uint8_t> buffer(tun.getMtu());

  while (1) {
    // try{
    ssize_t nread = tun.read(buffer);
    if (nread == 0)  // Timeout from select occured.
      continue;
    if (nread < 0) {
      spdlog::error("Error reading from interface");
      tun.close();
    }
    buffer.resize(nread);
    s.onTunPacket(std::move(buffer));
    buffer.resize(tun.getMtu());
    spdlog::info("Read {} bytes", nread);
  }
  tun.bringUp(false);
}

int main() {
  TunInterface tun("tuns0");
  tun.setPersist(0);
  // tun.setMac("02:23:45:67:89:ab");
  tun.setIp("172.16.0.1");
  tun.setNetmask("255.255.0.0");

  Server srv(tun);
  std::thread t(tunWorker, std::ref(tun), std::ref(srv));
  try {
    boost::asio::io_service io_service;

    ServerAcceptor s(io_service, 1234, srv);
    s.start_accept();
    io_service.run();

  } catch (std::exception& e) {
    spdlog::error("Exception: {}", e.what());
  }
  return 0;
}