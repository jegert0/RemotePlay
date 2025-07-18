#include "emulation.hpp"


emulatedController::emulatedController( PVIGEM_CLIENT * cli )
    :state{0, 0, 0, 0, 0, 0 ,0},
    client(cli)   
{
    createXbox360Controller( );
}

// overloaded consturctor to create initial
// takes pointer to existing client and a controllerState struct
emulatedController::emulatedController( PVIGEM_CLIENT * cli, controllerState sti )
    :state(sti),
    client(cli)
{
    createXbox360Controller( );
}

// vigem client is being shared by multiple controllers to dellocate it outside of this class
// not allocating anything to heap so far
emulatedController::~emulatedController() {}

void emulatedController::createXbox360Controller(  )
{
    this->pad = vigem_target_x360_alloc(); //creates virtual xbox 360 controller
    vigem_target_set_vid(pad, 0x045E); // Microsoft
    vigem_target_set_pid(pad, 0x028E); // Xbox 360 Controller

    std::this_thread::sleep_for( std::chrono::seconds( 1 )); //wait 1 sec before adding

    const VIGEM_ERROR pir = vigem_target_add( *this->client, this->pad); //connect the gamepad to the vigem client

    if ( !VIGEM_SUCCESS( pir ) )
    {
        std::cerr << "Target Plug in failed with error code: 0x" << std::hex << pir << std::endl;
        throw std::runtime_error(" target add failure ");
        
    }
}

// most likly change to get the vigem/ XUSB state
controllerState emulatedController::getState()
{
    return this->state;
}

//runs target update without updating XUSB_REPORT
void emulatedController::updateState()
{
    vigem_target_x360_update( *(this->client), this->pad, this->viState);
}

//updates the XUSB_REPORT state with controllerSate struct and runs target update
void emulatedController::updateState( controllerState st )
{
    this->state = st;
    this->viState.sThumbLX = st.lx;
    this->viState.sThumbLY = st.ly;
    this->viState.sThumbRX = st.ry;
    this->viState.sThumbRY = st.rx;
    this->viState.wButtons = st.buttons;
    this->viState.bLeftTrigger = st.lt;
    this->viState.bRightTrigger = st.rt;
    vigem_target_x360_update( *(this->client), this->pad, this->viState);
}

//contructor to assosicate joystick pointer and start with blank state
Joystick::Joystick( SDL_Joystick * gc, int id) : gamecontroller(gc), state{0,0,0,0,0,0,0}, index(id) {}

Joystick::~Joystick(){} //nothing got allocated yet

uint16_t Joystick::SDLtoXUSB ( int but )
{   //touchpad button ignored for now
    switch ( but )
    {
        case ( 0 ): return XUSB_GAMEPAD_A;
        case ( 1 ): return XUSB_GAMEPAD_B;
        case ( 2 ): return XUSB_GAMEPAD_X;
        case ( 3 ): return XUSB_GAMEPAD_Y;
        case ( 4 ): return XUSB_GAMEPAD_BACK;
        case ( 5 ): return XUSB_GAMEPAD_GUIDE;
        case ( 6 ): return XUSB_GAMEPAD_START;
        case ( 7 ): return XUSB_GAMEPAD_LEFT_THUMB;
        case ( 8 ): return XUSB_GAMEPAD_RIGHT_THUMB;
        case ( 9 ): return XUSB_GAMEPAD_LEFT_SHOULDER;
        case ( 10 ): return XUSB_GAMEPAD_RIGHT_SHOULDER;

        default: return 0;

    }
}

uint16_t Joystick::SDLHattoXUSB( int hat )
{
    switch( hat )
    {
        case 1: return XUSB_GAMEPAD_DPAD_UP;
        case 2: return XUSB_GAMEPAD_DPAD_RIGHT;
        case 4: return XUSB_GAMEPAD_DPAD_DOWN;
        case 8: return XUSB_GAMEPAD_DPAD_LEFT;

        default: return 0;
    }
}

void Joystick::updateState( SDL_Event event )
{
    switch( event.type )
    {
        case ( SDL_EVENT_JOYSTICK_AXIS_MOTION ):
            
             switch ( event.jaxis.axis % 6 )
            {
                        //get the percent of the total motion done

                        case 0://left stick x
                            //if (event.jaxis.value < -JOYSTICK_DEAD_ZONE || event.jaxis.value > JOYSTICK_DEAD_ZONE) 
                            state.lx = event.jaxis.value;
                            //else state.lx = 0;
                            break;

                        case 1://left stick y
                            //if (event.jaxis.value < -JOYSTICK_DEAD_ZONE || event.jaxis.value > JOYSTICK_DEAD_ZONE) 
                            //my ds4 controller uses up as negative and when inverted it overloads and becomes negative
                            //when the stick is all the way at the top
                            state.ly = -event.jaxis.value-1;
                            //else state.ly = 0;
                            break;

                        case 2://right stick x
                            if (event.jaxis.value < -JOYSTICK_DEAD_ZONE || event.jaxis.value > JOYSTICK_DEAD_ZONE) 
                            state.ry = event.jaxis.value;
                            else state.ry = 0;
                            break;

                        case 3://right stick y
                            if (event.jaxis.value < -JOYSTICK_DEAD_ZONE || event.jaxis.value > JOYSTICK_DEAD_ZONE) 
                            state.rx = event.jaxis.value;
                            else state.rx = 0;
                            break;

                        case 4: //left trigger button
                            state.lt = event.jaxis.value;
                            //SDL_Log("Left trigger value: %d", Ltrigger);
                            break;

                        case 5: // right trigger button
                            state.rt = event.jaxis.value;
                            break;

                        default:
                            break;
            }
            
            break;

        case ( SDL_EVENT_JOYSTICK_BUTTON_DOWN ) :
            state.buttons |= SDLtoXUSB( event.jbutton.button );
            break;
        
        case ( SDL_EVENT_JOYSTICK_BUTTON_UP ) :
            state.buttons &= ~SDLtoXUSB( event.jbutton.button );
            break;  

        case ( SDL_EVENT_JOYSTICK_HAT_MOTION ):
                for ( int i = 0; i < 4; i++)
                {
                    if ( event.jhat.value & hatCases[i]) state.buttons |= SDLHattoXUSB( hatCases[i] );
                    else state.buttons &= ~SDLHattoXUSB( hatCases[i] );
                }
            break;
    }
}

controllerState Joystick::getState()
{
    return this->state;
}