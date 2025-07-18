#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

int main()
{
    try
    {
        boost::asio::io_context io;

        udp::resolver resolver(io);
        udp::endpoint endpoint = *resolver.resolve(udp::v4(), "localhost", "9000").begin();

        udp::socket socket(io);
        socket.open(udp::v4());

        std::string message;

        while( true )
        {
            std::cout << "Enter Message: ";
            std::getline( std::cin, message );

            if ( message.empty() ) continue;

            socket.send_to(boost::asio::buffer( message ), endpoint );
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}