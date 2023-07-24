#pragma once
#include <viface/utils.hpp>
#include <viface/viface.hpp>

class RaiiViFace {
 public:
  RaiiViFace(const std::string& name, const std::string& ip,
             const std::string& mac)
      : iface(name, false) {
    iface.setIPv4(ip);
    // iface.setMAC(mac);

    iface.up();
  }
  ~RaiiViFace() { iface.down(); }
  viface::VIface* getVifacePtr() { return &iface; }
  void write(std::vector<uint8_t> data) { iface.send(data); }

 private:
  viface::VIface iface;
};