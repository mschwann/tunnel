#pragma once
#include "Socket.h"
#include "SocketInterface.h"
#include "myviface.h"

class ServerConnector : public SocketInterface {
 public:
  ServerConnector(std::unique_ptr<boost::asio::ip::tcp::socket> s,
                  RaiiViFace& vf)
      : s_(std::move(s), *this), vf_(vf) {}
  void onConnect() { s_.read(); }
  void onRead(std::vector<uint8_t> data) override {
    std::cout << "Socket got packet." << std::endl;
    vf_.write(std::move(data));
  }
  void write(std::vector<uint8_t> data) {
    std::cout << "Socket gets to send packet" << std::endl;
    s_.write(std::move(data));
  }
  void onClose(const boost::system::error_code& ec) override {}
  Socket& getSocket() { return s_; }

 protected:
  Socket s_;
  RaiiViFace& vf_;
};