#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <ws2bth.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <bluetoothapis.h>

#define PORT 80
#define DEFAULT_PORT "27015"

const char* ESP32MACaddress = "E8:9F:6D:26:9F:1A";

int str2ba(const char* straddr, BTH_ADDR* btaddr) {
    int i;
    unsigned int aaddr[6];
    BTH_ADDR tmpaddr = 0;

    if (sscanf(straddr, "%02x:%02x:%02x:%02x:%02x:%02x",
        &aaddr[0], &aaddr[1], &aaddr[2],
        &aaddr[3], &aaddr[4], &aaddr[5]) != 6)
        return 1;
    *btaddr = 0;
    for (i = 0; i < 6; i++) {
        tmpaddr = (BTH_ADDR)(aaddr[i] & 0xff);
        *btaddr = ((*btaddr) << 8) + tmpaddr;
    }
    return 0;
}

int main() {

	//init winsock
	WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
	int wsaError = WSAStartup(wVersionRequested, &wsaData);
	if (wsaError != 0) {
        // Tell the user that we could not find a usable WinSock DLL.
        printf("The Winsock dll not found!\n");
        return 0;
	}
    else {
        printf("The Winsock dll found!\n");
        printf("The status: %s.\n", wsaData.szSystemStatus);
    }

    //check to see if supports 2.2
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        //could not find usable dll
        printf("The dll do not support the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
        WSACleanup();
        return 0;
    } else {
        //proper version found
        printf("The dll supports the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
        printf("The highest version this dll can support: %u.%u\n", LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
    }

    //create socket
    SOCKET pc_socket;
    pc_socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM); //Could be different
    // Check for errors to ensure that the socket is a valid socket.
    if (pc_socket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return 0;
    } else {
        printf("socket() is OK!\n");
    }


    //bind the socket
    SOCKADDR_BTH service = { 0 };
    service.addressFamily = AF_BTH;
    service.serviceClassId = RFCOMM_PROTOCOL_UUID;
    service.port = BT_PORT_ANY;
    str2ba(ESP32MACaddress, &service.btAddr);
    if (bind(pc_socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) { //bind or connect?
        printf("bind() failed: %ld.\n", WSAGetLastError());
        closesocket(pc_socket);
        return 0;
    }
    else {
        printf("bind() is OK!\n");
    }

    //listens and checks for any errors
    if (listen(pc_socket, 1) == SOCKET_ERROR)
        printf("listen(): Error listening on socket %ld.\n", WSAGetLastError());
    else {
        printf("listen() is OK, I'm waiting for connections...\n");
    }
    
	ExitProcess(EXIT_SUCCESS);
}

