#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include "Server.h"
#include "Socket.h"
#include "SocketInterface.h"

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
    std::cout << "Start accept" << std::endl;
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

int main() {
  try {
    boost::asio::io_service io_service;
    Server srv;
    ServerAcceptor s(io_service, 1234, srv);
    s.start_accept();
    io_service.run();

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}