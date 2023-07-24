#include "tuntap_interface.h"

class TunInterface : private TunTapInterface {
  TunInterface(const std::string& name) : TunTapInterface(name, IFF_TUN) {
    alloc();
  }
  using TunTapInterface::bringUp;
  using TunTapInterface::close;
  using TunTapInterface::getIp;
  using TunTapInterface::getMtu;
  using TunTapInterface::getNetmask;
  using TunTapInterface::isUp;
  using TunTapInterface::read;
  using TunTapInterface::setIp;
  using TunTapInterface::setNetmask;
  using TunTapInterface::setPersist;
  using TunTapInterface::write;
};