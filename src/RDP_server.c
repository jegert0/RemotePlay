#include <gst/gst.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <stdint.h>

#pragma comment(lib, "ws2_32.lib")

//defining default ports if non given in execution
int stream_port = 5000;
int control_port = 5050;
char *stream_ip = "127.0.0.1";

    static GMainLoop *loop;

static gboolean my_bus_callback (GstBus * bus, GstMessage * msg, gpointer data)
{
    //g_print ("Got %s message: \n", GST_MESSAGE_TYPE_NAME (msg));

    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;

            gst_message_parse_error (msg, &err, &debug);
            g_print("Error: %s\n", err->message);
            g_error_free(err);
            g_free(debug);

            g_main_loop_quit (loop);
            break;
        }

        case GST_MESSAGE_EOS:
        //end of stream
        g_main_loop_quit(loop);
        break;
        default:
        //unhandeled message
        break;
    }

    //want to be notified again the next time there is a message
    return TRUE;
}

DWORD WINAPI start_gstream_thread (LPVOID arg)
{
    // int argc = ((int *)arg)[0];
    // char **argv = (char **)(((int*)arg) + 1);

    GstElement *pipeline;
    GError *error = NULL;
    GstBus *bus;
    guint bus_watch_id;

    gst_init(NULL, NULL);
    //set up and lanuch pipeline
    gchar pipeline_dsc[1024];
    snprintf(pipeline_dsc, 1024, "d3d11screencapturesrc show-cursor=true \
        ! videoconvert ! queue ! x264enc tune=zerolatency \
        ! rtph264pay ! udpsink host=%s port=%d", stream_ip, stream_port ); 
    pipeline = gst_parse_launch(pipeline_dsc, &error);

    if (!pipeline){
        g_printerr("Pipeline Error: %s", error->message);
        g_error_free (error);
        return 1;
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    printf("Stream started...\n");

    bus = gst_element_get_bus(pipeline);
    bus_watch_id = gst_bus_add_watch(bus, my_bus_callback, NULL);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    //stop streaming
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    gst_object_unref(bus);
    g_source_remove(bus_watch_id);
    g_main_loop_unref(loop);
    return 0;
    
}


// typedef enum 
// {
//     Keyboard,
//     Mouse
// } DataType;

typedef uint8_t DataType;
const DataType Keyboard = 0;
const DataType Mouse = 1;

typedef struct 
{
    DataType type;
    USHORT vkey;
    USHORT flags;
    USHORT scanCode;
}   Keyboard_Packet;

typedef struct 
{
    DataType type;
    int16_t dx; //relative movements
    int16_t dy;
    USHORT buttons; //button click events
    USHORT buttonData; // mouse wheel data
    USHORT flags;

}   Mouse_Packet;


void simulate_keyboard(Keyboard_Packet * pkt)
{
    INPUT input = {0};
    //clean memory in stack every time
    ZeroMemory(&input, sizeof(INPUT));

    input.type = INPUT_KEYBOARD;
    input.ki.wScan = pkt->scanCode;

    DWORD flags = KEYEVENTF_SCANCODE;

    if (pkt->flags & RI_KEY_BREAK) //key release
        flags |= KEYEVENTF_KEYUP;

    if  (pkt->flags & RI_KEY_E0) //extend key
        flags |= KEYEVENTF_EXTENDEDKEY;


    input.ki.dwFlags = flags;
    SendInput(1, &input, sizeof(INPUT));
}

void simulate_mouse(Mouse_Packet *pkt)
{
    
    INPUT input;
    ZeroMemory(&input, sizeof(INPUT));

    input.type = INPUT_MOUSE;

    DWORD flags = 0;

    if(pkt->dx || pkt->dy)
    {
        input.mi.dx = pkt->dx;
        input.mi.dy = pkt->dy;
        flags |= MOUSEEVENTF_MOVE;
    }

    //button presses
    flags |= pkt->buttons;

    /* mouse wheel */
    if(pkt->buttons & RI_MOUSE_WHEEL)
    {
        flags |= MOUSEEVENTF_WHEEL;
        input.mi.mouseData = pkt->buttonData;
    }

    //must translate raw input flags to send input flags

    if(pkt->buttons & RI_MOUSE_LEFT_BUTTON_DOWN)
        flags |= MOUSEEVENTF_LEFTDOWN;

    if(pkt->buttons & RI_MOUSE_LEFT_BUTTON_UP)
        flags |= MOUSEEVENTF_LEFTUP;

    if(pkt->buttons & RI_MOUSE_RIGHT_BUTTON_DOWN)
        flags |= MOUSEEVENTF_RIGHTDOWN;

    if(pkt->buttons & RI_MOUSE_RIGHT_BUTTON_UP)
        flags |= MOUSEEVENTF_RIGHTUP;

    if(pkt->buttons & RI_MOUSE_MIDDLE_BUTTON_DOWN)
        flags |= MOUSEEVENTF_MIDDLEDOWN;

    if(pkt->buttons & RI_MOUSE_MIDDLE_BUTTON_UP)
        flags |= MOUSEEVENTF_MIDDLEUP;


    input.mi.dwFlags = flags;
    SendInput(1, &input, sizeof(INPUT));
}


int main(int argc, char * argv[])
{   if (argc == 4) {
        stream_ip = argv[1];
        stream_port = atoi(argv[2]);
        control_port = atoi(argv[3]);
        printf("Starting stream to %s on ports %d and %d\n", stream_ip, stream_port, control_port);
    }
    else if (argc == 2){
        stream_ip = argv[1];
        printf("Starting stream to %s on default ports %d and %d\n", stream_ip, stream_port, control_port);
    }
    else {
        printf("proper use of execution: .\\server.exe <IP> <port> <port>\n");
        return 1;
    }

    /*  destination descided, safe to start gstream thread 
        keyboard inputs will be handeled this thread    */
    HANDLE gst_thread;
    gst_thread = CreateThread (
        NULL,
        0,
        start_gstream_thread,
        NULL,
        0,
        NULL
    );

    if (gst_thread) {
        printf("Stream thread created successfully...\n");
    }
    else {
        perror("Stream thread could not be created!!!\n");
        exit(1);
    }   
    

    
    //init winsock
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server_addr, client_addr;
    int client_size = sizeof(client_addr);

    WSAStartup(MAKEWORD(2,2), &wsaData);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    //make socket non binding
    // u_long mode = 1;
    // ioctlsocket(sock, FIONBIO, &mode);
    //we want blocking, this threads only purpose is to recieve keyboard and mouse data to simulate

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)); 

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; //accept any ip
    server_addr.sin_port = htons(control_port);

    if (bind(sock, (SOCKADDR *) &server_addr, sizeof(server_addr))) {
        printf("bind failed with error %d\n", WSAGetLastError());
        exit(1);
    }

    printf("UDP SERVER up and listening...\n");

    //listen for udp input packets - blocking
    uint8_t buffer[1024]; //generic buffer that will be turned into different struct depending on mouse or keyboard data
    while (1) {
        int bytes = recvfrom(sock, (char*) &buffer, sizeof(buffer), 0, (struct sockaddr*) &client_addr, &client_size);

        if (bytes > 0) {
            DataType type = buffer[0];
           
            
            if (type == Keyboard) {
                Keyboard_Packet *pkt = (Keyboard_Packet *) buffer;
                simulate_keyboard(pkt);
            }
            else if ( type == Mouse ) {
                Mouse_Packet *pkt = (Mouse_Packet *) buffer;
                simulate_mouse(pkt);
            }
            else printf("Unkown packet\n");
            // switch(type) {
                
            //     case Keyboard: {
            //         Keyboard_Packet *pkt = (Keyboard_Packet *) buffer;
            //         simulate_keyboard(pkt);
            //         break;
            //     }

            //     case Mouse: {
            //         Mouse_Packet *pkt = (Mouse_Packet *) buffer;
            //         simulate_mouse(pkt);
            //         break;
            //     }

            //     default:
            //         printf("Unkown packet\n");
                
            // }
            
        }
        //Sleep(1); //
    }
    
    closesocket(sock);
    WSACleanup();
    return 0;
}

