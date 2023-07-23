#include "SocketInterface.h"
#include "Socket.h"
#include "SocketConnector.h"
#include "SocketRegistry.h"
#include "myviface.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include <iostream>

using boost::asio::ip::tcp;

class MyDispatcher
{
    public:
      MyDispatcher() = default;
      ~MyDispatcher() = default;
      bool handler(std::string const& name, uint id, std::vector<uint8_t>& packet) {   
        std::cout << "Dispatcher got a packet." << std::endl;
        SocketRegistry::inst().forEach([&](std::shared_ptr<ServerConnector>& sc){
          sc->write(packet);
        });
        return true;
      }
};

class ServerAcceptor
{
public:
  ServerAcceptor(boost::asio::io_service& io_service, short port, RaiiViFace& vf)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      vf_(vf)
  {
    start_accept();
  }

    void doAccept(boost::asio::ip::tcp::socket* s, const boost::system::error_code& ec)
  {
    if(!ec)
    {
      //Need a better solution, that is not going to waste memory.
      auto sc = std::make_shared<ServerConnector>(std::move(std::unique_ptr<boost::asio::ip::tcp::socket>(s)), vf_);
      SocketRegistry::inst().add(sc);
      sc->onConnect();
    }
    else
    {
      std::cout << "Acceptor fail" << std::endl;
      delete s;
    }
    start_accept();
  }

private:
  void start_accept()
  {
    std::cout << "Start accept" << std::endl;
    auto sock = new boost::asio::ip::tcp::socket(io_service_);
    acceptor_.async_accept(*sock, boost::bind(&ServerAcceptor::doAccept, this, sock, boost::asio::placeholders::error));
  }
  RaiiViFace& vf_;
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
};

void doDispatch(std::set<viface::VIface*> myifaces)
{
    MyDispatcher printer;
    viface::dispatcher_cb mycb = std::bind(
              &MyDispatcher::handler,
              &printer,
              std::placeholders::_1,
              std::placeholders::_2,
              std::placeholders::_3
              );
  viface::dispatch(myifaces, mycb);
}

int main()
{
  try
  {
    std::cout << "App start" << std::endl;
    RaiiViFace vfs("vfs0", "172.16.0.1", "92:69:34:5C:C3:28");
    std::cout << "Viface created." << std::endl;

   std::cout << "Dispatch thread..." << std::endl;
   std::thread t(doDispatch, std::set<viface::VIface*>{vfs.getVifacePtr()});

    boost::asio::io_service io_service;
    std::cout << "Create ServerAcceptor" << std::endl;
    ServerAcceptor s(io_service, 1234, vfs);
    io_service.run();

  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}