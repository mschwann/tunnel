#pragma once

#include <Socket.h>
#include <SocketInterface.h>

#include <boost/asio.hpp>

class Client : public SocketInterface {
 public:
  Client() = default;
  ~Client() = default;
  void onRead(std::vector<uint8_t> data) {
    std::cout << "Client read:" << std::string(data.begin(), data.end())
              << std::endl;
    s_->close(boost::system::error_code());
  }
  void onClose(const boost::system::error_code& ec) { s_.reset(); }
  void onConnectSuccess(std::shared_ptr<Socket> s) {
    s_ = s;
    std::string helloWorld = "Hello World!";
    s_->write(std::vector<uint8_t>(helloWorld.begin(), helloWorld.end()));
  }
  void onConnectFail(const boost::system::error_code& ec) {}

 private:
  std::shared_ptr<Socket> s_;
};