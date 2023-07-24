#include <arpa/inet.h>
#include <fcntl.h> /* For O_RDWR */
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h> /* For open(), creat() */

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "tuntap_interface.h"

TunTapInterface::TunTapInterface(const std::string& name, int flags)
    : flags_(flags) {
  memset(tun_name, 0, IFNAMSIZ);
  memcpy(tun_name, name.c_str(), std::min((int)name.size(), IFNAMSIZ));
}

void TunTapInterface::alloc() {
  int err;

  char clonedev[] = "/dev/net/tun";
  if ((fd_ = open(clonedev, O_RDWR)) < 0) {
    throw(std::runtime_error("Failed to open clonedev."));
  }

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = flags_;

  if (*tun_name) {
    strncpy(ifr.ifr_name, tun_name, IFNAMSIZ);
  }

  if ((err = ioctl(fd_, TUNSETIFF, (void*)&ifr)) < 0) {
    std::cout << "ioctl failed: " << err << std::endl;
    close();
    throw(std::runtime_error("ioctl failed"));
  }
  strcpy(tun_name, ifr.ifr_name);

  kernelSocket_ = socket(AF_INET, SOCK_STREAM, 0);  // for later
}

void TunTapInterface::setPersist(bool persist) {
  if (ioctl(fd_, TUNSETPERSIST, persist) < 0) {
    throw(std::runtime_error("failed to set persistance."));
  }
}

std::string TunTapInterface::getMAC() {
  ifreq ifr;
  readFlags(ifr);
  if (ioctl(kernelSocket_, SIOCGIFHWADDR, &ifr) != 0) {
    throw std::runtime_error("getMacFailed");
  }
  std::ostringstream addr;
  addr << std::hex << std::setfill('0');
  for (int i = 0; i < 6; i++) {
    addr << std::setw(2) << (unsigned int)(0xFF & ifr.ifr_hwaddr.sa_data[i]);
    if (i != 5) {
      addr << ":";
    }
  }
  return addr.str();
}

std::string TunTapInterface::getIp() {
  ifreq ifr;
  readFlags(ifr);
  if (ioctl(kernelSocket_, SIOCGIFADDR, &ifr) != 0) {
    throw std::runtime_error("Failed to read ip");
  }
  char addr[INET_ADDRSTRLEN];
  memset(&addr, 0, sizeof(addr));

  struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
  if (inet_ntop(AF_INET, &(ipaddr->sin_addr), addr, sizeof(addr)) == NULL) {
    throw std::runtime_error("Failed to convert ip.");
  }

  return std::string(addr);
}

void TunTapInterface::setIp(const std::string& ipv4) {
  ifreq ifr;
  readFlags(ifr);
  struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;
  addr->sin_family = AF_INET;
  if (!inet_pton(AF_INET, ipv4.c_str(), &addr->sin_addr)) {
    throw std::runtime_error("Invalid ip");
  }
  if (ioctl(kernelSocket_, SIOCSIFADDR, &ifr) != 0) {
    throw std::runtime_error("Unable to set ip");
  }
}

void TunTapInterface::setNetmask(const std::string& netMask) {
  ifreq ifr;
  readFlags(ifr);
  struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;
  addr->sin_family = AF_INET;
  if (!inet_pton(AF_INET, netMask.c_str(), &addr->sin_addr)) {
    throw std::runtime_error("Invalid netmask.");
  }

  if (ioctl(kernelSocket_, SIOCSIFNETMASK, &ifr) != 0) {
    throw std::runtime_error("Unable to setup netmask");
  }
}

void TunTapInterface::setMac(const std::string& mac) {
  if ((flags_ | IFF_TUN) != 1) return;
  struct ifreq ifr;
  readFlags(ifr);

  ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;

  sscanf(mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         reinterpret_cast<unsigned char*>(&ifr.ifr_hwaddr.sa_data[0]),
         reinterpret_cast<unsigned char*>(&ifr.ifr_hwaddr.sa_data[1]),
         reinterpret_cast<unsigned char*>(&ifr.ifr_hwaddr.sa_data[2]),
         reinterpret_cast<unsigned char*>(&ifr.ifr_hwaddr.sa_data[3]),
         reinterpret_cast<unsigned char*>(&ifr.ifr_hwaddr.sa_data[4]),
         reinterpret_cast<unsigned char*>(&ifr.ifr_hwaddr.sa_data[5]));

  if (ioctl(kernelSocket_, SIOCSIFHWADDR, &ifr) != 0) {
    throw std::runtime_error("Unable to set mac.");
  }
}

bool TunTapInterface::isUp() {
  ifreq ifr;
  readFlags(ifr);
  return (ifr.ifr_flags & IFF_UP) != 0;
}

std::string TunTapInterface::getNetmask() {
  ifreq ifr;
  readFlags(ifr);
  if (ioctl(kernelSocket_, SIOCGIFNETMASK, &ifr) != 0) {
    throw std::runtime_error("Failed to read netmask.");
  }
  char addr[INET_ADDRSTRLEN];
  memset(&addr, 0, sizeof(addr));

  struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
  if (inet_ntop(AF_INET, &(ipaddr->sin_addr), addr, sizeof(addr)) == NULL) {
    throw std::runtime_error("Failed to convert ip.");
  }

  return std::string(addr);
}

uint32_t TunTapInterface::getMtu() {
  ifreq ifr;
  readFlags(ifr);
  if (ioctl(kernelSocket_, SIOCSIFMTU, &ifr) != 0) {
    throw std::runtime_error("unable to get mtu.");
  }
  return ifr.ifr_mtu;
}

void TunTapInterface::bringUp(bool up) {
  ifreq ifr;
  readFlags(ifr);
  if (up)
    ifr.ifr_flags |= IFF_UP;
  else
    ifr.ifr_flags &= ~IFF_UP;
  if (ioctl(kernelSocket_, SIOCSIFFLAGS, &ifr) != 0) {
    throw std::runtime_error("Unable to set the interface up.");
  }
}

ssize_t TunTapInterface::read(std::vector<uint8_t>& buffer, size_t usec) {
  fd_set set;
  FD_ZERO(&set);     /* clear the set */
  FD_SET(fd_, &set); /* add our file descriptor to the set */
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = usec;

  int rv = select(fd_ + 1, &set, NULL, NULL, &timeout);
  if (rv == -1)
    return 0;  // throw std::runtime_error("Select failed.");
  else if (rv == 0)
    return 0;
  else
    return ::read(fd_, buffer.data(), buffer.size());
}

void TunTapInterface::readFlags(ifreq& ifr) {
  memset(&ifr, 0, sizeof(struct ifreq));
  (void)strncpy(ifr.ifr_name, tun_name, IFNAMSIZ);
  if (ioctl(kernelSocket_, SIOCGIFFLAGS, &ifr) != 0) {
    throw std::runtime_error("readFlags failed");
  }
}

int TunTapInterface::write(std::vector<uint8_t> buff) {
  int nwrite;
  if ((nwrite = ::write(fd_, buff.data(), buff.size())) < 0) {
    std::runtime_error("Error writing");
  }
  return nwrite;
}