/**
Refererences:
Helpful examples on getting connected devices - https://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4k.html
Bluetooth APIs - https://docs.microsoft.com/en-us/windows/win32/api/bluetoothapis/
Microsoft example - https://github.com/microsoftarchive/msdn-code-gallery-microsoft/blob/master/Official%20Windows%20Platform%20Sample/Bluetooth%20connection%20sample/%5BC%2B%2B%5D-Bluetooth%20connection%20sample/C%2B%2B/bthcxn.cpp
Bluetooth programming with Windows sockets - https://docs.microsoft.com/en-us/windows/win32/bluetooth/bluetooth-programming-with-windows-sockets
**/


#include <stdlib.h>
#include <stdio.h>
#include <Winsock2.h>
#include <sstream>
#include <string>
#include <iostream> 
#include <thread>

#include "utils.hpp"
#include "simpleble/SimpleBLE.h"

//BTH_ADDR esp32BtAddress;
//SOCKADDR_BTH btSocketAddress;
//SOCKET btClientSocket;
using namespace std::chrono_literals;

#define serviceUUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define charUUID     "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define charUUID_TRK "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define charUUID_GUI "aad41096-f795-4b3b-83bb-858051e5e284"
#define charUUID_DRV "22d7a034-791d-49f6-a84e-ef78ab2473ad"

#pragma comment(lib, "ws2_32.lib")

SimpleBLE::Peripheral search_for_esp32_and_connect_by_service(std::vector<SimpleBLE::Peripheral>& peripherals) {
    for (auto& peripheral : peripherals) {
        for (auto& service : peripheral.services()) {
            if (service.uuid() == serviceUUID) {
                peripheral.connect();
                std::cout << "Connecting to " << peripheral.identifier() << " [" << peripheral.address() << "]" << " via serviceUUID: " << serviceUUID << std::endl;
                return peripheral;
            }
        }
    }
}

void process_esp32_data(SimpleBLE::ByteArray& bytes) {
    std::istringstream iss(bytes);
    std::vector<std::string> tokens;
    std::string token;

    while (std::getline(iss, token, ';')) {
        tokens.push_back(token);
    }

    if (tokens.size() >= 8 && tokens[1] == "TK1") {
        // Extract the speed value (assuming it's at index 5)
        double speed = std::stod(tokens[8]);
        std::cout << "Speed: " << speed << std::endl;
    }
    else {
        std::cout << "Invalid format." << std::endl;
    }
}

void startup_marchvr_hub(const char* command) {
    int returnCode = std::system(command);

    if (returnCode == 0) {
        std::cout << "MarchVR_Hub started successfully." << std::endl;
    }
    else {
        std::cout << "Error starting MarchVR_Hub Python script." << std::endl;
    }
}

int main()
{
//=====================CONNECT TO ESP VIA BLE===================
    auto adapter_optional = Utils::getAdapter();

    if (!adapter_optional.has_value()) {
        return EXIT_FAILURE;
    }

    auto adapter = adapter_optional.value();

    std::vector<SimpleBLE::Peripheral> peripherals;

    adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral peripheral) { peripherals.push_back(peripheral); });

    adapter.set_callback_on_scan_start([]() { std::cout << "Scan started." << std::endl; });
    adapter.set_callback_on_scan_stop([]() { std::cout << "Scan stopped." << std::endl; });
    // Scan for 5 seconds and return.
    adapter.scan_for(5000);

    // Print found devices
    std::cout << "The following devices were found:" << std::endl;
    for (size_t i = 0; i < peripherals.size(); i++) {
        std::cout << "[" << i << "] " << peripherals[i].identifier() << " [" << peripherals[i].address() << "]"
            << "Signal Strength: " << peripherals[i].rssi() << "dBm" << std::endl;
    }

    auto peripheral = search_for_esp32_and_connect_by_service(peripherals);

    if (peripheral.is_connected()) {
        std::cout << "Successfully connected." << std::endl;
    }
    else {
        std::cout << "Connection failed." << std::endl;
        return 1;
    }

    // Subscribe to the characteristic.
    peripheral.notify(serviceUUID, charUUID_DRV, [&](SimpleBLE::ByteArray bytes){
        //SimpleBLE::ByteArray test = bytes;
        process_esp32_data(bytes);
    });

    //=================SETUP AND INITIALIZE SOCKETS======================
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // Create the first socket (HUB)
    SOCKET serverSocketHub = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketHub == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind the socket to a local address and port
    sockaddr_in serverAddrHub;
    serverAddrHub.sin_family = AF_INET;
    serverAddrHub.sin_addr.s_addr = INADDR_ANY;
    serverAddrHub.sin_port = htons(8080); // Replace with your desired port

    if (bind(serverSocketHub, (sockaddr*)&serverAddrHub, sizeof(serverAddrHub)) == SOCKET_ERROR) {
        std::cerr << "Binding failed to Hub." << std::endl;
        closesocket(serverSocketHub);
        WSACleanup();
        return 1;
    }

    // Create the second socket (DRIVER)
    SOCKET serverSocketDriver = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketDriver == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }
    sockaddr_in serverAddrDriver;
    serverAddrDriver.sin_family = AF_INET;
    serverAddrDriver.sin_addr.s_addr = INADDR_ANY;
    serverAddrDriver.sin_port = htons(8081);

    if (bind(serverSocketDriver, (sockaddr*)&serverAddrDriver, sizeof(serverAddrDriver)) == SOCKET_ERROR) {
        std::cerr << "Binding failed to Driver." << std::endl;
        closesocket(serverSocketDriver);
        WSACleanup();
        return 1;
    }
//================START PYTHON SCRIPT===========================
    const char* command = "python MarchVR_Hub.py";

    std::thread t1(startup_marchvr_hub, command);

//================BEGIN TO LISTEN TO PORT=======================
    std::cout << "Server listening on port 8080..." << std::endl;

    if (listen(serverSocketHub, 1) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(serverSocketHub);
        WSACleanup();
        return 1;
    }

    std::cout << "Waiting for a connection..." << std::endl;

    SOCKET client_socket_hub = accept(serverSocketHub, NULL, NULL);
    if (client_socket_hub == INVALID_SOCKET) {
        std::cerr << "Accept failed." << std::endl;
        closesocket(serverSocketHub);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to MarchVR Hub." << std::endl;

    std::cout << "Server listening on port 8081..." << std::endl;

    if (listen(serverSocketDriver, 1) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(serverSocketDriver);
        WSACleanup();
        return 1;
    }

    std::cout << "Waiting for a connection..." << std::endl;

    SOCKET client_socket_driver = accept(serverSocketDriver, NULL, NULL);
    if (client_socket_driver == INVALID_SOCKET) {
        std::cerr << "Accept failed." << std::endl;
        closesocket(serverSocketDriver);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to MarchVR Driver." << std::endl;

    while (1) {
        char buffer[128];
        int bytes_received = recv(client_socket_hub, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }
        std::cout << "Received on socket: " << std::string(buffer, bytes_received) << std::endl << std::endl;

        std::string response = "This message is from c++ socket application";
        send(client_socket_hub, response.c_str(), response.size(), 0);
        std::this_thread::sleep_for(5s);
    }

    // Cleanup the BLE connection
    peripheral.unsubscribe(serviceUUID, charUUID);
    peripheral.disconnect();

    // Close the server socket and clean up
    closesocket(serverSocketHub);
    closesocket(serverSocketDriver);
    WSACleanup();

    return 0;
}