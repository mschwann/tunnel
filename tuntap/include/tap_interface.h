#include "tuntap_interface.h"

class TapInterface : private TunTapInterface
{
    TapInterface(const std::string& name) : 
        TunTapInterface(name, IFF_TAP) { alloc();}
        using TunTapInterface::setPersist;
    using TunTapInterface::getMAC;
    using TunTapInterface::getIp;
    using TunTapInterface::setIp;
    using TunTapInterface::setNetmask;
    using TunTapInterface::setMac;
    using TunTapInterface::isUp;
    using TunTapInterface::getNetmask;
    using TunTapInterface::getMtu;
    using TunTapInterface::bringUp;
    using TunTapInterface::read;
    using TunTapInterface::write;
    using TunTapInterface::close;
};