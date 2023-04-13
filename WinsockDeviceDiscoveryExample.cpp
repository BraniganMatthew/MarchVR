// Note: These header files used are from the
// Windows SDKs v6.0A. The Windows CE header files
// are different though some of the file names might be similar
// and this is 'tally' with the VS project type. In full version VS
// there is a project template for Windows CE which is SmartDevice.
// Choosing the SmartDevice project template will 'bind' the project
// to the appropriate header files and library

// Link to ws2_32.lib
#include <winsock2.h>
#include <ws2bth.h>
#include <BluetoothAPIs.h>
#include <stdio.h>

#define MAX_NAME 248
typedef ULONGLONG bt_addr, *pbt_addr, BT_ADDR, *PBT_ADDR;

union {
    // Storage for returned struct
    CHAR buf[5000];
    // properly align buffer to BT_ADDR requirements
    SOCKADDR_BTH _Unused_;
} butuh;



//------------------------------------------------------------------------
// Function: FindingBtDevices
// Purpose: Performs a device inquiry displays the device name in a console
//------------------------------------------------------------------------

BOOL FindingBtDevices() {
    WSAQUERYSET wsaq;
    HANDLE hLookup;
    LPWSAQUERYSET pwsaResults;
    DWORD dwSize;
    BOOL bHaveName;
    BT_ADDR btAddr;
    DWORD dwNameSpace;

    pwsaResults = (LPWSAQUERYSET) butuh.buf;
    dwSize  = sizeof(butuh.buf);
    ZeroMemory(&wsaq, sizeof(wsaq));
    wsaq.dwSize = sizeof(wsaq);

    // namespace MUST be NS_BTH for bluetooth queries
    wsaq.dwNameSpace = NS_BTH;
    wsaq.lpcsaBuffer = NULL;
    printf("\n");

    // initialize searching procedure
    if (WSALookupServiceBegin(&wsaq, LUP_CONTAINERS, &hLookup) == SOCKET_ERROR) {
        printf("WSALookupServiceBegin() failed %d\r\n", WSAGetLastError());
        return FALSE;
    }
    else
        printf("WSALookupServiceBegin() pretty damn OK!\n");

    ZeroMemory(pwsaResults, sizeof(WSAQUERYSET));
    pwsaResults->dwSize = sizeof(WSAQUERYSET);
    pwsaResults->dwNameSpace = NS_BTH;
    pwsaResults->lpBlob = NULL;

    // iterate through all found devices, returning name and address
    while (WSALookupServiceNext(hLookup, LUP_RETURN_NAME | LUP_RETURN_ADDR, &dwSize, pwsaResults) == 0) {
        btAddr = ((SOCKADDR_BTH *)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr;

        // Verify the returned name
        bHaveName = (pwsaResults->lpszServiceInstanceName) && *(pwsaResults->lpszServiceInstanceName);
        dwNameSpace = pwsaResults->dwNameSpace;

        // print name and address
        wprintf (L"Name\tNAP Address\tSAP Address\tName Space\n");
        wprintf (L"%s\t0X%04X\t\t0X%08X\t0X%0X\n", pwsaResults->lpszServiceInstanceName, GET_NAP(btAddr), GET_SAP(btAddr),
                            dwNameSpace);
    }

    if(WSALookupServiceEnd(hLookup) == 0)
        printf("WSALookupServiceEnd(hLookup) is OK!\n");
    else
        printf("WSALookupServiceEnd(hLookup) failed with error code %ld\n",WSAGetLastError());
    return TRUE;
}

int main(int argc, char **argv) {
    WSADATA wsd;
    BOOL retVal;

    if(WSAStartup(MAKEWORD(2,2), &wsd) != 0)
        printf("WSAStartup() failed with error code %ld\n", WSAGetLastError());
    else
        printf("WSAStartup() is OK!\n");
    retVal = FindingBtDevices();

    if(FindingBtDevices() == FALSE)
        printf("\nNo bulutooth device in range found!\n");
    else
        printf("\nWell, found some bluetooth devices around!\n");

    if(WSACleanup() == 0)
        printf("WSACleanup() is OK!\n");
    else
        printf("WSACleanup() failed with error code %ld\n", WSAGetLastError());
    return 0;
}

// https://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4j.html