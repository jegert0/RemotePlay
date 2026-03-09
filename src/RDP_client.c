#include <gst/gst.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdint.h>

#pragma comment(lib, "ws2_32.lib")

int control_port = 5050;
int stream_port = 5000;
char *stream_ip = "127.0.0.1";

//recieve gstream and display, scan and send keyboard and mouse data

static GMainLoop *loop;
SOCKET udp_sock;
struct sockaddr_in server_addr;


void send_packet(const void* data, int size)
{
    int r = sendto(udp_sock, data, size, 0,
               (struct sockaddr*)&server_addr,
               sizeof(server_addr));

    if (r == SOCKET_ERROR) {
        printf("sendto failed: %d\n", WSAGetLastError());
    }
}

// typedef enum 
// {
//     Keyboard,
//     Mouse
// } DataType;
typedef uint8_t DataType;
const DataType Keyboard = 0;
const DataType Mouse = 1;


void handle_raw_input(LPARAM lparam)
{
    //using fixed stack buffer, keyboard and mouse input size is known and common
    //faster than dyamic allocation or global pointer
    RAWINPUT raw;
    UINT dwSize = sizeof(RAWINPUT);
    GetRawInputData((HRAWINPUT) lparam, RID_INPUT, &raw, &dwSize, sizeof(RAWINPUTHEADER));

    if (raw.header.dwType == RIM_TYPEKEYBOARD) {
        RAWKEYBOARD *k = &raw.data.keyboard;

        struct {
            DataType type;
            USHORT vkey;
            USHORT flags;
            USHORT scanCode;
        }packet;
    
        packet.type = Keyboard;
        packet.vkey = k->VKey;
        packet.flags = k->Flags;
        packet.scanCode = k->MakeCode;

        send_packet(&packet, sizeof(packet)); //send data
    }
    else if (raw.header.dwType == RIM_TYPEMOUSE) {
        RAWMOUSE *m = &raw.data.mouse;
        if(m->lLastX == 0 && m->lLastY == 0 && m->usButtonFlags == 0)
            return;

        struct {
            DataType type;
            int16_t dx; //relative movements
            int16_t dy;
            USHORT buttons; //button click events
            USHORT buttonData; // mouse wheel data
            USHORT flags; //how
        } packet;

        packet.type = Mouse;
        packet.dx = m->lLastX;
        packet.dy = m->lLastY;
        packet.buttons = m->usButtonFlags;
        packet.buttonData = m->usButtonData;
        packet.flags = m->usFlags;

        send_packet(&packet, sizeof(packet));
    }

}

//callback function that triggers when window receives message from os
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_INPUT) {
        handle_raw_input(lParam);
    }
    
    //let windows take care of every other message
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

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

    //so we are notified for other messages
    return TRUE;
}

DWORD WINAPI recv_gstream (LPVOID args)
{
    GstElement *pipeline;
    GError *error = NULL;
    GstBus *bus;
    guint bus_watch_id;

    gst_init(NULL, NULL);

    gchar pipeline_desc[1024];
    snprintf(pipeline_desc, 1024, "udpsrc port=%d ! application/x-rtp,encoding-name=H264 ! \
    rtph264depay ! avdec_h264 ! autovideosink", stream_port);

    pipeline = gst_parse_launch(pipeline_desc, &error);
    if (!pipeline) {
        g_printerr("Pipeline Error: %s", error->message);
        g_error_free (error);
        return 1;
    }

    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    printf("Pipeline is playing\n");

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

int main(int argc, char * argv[]) 
{
    if (argc == 4) {
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
        printf("proper use of execution: .\\client.exe <IP> <port> <port>\n");
        return 1;
    }

    //start receiving stream
    HANDLE gst_thread;
    gst_thread = CreateThread (
        NULL,
        0,
        recv_gstream,
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

    //winsock init
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(control_port);
    inet_pton(AF_INET, stream_ip, &server_addr.sin_addr);

    //create hidden window where keyboard and mouse messages will go to
    WNDCLASS wc = {0};
    wc.lpfnWndProc =  WndProc;
    wc.lpszClassName = "RawInputClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        "RawInputClass", "Hidden",
        0, 0,0,0,0,
        NULL, NULL, NULL, NULL
    );

    //register raw input devices
    RAWINPUTDEVICE rid[2];

    //keyboard
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x06;
    rid[0].dwFlags = RIDEV_INPUTSINK;
    rid[0].hwndTarget = hwnd;

    //mouse
    rid[1].usUsagePage = 0x01;
    rid[1].usUsage = 0x02;
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = hwnd;

    RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));

    printf("Async Raw input sender started...\n");

    //message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    closesocket(udp_sock);
    WSACleanup();
    return 0;
 
}