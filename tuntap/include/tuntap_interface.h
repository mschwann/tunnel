#pragma once
#include <linux/if.h>
#include <linux/if_tun.h>
#include <unistd.h>

#include <cstdint>
#include <string>
#include <vector>

class TunTapInterface {
 public:
  TunTapInterface(const std::string& name, int flags);

  void alloc();

  void setPersist(bool persist);

  std::string getMAC();
  std::string getIp();
  void setIp(const std::string& ipv4);
  void setNetmask(const std::string& netMask);
  void setMac(const std::string& mac);
  bool isUp();
  std::string getNetmask();

  uint32_t getMtu();

  void bringUp(bool up);

  ssize_t read(std::vector<uint8_t>& buffer, size_t usec = 10000);
  int write(std::vector<uint8_t> buff);
  void close() { ::close(fd_); }

 private:
  char tun_name[IFNAMSIZ];  // 16

  int flags_;
  int fd_;
  int kernelSocket_;

  void readFlags(ifreq& ifr);
};