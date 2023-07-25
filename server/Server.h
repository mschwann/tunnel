#pragma once
#include "Socket.h"
#include "SocketInterface.h"
#include "tun_interface.h"
#include "tuntap_interface.h"

class Server : public SocketInterface {
 public:
  Server(TunInterface& tun) : tun_(tun) {}
  ~Server() = default;
  void onRead(std::vector<uint8_t> data) { tun_.write(data); }
  void onClose(const boost::system::error_code& ec) { s_.reset(); }
  void onConnectSuccess(std::shared_ptr<Socket> s) { s_ = s; }
  void onConnectFail(const boost::system::error_code& ec) {}
  void onTunPacket(std::vector<uint8_t> buff) {
    if (s_) s_->write(std::move(buff));
  }

 private:
  std::shared_ptr<Socket> s_;
  TunInterface& tun_;
};