/*
To DO:
Make and emulatoed controller that copies every input from connected controller,
Have mirror copy emulated controller

*/
#define SDL_MAIN_HANDLED
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <windows.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <ViGEm/Client.h>
#include "emulation.hpp"


SDL_Joystick * gamecontroller;

int main(int argc, char * argv[])
{
    PVIGEM_CLIENT client = vigem_alloc(); //start vigem api
    if ( !client )
    {
        std::cerr << "Client create error" << std::endl;
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

    if ( !SDL_Init( SDL_INIT_JOYSTICK ) )
    {
        SDL_Log( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
    }
    else
    {
        SDL_PollEvent( NULL ); //just to update the device list
        SDL_JoystickID * joysticks = SDL_GetJoysticks( NULL );
        gamecontroller = SDL_OpenJoystick( joysticks[0] ); // open the first controller
        if ( !gamecontroller ){
            SDL_Log("Game controller could not be open! Error: %s\n", SDL_GetError());
            exit(0);
        }
        else 
        {
            SDL_Log("gamecontroller opened, device id: %d, %s\n", joysticks[0], SDL_GetJoystickName( gamecontroller ));
        }

        SDL_free( joysticks );

    }
    
    Joystick stick( gamecontroller, 0);
    emulatedController Econtroller( &client );
    emulatedController Econtroller2( &client );
    

    int esc = 1;
    int toggle = 0;

    controllerState st = {0, 1600, 8000, 0, 0, 0, 0};

    SDL_Event event;
    while ( esc )
    {

        while(SDL_PollEvent( &event ))
        {
            if ( GetAsyncKeyState( VK_ESCAPE ) & 0x8000 )
            {
                esc = 0;
                break;
            }
            stick.updateState( event );
            Econtroller.updateState( stick.getState() );
            Econtroller2.updateState( stick.getState() );

        }
      
    }
    return 0;
}