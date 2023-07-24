#include "Socket.h"

#include <atomic>
#include <thread>

#include "spdlog/spdlog.h"

Socket::Socket(std::unique_ptr<boost::asio::ip::tcp::socket> socket,
               SocketInterface& interface)
    : socket_(std::move(socket)), buf_(1024), interface_(interface) {}

void Socket::close(const boost::system::error_code& ec) {
  bool localStopped = stopped_.load(std::memory_order_acquire);
  while (!stopped_.compare_exchange_weak(localStopped, true,
                                         std::memory_order_acquire,
                                         std::memory_order_release)) {
    std::this_thread::yield();
  }
  if (!localStopped) {
    spdlog::debug("Socket: Executing stop");
    boost::system::error_code ec;
    socket_->close(ec);
    interface_.onClose(ec);
  }
}

void Socket::write(std::vector<uint8_t> data) {
  boost::asio::async_write(*socket_, boost::asio::buffer(data, data.size()),
                           std::bind(&Socket::handleWrite, this, _1));
}

void Socket::read() {
  socket_->async_read_some(
      boost::asio::mutable_buffer(buf_.data(), buf_.size()),
      std::bind(&Socket::handleRead, this, _1, _2));
}

void Socket::handleRead(const boost::system::error_code& ec, size_t n) {
  if (stopped_) return;
  if (ec) {
    spdlog::warn("Spdlog: Error on receive.");
    close(ec);
    return;
  }
  if (n) {
    buf_.resize(n);
    interface_.onRead(std::move(buf_));
    buf_.resize(1024);
  }
  socket_->async_read_some(
      boost::asio::mutable_buffer(buf_.data(), buf_.size()),
      std::bind(&Socket::handleRead, this, _1, _2));
}

void Socket::handleWrite(const boost::system::error_code& ec) {
  if (stopped_) return;
  if (!socket_->is_open()) {
    spdlog::info("Connection timed out");
    return;
  }
  if (ec) {
    spdlog::info("Connect error: {}", ec.message());
    close(ec);
    return;
  }
}
void Socket::handleConnect(const boost::system::error_code& ec) {
  if (stopped_) {
    return;
  }
  if (ec || !isOpen()) {
    interface_.onConnectFail(ec);
    return;
  }
  interface_.onConnectSuccess(shared_from_this());
  read();
}