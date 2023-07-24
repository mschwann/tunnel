#include "tuntap_interface.h"

class TapInterface : private TunTapInterface {
  TapInterface(const std::string& name) : TunTapInterface(name, IFF_TAP) {
    alloc();
  }
  using TunTapInterface::bringUp;
  using TunTapInterface::close;
  using TunTapInterface::getIp;
  using TunTapInterface::getMAC;
  using TunTapInterface::getMtu;
  using TunTapInterface::getNetmask;
  using TunTapInterface::isUp;
  using TunTapInterface::read;
  using TunTapInterface::setIp;
  using TunTapInterface::setMac;
  using TunTapInterface::setNetmask;
  using TunTapInterface::setPersist;
  using TunTapInterface::write;
};