#include <Socket.h>
#include <SocketInterface.h>

#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

#include "ClientConnector.h"
#include "myviface.h"

struct MacHeader {
  uint8_t dstAddr[6];
  uint8_t srcAddr[6];
  uint16_t type;  // 8 is TCP?
};

struct TcpHeader {
  uint16_t srcPort;
  uint16_t dstPort;

  uint32_t seq;

  uint32_t ackNum;

  uint8_t offset;  // 4bits offset +  4bits reserved
  uint8_t flags;
  uint16_t windowSize;

  uint16_t checksum;
  uint16_t urgentPointer;
};

struct IpHeader {
  uint8_t versionLenght;  // 4bit version, 4bit lenght
  uint8_t typeOfService;  // 3bit for IP precedence, 4bits for Tos, last bit
                          // not used.
  uint16_t totalLen;      // includes ip header and the user data.

  uint16_t identifier;
  uint16_t flagsFragmentedOffset;

  uint8_t timeToLive;
  uint8_t protocol;
  uint16_t headerChecksum;

  uint8_t srcIp[4];
  uint8_t dstIp[4];
};

class IpFrame {
 public:
  IpFrame(std::vector<uint8_t>& data) : data_(data) {}
  void print() {
    IpHeader* ip = reinterpret_cast<IpHeader*>(data_.data());
    std::cout << "IPHeader:" << std::endl;
    std::cout << "srcIP:" << std::dec << (int)ip->srcIp[0] << "."
              << (int)ip->srcIp[1] << "." << (int)ip->srcIp[2] << "."
              << (int)ip->srcIp[3] << std::endl;
    std::cout << "dstIP:" << std::dec << (int)ip->dstIp[0] << "."
              << (int)ip->dstIp[1] << "." << (int)ip->dstIp[2] << "."
              << (int)ip->dstIp[3] << std::endl;
    std::cout << viface::utils::hexdump(data_) << std::endl;
  }

 private:
  std::vector<uint8_t>& data_;
};

class MacFrame {
 public:
  MacFrame(std::vector<uint8_t> data) : data_(data) {}
  void print() {
    MacHeader h;
    memcpy(&h, data_.data(), sizeof(MacHeader));
    std::vector<uint8_t> innerData(std::begin(data_) + sizeof(MacHeader),
                                   std::end(data_));
    std::cout << "dstAddress:" << std::hex << (int)h.dstAddr[0];
    for (size_t i = 1; i < 6; i++)
      std::cout << std::hex << ":" << (int)h.dstAddr[i];
    std::cout << std::endl;
    std::cout << "srcAddr:" << std::hex << (int)h.srcAddr[0];
    for (size_t i = 1; i < 6; i++)
      std::cout << std::hex << ":" << (int)h.srcAddr[i];
    std::cout << std::endl;
    std::cout << "Type:" << std::hex << h.type << std::endl;
    if (h.type == 8)  // TCP!
    {
      IpFrame ip(innerData);
      ip.print();
    }
  }
  std::vector<uint8_t> getPacket() { return std::move(data_); }

 private:
  std::vector<uint8_t> data_;
};

class Client : public ClientConnector {
 public:
  Client(boost::asio::io_context& io_context, const std::string& ip,
         size_t port, RaiiViFace& vf)
      : ClientConnector(io_context, ip, port), vf_(vf) {
    connect();
  }
  ~Client() {}

  void onRead(std::vector<uint8_t> data) override {
    std::cout << "Client got packet." << std::endl;
    vf_.write(std::move(data));
  }
  void onClose(const boost::system::error_code& ec) override {
    std::cout << "Closed :) " << std::endl;
  }
  bool viface_handler(std::string const& name, uint id,
                      std::vector<uint8_t>& packet) {
    std::cout << "Viface got packet." << std::endl;
    std::cout << " from interface " << name;
    std::cout << std::endl;
    MacFrame macFrame(std::move(packet));
    macFrame.print();
    s_.write(std::move(macFrame.getPacket()));
    return true;
  }
  void onConnectionFail(const boost::system::error_code& err) override {
    std::cout << "Reconnect in 1s" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    connect();
  }
  void onConnectSuccess() override {
    std::cout << "Connected!" << std::endl;
    s_.read();
  }

 private:
  RaiiViFace& vf_;
};

void dispatchThread(std::set<viface::VIface*> myifaces, Client& c) {
  viface::dispatcher_cb mycb =
      std::bind(&Client::viface_handler, &c, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);
  viface::dispatch(myifaces, mycb);
}

int main(int argc, char* argv[]) {
  try {
    boost::asio::io_context io_context;
    RaiiViFace vfc("vfc0", "172.16.0.2", "92:69:34:5C:C3:28");
    Client c(io_context, "192.168.0.111", 1234, vfc);

    std::set<viface::VIface*> myifaces = {vfc.getVifacePtr()};
    std::thread t(dispatchThread, myifaces, std::ref(c));
    std::cout << "IoContext::run" << std::endl;
    // for(;;)
    { io_context.run(); }

    // std::this_thread::sleep_for(std::chrono::seconds(100));
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}