#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <atomic>
#include <memory>
#include <thread>
#include <chrono>

namespace SERVER_HANDL
{
class Service
{
private:
    void HandleClient(std::shared_ptr<boost::asio::ip::tcp::socket> sock) {
        try
        {
            boost::asio::streambuf request;
            boost::asio::read_until(*sock.get(), request, '\n');
            int i = 0;
            while (i != 1100000)
            {
                i++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::string response = "Response\n";
            boost::asio::write(*sock.get(), boost::asio::buffer(response));
        }
        catch(const boost::system::system_error& e)
        {
            std::cerr << "Error occured! Error code = " << e.code() << ". Message: " << e.what() << '\n';
        }
        delete this;
    }
public:
    Service(){}
    ~Service(){}
    void StartHandligClient(std::shared_ptr<boost::asio::ip::tcp::socket> sock) {
        std::thread th([this, sock]() {
            HandleClient(sock);
        });
        th.detach();
        
    }
};

}
