#define _WIN32_WINNT 0x0601
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

int main()
{
    try
    {
        boost::asio::io_context io;

        udp::socket socket(io, udp::endpoint(udp::v4(), 9000));

        std::cout << "Server listening on port 9000" << std::endl;

        //synchronous udp receive
        for(;;)
        {
            char data[1024];
            udp::endpoint remote_endpoint;
            boost::system::error_code error;

            size_t length = socket.receive_from(boost::asio::buffer(data), remote_endpoint, 0, error);

            if (!error && length > 0) 
            {
                std::cout << "Received From "
                            << remote_endpoint.address().to_string()
                            << ":" << remote_endpoint.port()
                            << " -> " << std::string(data, length)
                            << std::endl;
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;

}