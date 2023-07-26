#include "tuntap_worker.h"

#include <spdlog/spdlog.h>

void tuntap_worker(TunTapInterface& tuntap, std::atomic_bool& isShutdown,
                   size_t bufSize,
                   std::function<void(std::vector<uint8_t>)> callback) {
  std::vector<uint8_t> buffer(bufSize);
  tuntap.bringUp(true);
  while (!isShutdown.load(std::memory_order_consume)) {
    // try{
    ssize_t nread = tuntap.read(buffer);
    if (nread == 0)  // Timeout from select occured.
      continue;
    if (nread < 0) {
      spdlog::error("Error reading from interface");
      tuntap.close();
    }
    spdlog::info("Read {} bytes", nread);
    buffer.resize(nread);
    callback(std::move(buffer));
    buffer.resize(bufSize);
  }

  tuntap.bringUp(false);
}