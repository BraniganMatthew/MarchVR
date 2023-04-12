/*#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2bth.h>
#include <string>

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
	SOCKADDR_BTH sockAddr;
	SOCKET btSocket;
	int error;


	btSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.addressFamily = AF_BTH;
	sockAddr.serviceClassId = RFCOMM_PROTOCOL_UUID;
	sockAddr.port = BT_PORT_ANY;
	//sockAddr.btAddr = <your bluetooth address>
    str2ba(ESP32MACaddress, &sockAddr.btAddr);
	error = connect(btSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr));
    if (error == SOCKET_ERROR) printf("Error Connecting");
    else printf("Connection established");
    return 0;
}

*/


/*
//listens and checks for any errors
if (listen(pc_socket, 1) == SOCKET_ERROR)
    printf("listen(): Error listening on socket %ld.\n", WSAGetLastError());
else {
    printf("listen() is OK, I'm waiting for connections...\n");
}


// Create a temporary SOCKET object called AcceptSocket for accepting connections.
SOCKET AcceptSocket;

// Create a continuous loop that checks for connections requests. If a connection
// request occurs, call the accept function to handle the request.
printf("Server: Waiting for a client to connect...\n");
printf("***Hint: Server is ready...run your client program...***\n");

// Do some verification...
while (1) {
    AcceptSocket = SOCKET_ERROR;
    while (AcceptSocket == SOCKET_ERROR) {
        AcceptSocket = accept(pc_socket, NULL, NULL);
    }
    // else, accept the connection...
    // When the client connection has been accepted, transfer control from the
    // temporary socket to the original socket and stop checking for new connections.
    printf("Server: Client Connected!\n");
    pc_socket = AcceptSocket;
    break;
}
*/