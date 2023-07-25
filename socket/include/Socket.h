#pragma once
#include <atomic>
#include <boost/asio.hpp>
#include <functional>
#include <memory>

#include "SocketInterface.h"

using std::placeholders::_1;
using std::placeholders::_2;

class Socket : public std::enable_shared_from_this<Socket> {
 public:
  Socket(std::unique_ptr<boost::asio::ip::tcp::socket> socket,
         SocketInterface& interface);
  ~Socket() = default;

  bool isStopped() { return stopped_.load(std::memory_order_acquire); }
  bool isOpen() {
    return !stopped_.load(std::memory_order_acquire) && socket_->is_open();
  }

  void close(const boost::system::error_code& ec);

  void write(std::vector<uint8_t> data);
  void read();
  std::unique_ptr<boost::asio::ip::tcp::socket>& getRawSocket() {
    return socket_;
  }

  void handleConnect(const boost::system::error_code& ec);
  void connect(const boost::asio::ip::tcp::endpoint& enpoint) {
    socket_->async_connect(enpoint,
                           [this](const boost::system::error_code& ec) mutable {
                             handleConnect(ec);
                           });
  }

 private:
  void handleReadHeader(const boost::system::error_code& ec, size_t n,
                        size_t prev);
  void handleRead(const boost::system::error_code& ec, size_t n);
  void handleWrite(const boost::system::error_code& ec);

  std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
  std::vector<uint8_t> buf_;
  std::vector<uint8_t> packet_;
  size_t packetSize_;
  size_t readSize_;
  std::atomic_bool stopped_ = false;
  SocketInterface& interface_;
};