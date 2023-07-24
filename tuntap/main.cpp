#include <linux/if_tun.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <csignal>
#include <cstring>
#include <functional>
#include <thread>

#include "tuntap_interface.h"

namespace {
std::function<void(int)> shutdown_handler;
void signal_handler(int signal) {
  switch (signal) {
    case SIGINT:
      shutdown_handler(signal);
      break;
  }
}
}  // namespace

int main() {
  std::atomic_bool isShutdown;
  signal(SIGINT, signal_handler);

  TunTapInterface tun("tun0", IFF_TUN);
  tun.alloc();
  tun.setPersist(0);
  tun.setMac("02:23:45:67:89:ab");
  tun.setIp("172.16.0.2");
  tun.setNetmask("255.255.0.0");
  tun.bringUp(true);
  spdlog::info("MAC:{}", tun.getMAC());
  spdlog::info("IP:{}", tun.getIp());
  spdlog::info("Netmask:{}", tun.getNetmask());
  spdlog::info("MTU: {}", tun.getMtu());
  spdlog::info("UP: {}", tun.isUp());

  shutdown_handler = [&](int signal) {
    spdlog::info("TunTap shutdown");
    isShutdown.store(true, std::memory_order_release);
  };

  std::vector<uint8_t> buffer(tun.getMtu());

  while (!isShutdown.load(std::memory_order_consume)) {
    // try{
    ssize_t nread = tun.read(buffer);
    if (nread == 0)  // Timeout from select occured.
      continue;
    if (nread < 0) {
      spdlog::error("Error reading from interface");
      tun.close();
    }
    spdlog::info("Read {} bytes", nread);
  }

  tun.bringUp(false);
  spdlog::info("UP: {}", tun.isUp());

  return 0;
}