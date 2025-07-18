CC = g++
CL = cl
CFLAGS = -IC:\mingw_dev_lib\include -LC:\mingw_dev_lib\lib -w "-Wl,-subsystem,windows" -lmingw32  -lSDL3
DEPS = includes.hpp emulate.hpp emulate.cpp remoteTest.cpp RemotePlayer.cpp
OBJs = includes.o emulate.o emulate.o remoteTest.o RemotePlayer.o
#need msvc ti compile these, vigem is dependant on msvc
BUS_DEPENDANT = emulation.cpp
VIFLAGS = /W4 /EHsc /I"C:\mingw_dev_lib\include" /I"C:\Grams\RemotePlayer\vcpkg\installed\x64-windows\include" /link /LIBPATH:"C:\Grams\RemotePlayer\vcpkg\installed\x64-windows\lib" /LIBPATH:"C:\mingw_dev_lib\lib\x86"
LIBS = setupapi.lib ViGEmClient.lib user32.lib SDL3.lib
MSINCLUDE = /I"C:\mingw_dev_lib\include" /I"C:\Grams\RemotePlayer\vcpkg\installed\x64-windows\include" /I"C:\mingw_dev_lib\SDL3-VC\include"
LIBPATHS = /LIBPATH:"C:\Grams\RemotePlayer\vcpkg\installed\x64-windows\lib" /LIBPATH:"C:\mingw_dev_lib\SDL3-VC\lib\x64"
BOOSTIN = /I"C:\mingw_dev_lib\boost_1_88_0"

VIGEM_LIB = /LIBPATH:"C:\Grams\RemotePlayer\vcpkg\installed\x64-windows\lib"
SERVER_DEPS = RemotePlayer_server.cpp emulation.cpp connection.cpp
SERVER_INC = /I"C:\mingw_dev_lib\include" /I".\vcpkg\installed\x64-windows\include" /I"C:\mingw_dev_lib\boost_1_88_0"
SERVER_LIBS = setupapi.lib ViGEmClient.lib user32.lib

CLIENT_DEPS = RemotePlayer_client.cpp connection.cpp emulation.cpp
CLIENT_INC = /I"C:\mingw_dev_lib\include" /I"C:\mingw_dev_lib\boost_1_88_0"
CLIENT_LIBS = user32.lib SDL3.lib
CLIENT_LIBPATHS = /LIBPATH:"C:\mingw_dev_lib\SDL3-VC\lib\x64"

RemotePlayer: RemotePlayer.cpp emulation.cpp
	$(CL) /FeRemotePlayer.exe /D_AMD64_ RemotePlayer.cpp emulation.cpp /W4 /EHsc $(MSINCLUDE) /link /SUBSYSTEM:CONSOLE $(LIBPATHS) $(LIBS)
	del /Q *.obj

Server: $(SERVER_DEPS) #uses amd4 architecture to build "/D_AMD64"
	$(CL) /FeServer.exe /W4 /EHsc /D_AMD64_ $(SERVER_DEPS) $(SERVER_INC) /link $(VIGEM_LIB) $(SERVER_LIBS)
	del /Q *.obj

Client: 
	$(CL) /FeClient.exe /W4 /EHsc /D_AMD64_ $(CLIENT_DEPS) $(CLIENT_INC) /link $(VIGEM_LIB) $(CLIENT_LIBPATHS) $(LIBS)
	del /Q *.obj

boostTest: boostTest.cpp
	$(CL) /FeboostTest.exe boostTest.cpp $(BOOSTIN) /EHsc
	del /Q boostTest.obj

remoteTest: remoteTest.cpp
	$(CC) -o remoteTest remoteTest.cpp $(CFLAGS) -Wall

msvcTest: remoteTest.cpp
	$(CL) /EHsc remoteTest.cpp /FemsvcTest.exe /I"C:\mingw_dev_lib\SDL3-VC\include" /link /SUBSYSTEM:CONSOLE /LIBPATH:"C:\mingw_dev_lib\SDL3-VC\lib\x64" SDL3.lib
	del /Q remoteTest.obj

udpServer: udpServerTest.cpp
	$(CL) /EHsc udpServerTest.cpp $(BOOSTIN) /link ws2_32.lib
	del /Q udpServerTest.obj

udpClient: udpClientTest.cpp
	$(CL) /EHsc udpClientTest.cpp $(BOOSTIN)

clean: *.obj
	del /Q *.obj