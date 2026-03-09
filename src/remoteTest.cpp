#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <vector>
#define SDL_MAIN_HANDLED

//screen dimensions
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 960;

//analog joystick deadzone
const int JOYSTICK_DEAD_ZONE = 8000;
const int pos_lsX = SCREEN_WIDTH / 3;
const int pos_s = (3 * SCREEN_HEIGHT) / 4;
const int pos_rsX = (2 * SCREEN_WIDTH) / 3;
const int stickRadius = 40;

//default button spacing
const int spacing = 30;
//dpad size
const int dpadSize = 20;
const int hatCases[] = { SDL_HAT_DOWN, SDL_HAT_RIGHT, SDL_HAT_LEFT, SDL_HAT_UP };

//game controller handler
SDL_Joystick * gamecontroller = NULL;

//window to display
SDL_Window * gWindow = NULL;

//Render opbject to render
SDL_Renderer * gRenderer = NULL;


#define CIRCLE_RESULUTION 200
//struct to keep data of a circle 
typedef struct CircleType{
    int x;
    int y; 
    int radius;
};

//function draws circle
void drawCircle( SDL_Renderer * render, int centerx, int centery, int radius ){
    int numPoints = CIRCLE_RESULUTION;
    SDL_FPoint * points = new SDL_FPoint[numPoints];
    double angleIncrement = (2 * SDL_PI_D ) / numPoints;

    for ( int i = 0; i < numPoints; i++ ){
        double angle = i * angleIncrement;
        points[i].x = static_cast<int>(centerx + radius * std::cos(angle));
        points[i].y = static_cast<int>(centery + radius * std::sin(angle));
    }

    SDL_RenderPoints(render, points, numPoints);
    delete[] points;

}

//make filled circle function
void drawCircleFill( SDL_Renderer * render, int centerx, int centery, int radius)
{
    int numPoints = CIRCLE_RESULUTION;
    double angleIncrement = 2 * SDL_PI_D / numPoints;
    double angle = 0.0;
    int x = 0;
    int y1 = 0;
    int y2 = 0;
    for ( int i = 0; i < numPoints/2; i++)
    {   
        angle = i * angleIncrement;
        //draw vertical line from x,y to x,-y
        x = static_cast<int> (centerx + radius * std::cos(angle));
        y1 = static_cast<int> (centery + radius * std::sin(angle));
        y2 = static_cast<int> (centery + radius * -std::sin(angle));
        SDL_RenderLine(render, x, y1, x, y2);
        
    }
   
}

//function that runs once to initialize SDL, return True if complete successfully
bool init(){

    //initialize flag
    bool success = true;

    //initialize SDL
    if( !SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK )){
        SDL_Log( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        success = false;
    }
    else{
        //refresh device list
        //SDL_PollEvent(NULL);
        //check for joysticks
        //int count = 0;
        SDL_JoystickID *joysticks = SDL_GetJoysticks( NULL );

       
            //SDL_Log("Device ID: %d", joysticks[i]);
            
            //open joysticks so it can start sending input to event queue
            gamecontroller = SDL_OpenJoystick( joysticks[0] );
        if ( !gamecontroller ){
            SDL_Log("Game controller could not be open! Error: %s\n", SDL_GetError());
        }
        else 
        {
            SDL_Log("gamecontroller opened, device id: %d, %s\n", joysticks[0], SDL_GetJoystickName( gamecontroller ));
            //break; // keep first controller
            
        }
            
        if (! gamecontroller ){

            SDL_Log("Warning no joysticks connected!\n");
            
        }

        SDL_free(joysticks); //i guess since its only 1 joystick you dont need to refrence it?
        //no longer connecting anymore joysticks so dont need
        
        //create window and renderer
        if( !SDL_CreateWindowAndRenderer("Remote Test", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &gWindow, &gRenderer)){
            SDL_Log("Window or Renderer could not be created! Error: %s\n", SDL_GetError());
            success = false;
        }
        else{

            SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF); //set screen to balck
        }
    }


    return success; //finish initialization
}

