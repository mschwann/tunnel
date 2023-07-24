#include "tuntap_interface.h"

#include <iostream>
#include <linux/if_tun.h>
#include <atomic>
#include <csignal>
#include <functional>
#include <cstring>
#include <thread>


namespace {
std::function<void(int)> shutdown_handler;
void signal_handler(int signal) {
    switch (signal) {
        case SIGINT:
            shutdown_handler(signal); 
            break;
    }}
} // namespace

int main()
{
    std::atomic_bool isShutdown;
    signal(SIGINT, signal_handler);

    TunTapInterface tun("tun0", IFF_TUN);
    tun.alloc();
    tun.setPersist(0);
    tun.setMac("02:23:45:67:89:ab");
    tun.setIp("172.16.0.2");
    tun.setNetmask("255.255.0.0");
    tun.bringUp(true);
    std::cout << "MAC:" << tun.getMAC() << std::endl;
    std::cout << "IP:" << tun.getIp() << std::endl;
    std::cout << "Netmask" << tun.getNetmask() << std::endl;
    std::cout << "MTU:" << tun.getMtu() << std::endl;
    std::cout << "UP:" << tun.isUp() << std::endl;

    shutdown_handler = [&](int signal) {
        std::cout << "TunTap shutdown...\n";
        isShutdown.store(true, std::memory_order_release);
    };

    std::vector<uint8_t> buffer(tun.getMtu());

    while(!isShutdown.load(std::memory_order_consume)) {
        //try{
            ssize_t nread = tun.read(buffer);
            if(nread == 0) //Timeout from select occured.
                continue;
            if(nread < 0) {
                std::cout << "Error reading from interface" << std::endl;
                tun.close();
            }
            printf("Read %ld bytes \n", nread);
    }
    
    tun.bringUp(false);
    std::cout << "UP:" << tun.isUp() << std::endl;

    return 0;
}