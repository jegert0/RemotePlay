/*
    Program supports opening multiple joysticks on 1 machine but the server recognizes 1 enpoint to 1 controller

    need to actually get and send real controlelr state
*/

#include "includes.hpp"
#include "connection.hpp"
#include <boost/asio.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#define SDL_MAIN_HANDLED
#include <mutex>

//use atomic condition variable to check if client is still running between threads
std::atomic<bool> running = true;
std::vector<SDL_Joystick *> gamecontrollers; //vector of game controllers just in case 

//use a different thread to listen for commands
void inputThreadFunc( Udp_Client* cli )
{   
    
    std::string input;
    std::cout <<"Enter Commands Here:" << std::endl << ">> ";
    while( std::getline( std::cin, input ) )
    {
        if ( input == "exit") 
        {   
            cli->turnOff();
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
        std::cout << "Correct Usage: .\\Client.exe <ip:port>" <<std::endl;
        return 1;
    }


    std::string ip;
    std::string port;

    std::string addr( argv[1] );
    auto pos = addr.find(":");

    if ( pos == std::string::npos )
    {
        std::cout << "port missing use <ip:port>" << std::endl;
        return 1;
    }

    ip = addr.substr( 0, pos );
    port = addr.substr( pos + 1 );

 
    std::cout << "Connecting to: " << ip << ":" << port << std::endl;

    //initialize sdl api
    if ( !SDL_Init( SDL_INIT_JOYSTICK ) )
    {
        SDL_Log( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
    }
    else
    {
        SDL_PollEvent( NULL ); //just to update the device list
        int numSticks;
        SDL_JoystickID * joysticks = SDL_GetJoysticks( &numSticks );

        for ( int i = 0; i < numSticks; i++)
        {
            gamecontrollers.push_back( SDL_OpenJoystick( joysticks[i] ) );
            SDL_Log("gamecontroller opened, device id: %d, %s\n", joysticks[i], SDL_GetJoystickName( gamecontrollers[i] ));
        }
        if ( gamecontrollers.empty() ){
            SDL_Log("Game controller could not be open! Error: %s\n", SDL_GetError());
            SDL_free( joysticks );
            exit(0);
        }

        SDL_free( joysticks );

    }

    //create boost client and start sending in controller data
    boost::asio::io_context io;
    Udp_Client client( io, ip, port );
    //create another thread here
    std::thread inputThread( inputThreadFunc, &client );
    io.run();

    SDL_Event event;

    /*
        Here is where physical controller state data is read and sent through the boost client to the destined server
        currently will just send the first joystick recognized in the vector of joystick pointers
    */
    Joystick stick( gamecontrollers[0], 0 );
    while ( client.isRunning() )
    {   
        while( SDL_PollEvent( &event ) )
        {
            stick.updateState( event );
            client.send_state( stick.getState() );
        }
    }

    //block so program does not terminate
    inputThread.join();

    return 0;
}