#pragma once

#include <Socket.h>
#include <SocketInterface.h>

#include <boost/asio.hpp>

#include "spdlog/spdlog.h"
#include "tun_interface.h"

class Client : public SocketInterface {
 public:
  Client(TunInterface& tun) : tun_(tun) {}
  ~Client() = default;
  void onRead(std::vector<uint8_t> data) {
    spdlog::info("Client read");
    tun_.write(std::move(data));
  }
  void onClose(const boost::system::error_code& ec) {
    s_->close(ec);
    s_.reset();
  }
  void onConnectSuccess(std::shared_ptr<Socket> s) {
    s_ = s;
    std::string helloWorld = "Hello World!";
  }
  void onConnectFail(const boost::system::error_code& ec) {}

  void onTunPacket(std::vector<uint8_t> buf) {
    if (s_) {
      s_->write(std::move(buf));
    }
  }

 private:
  std::shared_ptr<Socket> s_;
  TunInterface& tun_;
};