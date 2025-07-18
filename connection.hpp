#ifndef CONNECTION_HPP
#define CONNECITON_HPP

#include "includes.hpp"
#include "emulation.hpp"
#include <unordered_map>
#include <boost/asio.hpp>

using boost::asio::ip::udp;



class Udp_Server  //std::enable_shared_from_this<Udp_Server> possible shared pointer change later
{
    private:
    std::atomic<bool> running_;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::unordered_map<udp::endpoint, emulatedController*> controllers_;
    std::array<char, 1024> recv_buffer_;
    PVIGEM_CLIENT * vigem_client_;

    void start_receive();

    public:
    Udp_Server(boost::asio::io_context& io, std::string port, PVIGEM_CLIENT * vc);
    controllerState getState( udp::endpoint ep );
    int getNumClients();
    ~Udp_Server();
    bool isRunning();
    void turnOff();
};

class Udp_Client 
{
    private:
    std::atomic<bool> running_;
    udp::socket socket_;
    udp::endpoint endpoint_;
    udp::endpoint server_endpoint_;

    void start_receive( );

    public:
    Udp_Client(boost::asio::io_context& io, const std::string& host, const std::string& port);
    void send_state(const controllerState& state);
    bool isRunning();
    void turnOff();
    //~Udp_Client();


};

#endif
