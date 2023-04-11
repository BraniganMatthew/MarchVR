#include <string>
#include <winsock2.h>
#include <ws2bth.h>

int str2ba(const char *straddr, BTH_ADDR *btaddr) {
    int i;
    unsigned int aaddr[6];
    BTH_ADDR tmpaddr = 0;

    if (sscanf(straddr, "%02x:%02x:%02x:%02x:%02x:%02x",
                    &aaddr[0], &aaddr[1], &aaddr[2],
                    &aaddr[3], &aaddr[4], &aaddr[5]) != 6)
        return 1;
    *btaddr = 0;
    for (i = 0; i < 6; i++) {
        tmpaddr = (BTH_ADDR) (aaddr[i] & 0xff);
        *btaddr = ((*btaddr) << 8) + tmpaddr;
    }
    return 0;
}

int main() {
    SOCKADDR_BTH sockAddr;
    SOCKET btSocket;
    int error;
    const char* ESP32MACaddress = "E8:9F:6D:26:9F:1A";

    btSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    memset (&sockAddr, 0, sizeof(sockAddr));
    sockAddr.addressFamily = AF_BTH;
    sockAddr.serviceClassId = FaxServiceClass_UUID;
    sockAddr.port = BT_PORT_ANY;
    str2ba(ESP32MACaddress, &sockAddr.btAddr);
    error = connect(btSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr));
    return 0;
}

// https://stackoverflow.com/questions/20080327/bluetooth-with-c-and-winsock

// https://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4o.html

// https://www.tenouk.com/Winsock/Winsock2example2.html