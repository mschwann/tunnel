#include "Socket.h"

#include <atomic>
#include <thread>

#include "spdlog/spdlog.h"

Socket::Socket(std::unique_ptr<boost::asio::ip::tcp::socket> socket,
               SocketInterface& interface)
    : socket_(std::move(socket)), buf_(1024 * 1024), interface_(interface) {}

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
  size_t dataSize = data.size();
  boost::asio::async_write(*socket_,
                           boost::asio::buffer(&dataSize, sizeof(dataSize)),
                           std::bind(&Socket::handleWrite, this, _1));
  boost::asio::async_write(*socket_, boost::asio::buffer(data, data.size()),
                           std::bind(&Socket::handleWrite, this, _1));
}

void Socket::read() {
  socket_->async_read_some(
      boost::asio::mutable_buffer(&packetSize_, sizeof(packetSize_)),
      std::bind(&Socket::handleReadHeader, this, _1, _2));
}

void Socket::handleReadHeader(const boost::system::error_code& ec, size_t n) {
  if (stopped_) return;
  if (ec) {
    spdlog::warn("Spdlog: Error on receive.");
    close(ec);
    return;
  }
  if (n == sizeof(packetSize_)) {
    buf_.resize(packetSize_);
    packet_.reserve(packetSize_);
    socket_->async_read_some(
        boost::asio::mutable_buffer(buf_.data(), buf_.size()),
        std::bind(&Socket::handleRead, this, _1, _2));
  } else {
    spdlog::warn("Weird read: {}", n);
    close(ec);
  }
}

void Socket::handleRead(const boost::system::error_code& ec, size_t n) {
  if (stopped_) return;
  if (ec) {
    spdlog::warn("Spdlog: Error on receive.");
    close(ec);
    return;
  }
  if (n) {
    if (n == packetSize_) {
      interface_.onRead(std::move(buf_));
    } else {
      // This is a fragmented packet, we are doomed.
      spdlog::info("(Fragmented packet read)");
      std::copy(buf_.begin(), buf_.begin() + n, std::back_inserter(packet_));
      readSize_ += n;
      if (readSize_ >= packetSize_) {
        interface_.onRead(std::move(buf_));
      } else {
        socket_->async_read_some(
            boost::asio::mutable_buffer(buf_.data() + readSize_,
                                        packetSize_ - readSize_),
            std::bind(&Socket::handleRead, this, _1, _2));
        return;
      }
    }
  }
  read();
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