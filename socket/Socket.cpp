#include "Socket.h"

Socket::Socket(std::unique_ptr<boost::asio::ip::tcp::socket> socket,
               SocketInterface& interface)
    : socket_(std::move(socket)), buf_(1024), interface_(interface) {}

void Socket::stop(const boost::system::error_code& ec) {
  if (!stopped_) {
    stopped_ = true;
    interface_.onClose(ec);
    boost::system::error_code ec;
    socket_->close(ec);
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
    std::cout << "Error on receive." << std::endl;
    stop(ec);
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
    std::cout << "Connect timed out\n";
    return;
  }
  if (ec) {
    std::cout << "Connect error: " << ec.message() << "\n";
    stop(ec);
    return;
  }
  std::cout << "Write successfull!" << std::endl;
}