//function will drawn gamepad buttons and joystick area every frame
//function does not use images only geometrical renders
//does not have dynamic button pressing for now, just skeleton
//color scheme black backrgound with white button outlines
void drawBackround( SDL_Renderer * render ){
    //have to draw dpad(4 square buttons on left), control buttons (4 circles on right)
    // 2 joysticks
    // lb and rb 2 rectangles where index finger is
    // lt and rt 2 squares that are a lever where middle fingers are
    // also maybe touchpad for ps4 controller and start and menu buttons 

    //start with black screen
    SDL_SetRenderDrawColor( render, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear( render );

    //draw dpad, positioned middle left
    SDL_FRect dpad[4];
    //initialze all of them with a default value
    for(int i = 0; i < 4; i++) dpad[i] = { SCREEN_WIDTH / 4,  SCREEN_HEIGHT / 2, dpadSize, dpadSize };
    //moving them to positon by spacing px
    
    dpad[0].y -= spacing;
    dpad[3].y += spacing;
    dpad[1].x -= spacing;
    dpad[2].x += spacing;

    //draw back buttons
    SDL_FRect backButtons[6];
    backButtons[0] =  { SCREEN_WIDTH / 4 - 20,  SCREEN_HEIGHT / 4, 60, 40 };
    backButtons[1] =  { (3 * SCREEN_WIDTH) / 4 - 20,  SCREEN_HEIGHT / 4, 60, 40 };
    backButtons[2] =  { SCREEN_WIDTH / 4 - 20,  SCREEN_HEIGHT / 4 + 60, 60, 20 };
    backButtons[3] =  { (3 * SCREEN_WIDTH) / 4 - 20,  SCREEN_HEIGHT / 4 + 60, 60, 20 };
    backButtons[4] =  { SCREEN_WIDTH / 2 - (SCREEN_WIDTH/10), SCREEN_HEIGHT / 2, 20, 10};
    backButtons[5] =  { SCREEN_WIDTH / 2 + (SCREEN_WIDTH/10), SCREEN_HEIGHT / 2, 20, 10};


    //draw circle buttons
    CircleType circleButtons[4];
    for ( int i = 0; i < 4; i ++ ) circleButtons[i] = { ( 3 * SCREEN_WIDTH) / 4, SCREEN_HEIGHT / 2, 15 };

    circleButtons[0].y -= spacing;
    circleButtons[3].y += spacing;
    circleButtons[1].x -= spacing;
    circleButtons[2].x += spacing;;

    SDL_SetRenderDrawColor( render, 0xFF, 0xFF, 0xFF, 0xFF ); //default white outlining
    //render every button square
    for ( int i = 0; i < 4; i++ ) {
        SDL_RenderRect( render, &dpad[i] );
        drawCircle( render, circleButtons[i].x, circleButtons[i].y, circleButtons[i].radius );
    }

    for (int i = 0; i < sizeof(backButtons); i++)
    {
        SDL_RenderRect( render, &backButtons[i] );
    }


    drawCircle( render, SCREEN_WIDTH / 2, ( 3 * SCREEN_HEIGHT ) / 4, 20 ); // ps4 button
    //draw joysticks
    drawCircle( render, SCREEN_WIDTH / 3 , (3 * SCREEN_HEIGHT) / 4, stickRadius);
    drawCircle( render, (2 * SCREEN_WIDTH) / 3 , (3 * SCREEN_HEIGHT) / 4, stickRadius);

    //does not update render becuase it will cause flickering before inputs are considered
}

//function takes in joystick axis and moves dot within joystick circle
void drawJoystickDot(SDL_Renderer * render, double dotX, double dotY, int posx, int posy, int circleRadius){

    const int dotSize = 10; //make small dot of 10x10 px
    SDL_FRect joyDot = { (dotX * circleRadius) + posx - (dotSize/2), (dotY * circleRadius) + posy - (dotSize/2), dotSize, dotSize };
    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );
    SDL_RenderFillRect(render, &joyDot);
    
}

