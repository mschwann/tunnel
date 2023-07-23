#pragma once
#include <string>
#include <viface/viface.hpp>
#include <viface/utils.hpp>

class RaiiViFace
{
    public:
        RaiiViFace(const std::string& name)
        : iface(name)
        {

        }
        RaiiViFace(const std::string& name, const std::string& ip, const std::string& mac)
        : iface(name)
        {
            iface.setIPv4(ip);
            iface.setMAC(mac);

            iface.up();
        }
        ~RaiiViFace()
        {
            iface.down();
        }
        void write(std::vector<uint8_t> data)
        {
            iface.send(data);
        }
        viface::VIface* getVifacePtr() {return &iface;}
    private:
        viface::VIface iface;
};