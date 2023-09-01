// this program connects to the esp32 feather v2 utilizing
// winsock and another program that enables bluetooth connection
// on the esp32 itself through arduino. this is crucial to our 
// project as it allows direct implimentation into steamvr w/o 
// the bottlenecks of arduino and python
//
// created by richard cusolito
//
//#define _crt_secure_no_warnings
//#pragma comment(lib, "ws2_32.lib")
//#pragma execution_character_set("utf-8")
//#include <winsock2.h>
//#include <ws2bth.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <bluetoothapis.h>
//#include <string>
//
// esp32 address
//const char* esp32macaddress1 = "e8:9f:6d:26:9f:1a";
//const char* esp32macaddress2 = "e8:9f:6d:2f:27:d6";
//
// function to change esp32 address into proper form
//int str2ba(const char* straddr, bth_addr* btaddr) {
//    int i;
//    unsigned int aaddr[6];
//    bth_addr tmpaddr = 0;
//
//    if (sscanf(straddr, "%02x:%02x:%02x:%02x:%02x:%02x",
//        &aaddr[0], &aaddr[1], &aaddr[2],
//        &aaddr[3], &aaddr[4], &aaddr[5]) != 6)
//        return 1;
//    *btaddr = 0;
//    for (i = 0; i < 6; i++) {
//        tmpaddr = (bth_addr)(aaddr[i] & 0xff);
//        *btaddr = ((*btaddr) << 8) + tmpaddr;
//    }
//    return 0;
//}
//
//int main() {
//
//	 initialize winsock
//	wsadata wsadata;
//    word wversionrequested = makeword(2, 2);
//	int wsaerror = wsastartup(wversionrequested, &wsadata);
//     error checking
//	if (wsaerror != 0) {
//        printf("the winsock dll not found!\n");
//        return 0;
//	}
//    else {
//        printf("the winsock dll found!\n");
//        printf("the status: %s.\n", wsadata.szsystemstatus);
//    }
//
//     error checking to see if it supports 2.2
//    if (lobyte(wsadata.wversion) != 2 || hibyte(wsadata.wversion) != 2) {
//        printf("the dll do not support the winsock version %u.%u!\n", lobyte(wsadata.wversion), hibyte(wsadata.wversion));
//        wsacleanup();
//        return 0;
//    } else {
//        printf("the dll supports the winsock version %u.%u!\n", lobyte(wsadata.wversion), hibyte(wsadata.wversion));
//        printf("the highest version this dll can support: %u.%u\n", lobyte(wsadata.whighversion), hibyte(wsadata.whighversion));
//    }
//
//    
//    
//
//     connect the socket to esp32
//     see https://learn.microsoft.com/en-us/windows/win32/api/ws2bth/ns-ws2bth-sockaddr_bth for more info
//     creates a proper socket address for bt service
//    
//    sockaddr_bth service = { 0 };
//    while (1) {
//         create socket on pc
//        socket pc_socket;
//        pc_socket = socket(af_bth, sock_stream, bthproto_rfcomm);
//        set timeout to 5ms
//        int timeout_ms = 5;
//        setsockopt(pc_socket, sol_socket, so_rcvtimeo, (char*)&timeout_ms, sizeof(timeout_ms));
//         error checking
//        if (pc_socket == invalid_socket) {
//            printf("error at socket(): %ld\n", wsagetlasterror());
//            wsacleanup();
//            return 0;
//        } else {
//            printf("socket() is ok!\n");
//        }
//
//        memset(&service, 0, sizeof(service));
//        service.addressfamily = af_bth;
//        service.serviceclassid = rfcomm_protocol_uuid;
//        service.port = bt_port_any;
//        str2ba(esp32macaddress1, &service.btaddr);
//         connects and checks for errors
//        if (connect(pc_socket, (sockaddr*)&service, sizeof(service)) == socket_error) {
//            printf("connect() failed: %ld.\n", wsagetlasterror());
//            int close_error = closesocket(pc_socket);
//            if (close_error == socket_error)
//                wprintf(l"closesocket function failed with error: %ld\n", wsagetlasterror());
//            sleep(200);
//            continue;
//            return 0;
//        }
//        else {
//            printf("connect() is ok!\n");
//        }
//
//
//
//         receive data from the esp32
//        int recv_result;
//        char buffer[6]; // we receive coordinate char and then float data and make room to add null terminator (1 + 4 + 1)
//
//        while (1) {
//
//            recv_result = recv(pc_socket, buffer, sizeof(buffer) - 1, 0);
//            if (recv_result > 0) {
//                buffer[recv_result] = '\0'; //null terminator added to end of buffer
//                printf("received data: %s\n", buffer);
//                float temp = std::stof(buffer);
//                printf("data as float: %f\n", temp);
//            }
//            else if (recv_result == 0) {
//                printf("connection closed\n");
//                break;
//            }
//            else {
//                printf("recv failed: %d\n", wsagetlasterror());
//                break;
//           
//            }
//        }
//        closesocket(pc_socket);
//    }
//    
//    
//    wsacleanup();
//	exitprocess(exit_success);
//}
//
//