//draws button as long as it is being held down
void drawButtonsDown( SDL_Renderer * render, bool * buttons )
{   
    //x button on ds4
    if ( buttons[0] )
    {
        SDL_SetRenderDrawColor( render, 124, 178, 232, 255 );
        drawCircleFill( render, (3 * SCREEN_WIDTH) / 4, SCREEN_HEIGHT / 2 + spacing, 15 );
    }
    //circle button on ds4
    if ( buttons[1] )
    {
        SDL_SetRenderDrawColor( render, 255, 102, 102, 255 );
        drawCircleFill( render, (3 * SCREEN_WIDTH) / 4 + spacing, SCREEN_HEIGHT / 2, 15 );
    }
    //squre button on ds4
    if ( buttons[2] ) 
    {
        SDL_SetRenderDrawColor( render, 0xF6, 0x9D, 0xC8, 0xFF );
        drawCircleFill( render, (3 * SCREEN_WIDTH) / 4 - spacing, SCREEN_HEIGHT / 2, 15 );
    }
    //triangle button
    if ( buttons[3] )
    {
        SDL_SetRenderDrawColor( render, 64, 226, 160, 255 );
        drawCircleFill( render, (3 * SCREEN_WIDTH) / 4, SCREEN_HEIGHT / 2 - spacing, 15 );

    }

    //all other buttons are just going to be white
    SDL_SetRenderDrawColor ( render, 0xFF, 0xFF, 0xFF, 0xFF );



    SDL_FRect rect = {};

    if ( buttons[4] )
    {
        rect = { SCREEN_WIDTH / 2 - (SCREEN_WIDTH/10), SCREEN_HEIGHT / 2, 20, 10};
        SDL_RenderFillRect( render, &rect);
    }

    if ( buttons[5] ) //ps4 button
    {
        drawCircleFill( render, SCREEN_WIDTH / 2, ( 3 * SCREEN_HEIGHT ) / 4, 20 );
    }

    if ( buttons[6] )
    {
        rect = { SCREEN_WIDTH / 2 + (SCREEN_WIDTH/10), SCREEN_HEIGHT / 2, 20, 10};
        SDL_RenderFillRect( render, &rect);
    }

    if ( buttons[9] )
    {
        rect = { ( SCREEN_WIDTH) / 4 - 20,  SCREEN_HEIGHT / 4 + 60, 60, 20 };
        SDL_RenderFillRect( render, &rect );
    }

    if ( buttons[10] )
    {
        rect = { (3 * SCREEN_WIDTH) / 4 - 20,  SCREEN_HEIGHT / 4 + 60, 60, 20 };
        SDL_RenderFillRect( render, &rect);
    }
}

void drawButtonsDown( SDL_Renderer * render, std::vector<bool> buttons )
{   
    //x button on ds4
    if ( buttons[0] )
    {
        SDL_SetRenderDrawColor( render, 124, 178, 232, 255 );
        drawCircleFill( render, (3 * SCREEN_WIDTH) / 4, SCREEN_HEIGHT / 2 + spacing, 15 );
    }
    //circle button on ds4
    if ( buttons[1] )
    {
        SDL_SetRenderDrawColor( render, 255, 102, 102, 255 );
        drawCircleFill( render, (3 * SCREEN_WIDTH) / 4 + spacing, SCREEN_HEIGHT / 2, 15 );
    }
    //squre button on ds4
    if ( buttons[2] ) 
    {
        SDL_SetRenderDrawColor( render, 0xF6, 0x9D, 0xC8, 0xFF );
        drawCircleFill( render, (3 * SCREEN_WIDTH) / 4 - spacing, SCREEN_HEIGHT / 2, 15 );
    }
    //triangle button
    if ( buttons[3] )
    {
        SDL_SetRenderDrawColor( render, 64, 226, 160, 255 );
        drawCircleFill( render, (3 * SCREEN_WIDTH) / 4, SCREEN_HEIGHT / 2 - spacing, 15 );

    }

    //all other buttons are just going to be white
    SDL_SetRenderDrawColor ( render, 0xFF, 0xFF, 0xFF, 0xFF );



    SDL_FRect rect = {};

    if ( buttons[4] )
    {
        rect = { SCREEN_WIDTH / 2 - (SCREEN_WIDTH/10), SCREEN_HEIGHT / 2, 20, 10};
        SDL_RenderFillRect( render, &rect);
    }

    if ( buttons[5] ) //ps4 button
    {
        drawCircleFill( render, SCREEN_WIDTH / 2, ( 3 * SCREEN_HEIGHT ) / 4, 20 );
    }

    if ( buttons[6] )
    {
        rect = { SCREEN_WIDTH / 2 + (SCREEN_WIDTH/10), SCREEN_HEIGHT / 2, 20, 10};
        SDL_RenderFillRect( render, &rect);
    }

    if ( buttons[9] )
    {
        rect = { ( SCREEN_WIDTH) / 4 - 20,  SCREEN_HEIGHT / 4 + 60, 60, 20 };
        SDL_RenderFillRect( render, &rect );
    }

    if ( buttons[10] )
    {
        rect = { (3 * SCREEN_WIDTH) / 4 - 20,  SCREEN_HEIGHT / 4 + 60, 60, 20 };
        SDL_RenderFillRect( render, &rect);
    }
}

