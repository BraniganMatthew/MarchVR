// This program connects to the ESP32 Feather v2 utilizing
// Winsock and another program that enables bluetooth connection
// on the ESP32 itself through arduino. This is crucial to our 
// project as it allows direct implimentation into SteamVR w/o 
// the bottlenecks of arduino and python

// Created by Richard Cusolito

#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <ws2bth.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <bluetoothapis.h>

// ESP32 Address
const char* ESP32MACaddress = "E8:9F:6D:26:9F:1A";

// Function to change ESP32 Address into proper form
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

	// Initialize winsock
	WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
	int wsaError = WSAStartup(wVersionRequested, &wsaData);
    // Error checking
	if (wsaError != 0) {
        printf("The Winsock dll not found!\n");
        return 0;
	}
    else {
        printf("The Winsock dll found!\n");
        printf("The status: %s.\n", wsaData.szSystemStatus);
    }

    // Error checking to see if it supports 2.2
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("The dll do not support the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
        WSACleanup();
        return 0;
    } else {
        printf("The dll supports the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
        printf("The highest version this dll can support: %u.%u\n", LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
    }

    // Create socket on pc
    SOCKET pc_socket;
    pc_socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    // Error checking
    if (pc_socket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return 0;
    } else {
        printf("socket() is OK!\n");
    }


    // Connect the socket to ESP32
    // See https://learn.microsoft.com/en-us/windows/win32/api/ws2bth/ns-ws2bth-sockaddr_bth for more info
    // Creates a proper socket address for bt service
    SOCKADDR_BTH service = { 0 };
    service.addressFamily = AF_BTH; 
    service.serviceClassId = RFCOMM_PROTOCOL_UUID; 
    service.port = BT_PORT_ANY;
    str2ba(ESP32MACaddress, &service.btAddr);
    // Connects and checks for errors
    if (connect(pc_socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) { 
        printf("connect() failed: %ld.\n", WSAGetLastError());
        int close_error = closesocket(pc_socket);
        if (close_error == SOCKET_ERROR)
            wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 0;
    }
    else {
        printf("connect() is OK!\n");
    }

    while (1) {}

	ExitProcess(EXIT_SUCCESS);
}


