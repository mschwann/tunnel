#pragma once
#include "Socket.h"
#include "SocketInterface.h"

class Server : public SocketInterface {
 public:
  Server() = default;
  ~Server() = default;
  void onRead(std::vector<uint8_t> data) { s_->write(data); }
  void onClose(const boost::system::error_code& ec) { s_.reset(); }
  void onConnectSuccess(std::shared_ptr<Socket> s) { s_ = s; }
  void onConnectFail(const boost::system::error_code& ec) {}

 private:
  std::shared_ptr<Socket> s_;
};