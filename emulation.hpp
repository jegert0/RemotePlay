#ifndef EMULATE_HPP
#define EMULATE_HPP

#include "includes.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <ViGEm/Client.h>
#include <thread>

const int JOYSTICK_DEAD_ZONE = 8000;
const double SDL_JOYSTICK_MAX_DOUBLE = static_cast<double> (SDL_JOYSTICK_AXIS_MAX);
const int hatCases[] = { SDL_HAT_DOWN, SDL_HAT_RIGHT, SDL_HAT_LEFT, SDL_HAT_UP };

typedef struct controllerstate
{
    uint16_t buttons; //each bit relates to a button, can store if button is down if 1
    int16_t lx; //left joystick x axis
    int16_t ly; // left joystick y axis
    int16_t rx; //right joystick x axis
    int16_t ry; // right joystick y axis
    BYTE lt; // left trigger button
    BYTE rt; // right trigger button
    
} controllerState;


// needs valid vigem client beforehand
class emulatedController
{
    private:
        controllerState state;
        PVIGEM_CLIENT * client;
        PVIGEM_TARGET pad;
        XUSB_REPORT viState;

    public:
        emulatedController( PVIGEM_CLIENT * cli ); // constructor create empty controller state
        emulatedController( PVIGEM_CLIENT * cli, controllerState sti );// overloaded method with initial controller state 
        ~emulatedController(); //destructor
        void createXbox360Controller( );
        controllerState getState(); //returns copy of game state
        void updateState( controllerState st ); //updates controller state
        void updateState(); //overload to just updating the previous state in the vigem bus
        
};

class Joystick
{
    private:
    SDL_Joystick * gamecontroller;
    controllerState state;
    int index; //keep track of which controller incase multiple are connected

    uint16_t SDLtoXUSB( int but ); //sdl and xusb have different button layouts
    uint16_t SDLHattoXUSB( int hat );

    public:
    Joystick( SDL_Joystick * gc, int id);
    ~Joystick();
    void updateState( SDL_Event event ); //used in poll event loop to update controller state
    controllerState getState();
};

#endif