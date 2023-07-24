#pragma once
#include <boost/asio.hpp>
#include <cstdint>
#include <vector>

class Socket;
class SocketInterface {
 public:
  virtual ~SocketInterface() {}
  virtual void onRead(std::vector<uint8_t> data) = 0;
  virtual void onClose(const boost::system::error_code& ec) = 0;
  virtual void onConnectSuccess(std::shared_ptr<Socket> s) = 0;
  virtual void onConnectFail(const boost::system::error_code& ec) = 0;
};