//draws filled rectangle 
void drawHatsDown( SDL_Renderer * render, bool * hats )
{
    //all buttons will be white
    SDL_SetRenderDrawColor( render, 0xFF, 0xFF, 0xFF, 0xFF );
    //default hat
    SDL_FRect hat = { SCREEN_WIDTH / 4,  SCREEN_HEIGHT / 2, dpadSize, dpadSize };

    if ( hats[0] )
    {
        hat.y += spacing;
        SDL_RenderFillRect( render, &hat );
        hat.y -= spacing;
    }
    if ( hats[1] )
    {
        hat.x += spacing;
        SDL_RenderFillRect( render, &hat );
        hat.x -= spacing;
    }
    if ( hats[2] )
    {
        hat.x -= spacing;
        SDL_RenderFillRect( render, &hat );
        hat.x += spacing;
    }
    if ( hats[3] )
    {
        hat.y -= spacing;
        SDL_RenderFillRect( render, &hat );
        hat.y += spacing;
    }

}

//function draws back trigger buttons and fills based on their % pressed
void drawTriggers( SDL_Renderer * render, SDL_FRect * rect, Sint16 down )
{
    double frame = down + 32768;
    //SDL_Log("init frame: %lf\t", frame);
    frame = frame / (32768 * 2);
    SDL_SetRenderDrawBlendMode( render, SDL_BLENDMODE_BLEND );
    SDL_SetRenderDrawColor( render, 0xFF, 0xFF, 0xFF, frame * 255 );
    //SDL_Log("Frame: %lf\n", frame);

    SDL_RenderFillRect( render, rect );
}

void close(){
    //deallocate joystick
    if ( gamecontroller )
    {
        SDL_CloseJoystick( gamecontroller );
        gamecontroller = NULL;
    }

    //deallocate renderer and window
    SDL_DestroyRenderer( gRenderer );
    gRenderer = NULL;
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;

    //Quit SDL subsystem
    SDL_Quit();
}

