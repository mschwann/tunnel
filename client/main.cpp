#include <Socket.h>
#include <SocketInterface.h>

#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

#include "Client.h"

int main(int argc, char* argv[]) {
  try {
    boost::system::error_code ec;
    size_t port = 1234;
    auto ip_address_ = boost::asio::ip::address::from_string("127.0.0.1", ec);
    if (ec) {
      std::cout << "Failed to parse the IP address. Error code = " << ec.value()
                << ". Message: " << ec.message() << std::endl;
      throw std::runtime_error("Failed to parse IP address.");
    }

    boost::asio::io_context io_context;
    auto rawSocketPtr =
        std::make_unique<boost::asio::ip::tcp::socket>(io_context);

    Client c;
    auto endpoint = boost::asio::ip::tcp::endpoint(ip_address_, port);
    std::shared_ptr<Socket> s =
        std::make_shared<Socket>(std::move(rawSocketPtr), c);
    s->connect(endpoint);

    io_context.run();

    // std::this_thread::sleep_for(std::chrono::seconds(100));
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}