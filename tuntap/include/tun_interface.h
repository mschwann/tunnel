#include "tuntapinterface.h"

class TunInterface : private TunTapInterface
{
    TunInterface(const std::string& name) : 
        TunTapInterface(name, IFF_TUN) { alloc();}
    using TunTapInterface::setPersist;
    using TunTapInterface::getIp;
    using TunTapInterface::setIp;
    using TunTapInterface::setNetmask;
    using TunTapInterface::isUp;
    using TunTapInterface::getNetmask;
    using TunTapInterface::getMtu;
    using TunTapInterface::bringUp;
    using TunTapInterface::read;
    using TunTapInterface::write;
    using TunTapInterface::close;
};