#pragma once

#include <SocketInterface.h>
#include <Socket.h>

#include <boost/asio.hpp>

class ClientConnector : public SocketInterface
{
    public:
        ClientConnector(boost::asio::io_context& io_context, const std::string& ip, size_t port)
        :s_(std::make_unique<boost::asio::ip::tcp::socket>(io_context), *this),
        port_(port)
        {
            boost::system::error_code ec;
            ip_address_ = boost::asio::ip::address::from_string(ip, ec);
            if(ec)
            {
                std::cout << "Failed to parse the IP address. Error code = "<< ec.value() << ". Message: " << ec.message() << std::endl;
                throw std::runtime_error("Failed to parse IP address.");
            }
        }
        void connect(){
            s_.getRawSocket()->async_connect(boost::asio::ip::tcp::endpoint(ip_address_, port_),
            [this](const boost::system::error_code& ec) {
                handleConnect(ec);
            });
        }
        virtual void onConnectSuccess() = 0;
        virtual void onConnectionFail(const boost::system::error_code& err) = 0;
    protected:
        Socket s_;
    private:
        boost::asio::ip::address ip_address_;
        size_t port_;

        void handleConnect(const boost::system::error_code& ec)
        {
            if(s_.isStopped())
            {
                return;
            }
            if (ec || !s_.isOpen())
            {
                onConnectionFail(ec);
                return;
            }
            onConnectSuccess();
        }
};