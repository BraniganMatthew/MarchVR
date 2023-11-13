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
#include <queue>
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
#define SCK_VERSION2 0x0202

#pragma comment(lib, "ws2_32.lib")

std::queue<std::string> hub_command_q;
std::queue<std::string> driver_command_q;
std::queue<std::string> tracker_command_q;
SimpleBLE::Peripheral peripheral;
SOCKET serverSocketHub;
SOCKET serverSocketDriver;
SOCKET clientSocketHub;
SOCKET clientSocketDriver;

fd_set rfd;
struct timeval timeout;

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
    std::string temp(iss.str());
    std::vector<std::string> tokens;
    std::string token;

    while (std::getline(iss, token, ';')) {
        tokens.push_back(token);
    }

    if (tokens[2] == "GUI") {
        hub_command_q.push(temp);
        std::cout << temp << std::endl;
    }
    else if (tokens[2] == "DRV") {
        driver_command_q.push(temp);
        if (tokens[3] == "MOT") hub_command_q.push(temp);
        std::cout << temp << std::endl;
    }
    else if (tokens[2] == "TK1") {
        tracker_command_q.push(temp);
        std::cout << temp << std::endl;
        if (tokens[3] == "CAL") {
            std::cout << "Calibrate command from hub going to driver: " << temp << std::endl;
            driver_command_q.push(temp);
        }
    } else {
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

bool connect_to_esp_via_ble() {
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

    peripheral = search_for_esp32_and_connect_by_service(peripherals);

    if (peripheral.is_connected()) {
        std::cout << "Successfully connected." << std::endl;
    }
    else {
        std::cout << "Connection failed." << std::endl;
        return 1;
    }

    // Subscribe to the characteristic.
    peripheral.notify(serviceUUID, charUUID_DRV, [&](SimpleBLE::ByteArray bytes) {
        process_esp32_data(bytes);
        });
}

bool setup_hub_socket() {
    // Create the first socket (HUB)
    serverSocketHub = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketHub == INVALID_SOCKET) {
        std::cout << "Socket creation failed." << std::endl;
        WSACleanup();
        return false;
    }

    // Bind the socket to a local address and port
    SOCKADDR_IN serverAddrHub;
    serverAddrHub.sin_family = AF_INET;
    serverAddrHub.sin_addr.s_addr = INADDR_ANY;
    serverAddrHub.sin_port = htons(8080); // Replace with your desired port

    if (bind(serverSocketHub, (SOCKADDR*)&serverAddrHub, sizeof(serverAddrHub)) == SOCKET_ERROR) {
        std::cout << "Binding failed to Hub." << std::endl;
        closesocket(serverSocketHub);
        WSACleanup();
        return false;
    }
    return true;
}

bool setup_driver_socket() {
    // Create the second socket (DRIVER)
    serverSocketDriver = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketDriver == INVALID_SOCKET) {
        std::cout << "Socket creation failed." << std::endl;
        WSACleanup();
        return false;
    }
    sockaddr_in serverAddrDriver;
    serverAddrDriver.sin_family = AF_INET;
    serverAddrDriver.sin_addr.s_addr = INADDR_ANY;
    serverAddrDriver.sin_port = htons(8081);

    if (bind(serverSocketDriver, (sockaddr*)&serverAddrDriver, sizeof(serverAddrDriver)) == SOCKET_ERROR) {
        std::cout << "Binding failed to Driver." << std::endl;
        closesocket(serverSocketDriver);
        WSACleanup();
        return false;
    }
    return true;
}

bool listen_and_connect_hub() {
    std::cout << "Server listening on port 8080..." << std::endl;
    if (listen(serverSocketHub, 1) == SOCKET_ERROR) {
        std::cerr << "Listen failed on port 8080." << std::endl;
        closesocket(serverSocketHub);
        WSACleanup();
        return false;
    }

    std::cout << "Waiting for a connection..." << std::endl;

    clientSocketHub = accept(serverSocketHub, NULL, NULL);
    if (clientSocketHub == INVALID_SOCKET) {
        std::cerr << "Accept failed." << std::endl;
        closesocket(serverSocketHub);
        WSACleanup();
        return false;
    }

    std::cout << "Connected to MarchVR Hub." << std::endl;
    return true;
}

bool listen_and_connect_driver() {
    std::cout << "Server listening on port 8081..." << std::endl;

    if (listen(serverSocketDriver, 1) == SOCKET_ERROR) {
        std::cerr << "Listen failed on port 8081." << std::endl;
        closesocket(serverSocketDriver);
        WSACleanup();
        return false;
    }

    std::cout << "Waiting for a connection..." << std::endl;

    clientSocketDriver = accept(serverSocketDriver, NULL, NULL);
    if (clientSocketDriver == INVALID_SOCKET) {
        std::cerr << "Accept failed." << std::endl;
        closesocket(serverSocketDriver);
        WSACleanup();
        return false;
    }

    std::cout << "Connected to MarchVR Driver." << std::endl;
    return true;
}

int main()
{
//=====================CONNECT TO ESP VIA BLE===================
    if (!connect_to_esp_via_ble()) {
        std::cout << "Error in connect_to_esp_via_ble() function" << std::endl;
    }

//=================SETUP AND INITIALIZE SOCKETS=================

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    if (!setup_hub_socket()) {
        std::cout << "Error in setup_hub_socket() function" << std::endl;
    }
    if(!setup_driver_socket()) {
        std::cout << "Error in setup_driver_socket() function" << std::endl;
    }


//================START PYTHON SCRIPT===========================
    const char* command = "python MarchVR_Hub.py";

    std::thread t1(startup_marchvr_hub, command);

//================BEGIN TO LISTEN TO PORT=======================

    if (!listen_and_connect_hub()) {
        std::cout << "Error in listen_and_connect_hub() function" << std::endl;
    }
    
    if (!listen_and_connect_driver()) {
        std::cout << "Error in listen_and_connect_hub() function" << std::endl;
    }

//=============SET TIMEOUT FOR SELECT FUNCTION==================
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;

    while (1) {
        FD_ZERO(&rfd);
        FD_SET(clientSocketHub, &rfd); //maybe serverSocketHub
        int ret = select(clientSocketHub+1, &rfd, NULL, NULL, &timeout);
        char buffer[64];
        if (ret > 0) {
            std::cout << "We got data from hub!" << std::endl;
            memset(buffer, 0, sizeof(buffer)); //clear buffer
            int bytes_received = recv(clientSocketHub, buffer, sizeof(buffer) - 1, 0);
            std::string temp(buffer, sizeof(buffer));
            std::cout << temp << std::endl;
            process_esp32_data(temp);
        }
        if (!peripheral.is_connected()) {
            //reconnect with peripheral
            std::cout << "Peripheral device was disconnected! Attempting to reconnect." << std::endl;
            peripheral.connect();
            std::cout << "Connecting to " << peripheral.identifier() << " [" << peripheral.address() << "]" << " via serviceUUID: " << serviceUUID << std::endl;
        }
        while (!driver_command_q.empty()) {
            send(clientSocketDriver, driver_command_q.front().c_str(), driver_command_q.front().size(), 0);
            driver_command_q.pop();
            
        }
        while (!hub_command_q.empty()) {
            send(clientSocketHub, hub_command_q.front().c_str(), hub_command_q.front().size(), 0);
            hub_command_q.pop();
        }
        while (!tracker_command_q.empty()) {
            //send via bluetooth
            peripheral.write_request(serviceUUID, charUUID_TRK, tracker_command_q.front());
            tracker_command_q.pop();
        }
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