/*
    Program creates a server on host machine on specified port entered in the executable
    Need vigem bus installed, so this only runs on windows machine currently

    How to use: .\Server.exe <port>

    This needs msvc compiler because my version of vigem uses it, maybe there is another

    need to do x input
*/


#define _WIN32_WINNT 0x0601
#include <iostream>
#include <boost/asio.hpp>
#include <windows.h>
#include "includes.hpp"
#include "connection.hpp"
#include <ViGEm/Client.h>

//create emulated controllers based on the number of connected clients
std::vector<emulatedController> controllers;

//use a different thread to listen for commands
void inputThreadFunc( Udp_Server* ser )
{
    std::string input;
    std::cout <<"Enter Commands Here:" << std::endl << ">> ";
    while( std::getline( std::cin, input ) )
    {
        if ( input == "exit") 
        {
            ser->turnOff();
            break;
        }
        else std::cout << "Unkown Command" << std::endl;
        //add other commands here in future

        std::cout << ">> ";
    }

}

int main( int argc, char * argv[] )
{
    if ( argc != 2 )
    {
        std::cout << "Correct Usage: .\\Server.exe <port>" <<std::endl;
        return 1;
    }

    
    PVIGEM_CLIENT client = vigem_alloc(); //start vigem api
    if ( !client )
    {
        std::cerr << "vigem client create error" << std::endl;
        return -1;
    }
    
    //establish connection to driver
    //this is expensive only do it once
    const VIGEM_ERROR retval = vigem_connect( client );
    
    if ( !VIGEM_SUCCESS( retval ) )
    {
        std::cerr << "Vigem bus connection failed with error code : 0x" << std::hex << retval << std::endl;
        return -1;
    }

    boost::asio::io_context io;
    Udp_Server server( io, argv[1], &client ); //create udp listing to specific port
        //start other thread to listen for other commands
    std::thread inputThread( inputThreadFunc, &server );
    io.run();

    for ( int i = 0; i < server.getNumClients(); i++)
    {   
        //push new starter copies
        emulatedController c( &client ); 
        controllers.push_back( c );
    }

    
    //join thread at the end
    //block until finished with other threads

    inputThread.join();

    
    return 0;
}