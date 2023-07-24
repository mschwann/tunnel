#pragma once
#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <memory>

#include "SocketInterface.h"

using std::placeholders::_1;
using std::placeholders::_2;

class Socket {
 public:
  Socket(std::unique_ptr<boost::asio::ip::tcp::socket> socket,
         SocketInterface& interface);
  ~Socket() = default;

  bool isStopped() { return stopped_; }
  bool isOpen() { return !stopped_ && socket_->is_open(); }

  void stop(const boost::system::error_code& ec);

  void write(std::vector<uint8_t> data);
  void read();
  std::unique_ptr<boost::asio::ip::tcp::socket>& getRawSocket() {
    return socket_;
  }

 private:
  void handleRead(const boost::system::error_code& ec, size_t n);
  void handleWrite(const boost::system::error_code& ec);

  std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
  std::vector<uint8_t> buf_;
  bool stopped_ = false;
  SocketInterface& interface_;
};