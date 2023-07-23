#pragma once
#include <boost/asio.hpp>

#include <vector>
#include <cstdint>

class SocketInterface
{
    public:
        virtual ~SocketInterface() {}
        virtual void onRead(std::vector<uint8_t> data) = 0;
        virtual void onClose(const boost::system::error_code& ec) = 0;

};