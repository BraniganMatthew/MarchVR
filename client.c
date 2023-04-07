// Jibin Alex
// COP4600 Exercise 9
// Client

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

int main(int argc, const char* argv[]) {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != NO_ERROR)
         printf("Client: Error at WSAStartup().\n");
    else
         printf("Client: WSAStartup() is OK.\n");
    // Check if a port number was provided
    if (argc == 1) {
        printf("A port number was not specified for the client program!");
        return 1;
    }
    else {
        // Create client socket
        int portNumber = atoi(argv[1]);
        SOCKET clientSocket;
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            printf("Error creating client socket!");
            return 1;
        }

        // Specify server IP address and port
        struct sockaddr_in serverAddress;
        serverAddress.sin_port = htons(portNumber);
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        // Make connection using socket and server address
        int connection = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
        
        // Send message to server
        char message[] = "Jibin Alex:41923371";
        send(clientSocket, message, sizeof(message), 0);

        // Receive welcome message sent by server
        char array[13];
        char* receivedString = array; // The intended message to be received is 20 characters long + the null terminator so 43 total
        recv(clientSocket, receivedString, sizeof(array), 0);
        printf("%s", array);

        // Cleanup
        #ifdef _WIN32
            closesocket(clientSocket);
            WSACleanup();
        #else
            close(clientSocket);
        #endif
        return 0;
    }
}