int main( int argc, char * argv[] ){

    if( !init() ) SDL_Log( "SDL Failed to initialize!\n" );
    else{
        //mainloop flag
        bool quit = false;
        //event handler
        SDL_Event event;

        //left stick coords
        double lsX = 0.0;
        double lsY = 0.0;

        //right stick coords
        double rsX = 0.0;
        double rsY = 0.0;

        //back trigger buttons
        Sint16 Ltrigger = -32768;
        Sint16 Rtrigger = -32768;
        SDL_FRect ltrig = { SCREEN_WIDTH / 4 - 20,  SCREEN_HEIGHT / 4, 60, 40 };
        SDL_FRect rtrig = { (3 * SCREEN_WIDTH) / 4 - 20,  SCREEN_HEIGHT / 4, 60, 40 };

        double SDL_JOYSTICK_MAX_DOUBLE = static_cast<double> (SDL_JOYSTICK_AXIS_MAX);

        //stat array to check which buttons are being held
        int buttonLen = 15; // default amount of buttons if none detected
        if ( gamecontroller ) // if a controller is connected
        {
            buttonLen = SDL_GetNumJoystickButtons( gamecontroller );
        }
        std::vector<bool> button_held(buttonLen);
        for( int i = 0; i < buttonLen; i++) button_held[i] = false; //start all false


        //dpad buttons
        bool hat_held[4];
        for ( int i = 0; i < 4; i++) hat_held[i] = false;

        while( !quit ){
            
            while ( SDL_PollEvent(&event) != 0 ){
                
                //quit program condition
                if ( event.type == SDL_EVENT_QUIT ) quit = true;

                else if ( event.type == SDL_EVENT_JOYSTICK_AXIS_MOTION ){
                    //joystick movement
                    switch ( event.jaxis.axis ){
                        //get the percent of the total motion done

                        case 0://left stick x
                            if (event.jaxis.value < -JOYSTICK_DEAD_ZONE) lsX = event.jaxis.value / SDL_JOYSTICK_MAX_DOUBLE;
                            else if (event.jaxis.value > JOYSTICK_DEAD_ZONE) lsX = event.jaxis.value / SDL_JOYSTICK_MAX_DOUBLE;
                            else lsX = 0.0;
                            break;

                        case 1://left stick y
                            if (event.jaxis.value < -JOYSTICK_DEAD_ZONE) lsY = event.jaxis.value / SDL_JOYSTICK_MAX_DOUBLE;
                            else if (event.jaxis.value > JOYSTICK_DEAD_ZONE) lsY = event.jaxis.value / SDL_JOYSTICK_MAX_DOUBLE;
                            else lsY = 0.0;
                            break;

                        case 2://right stick x
                            if (event.jaxis.value < -JOYSTICK_DEAD_ZONE) rsX = event.jaxis.value / SDL_JOYSTICK_MAX_DOUBLE;
                            else if (event.jaxis.value > JOYSTICK_DEAD_ZONE) rsX = event.jaxis.value / SDL_JOYSTICK_MAX_DOUBLE;
                            else rsX = 0.0;
                            break;

                        case 3://right stick y
                            if (event.jaxis.value < -JOYSTICK_DEAD_ZONE) rsY = event.jaxis.value / SDL_JOYSTICK_MAX_DOUBLE;
                            else if (event.jaxis.value > JOYSTICK_DEAD_ZONE) rsY = event.jaxis.value / SDL_JOYSTICK_MAX_DOUBLE;
                            else rsY = 0.0;
                            break;

                        case 4: //left trigger button?
                            Ltrigger = event.jaxis.value;
                            //SDL_Log("Left trigger value: %d", Ltrigger);
                            break;

                        case 5: //right tigger button?
                            Rtrigger = event.jaxis.value;
                            //SDL_Log("Left trigger value: %d", Rtrigger);
                            break;

                        default:
                            break;
                    }
                }
                else if ( event.type == SDL_EVENT_JOYSTICK_BUTTON_DOWN)
                {   
                    SDL_Log("Button being held: %i\n", event.jbutton.button);
                    button_held[event.jbutton.button] = true;
                    
                }
                else if ( event.type == SDL_EVENT_JOYSTICK_BUTTON_UP )
                {
                    button_held[event.jbutton.button] = false;
                }
                
                else if ( event.type == SDL_EVENT_JOYSTICK_HAT_MOTION )
                {

                    for ( int i = 0; i < 4; i++)
                    {
                        if ( event.jhat.value & hatCases[i]) hat_held[i] = true;
                        else hat_held[i] = false;
                    }
                    
                }
               
            }
            //draw background skeleton
            drawBackround(gRenderer);
            
            //update screen
            //draw left joystick
            drawJoystickDot( gRenderer, lsX, lsY, pos_lsX, pos_s, stickRadius );
            //draw right joystick
            drawJoystickDot( gRenderer, rsX, rsY, pos_rsX, pos_s, stickRadius );

            //draw buttons being held down
            drawButtonsDown( gRenderer, button_held );
            //drawd dpad
            drawHatsDown( gRenderer, hat_held );
            //draw back trigger buttons
            drawTriggers( gRenderer, &ltrig, Ltrigger );
            drawTriggers( gRenderer, &rtrig, Rtrigger );
            
            SDL_RenderPresent(gRenderer);
        }

    }

    //deallcate stuff
    close();
    return 0;
}