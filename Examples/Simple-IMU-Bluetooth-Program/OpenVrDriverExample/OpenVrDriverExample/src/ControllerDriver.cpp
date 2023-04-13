//dependencies for bt portion
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#pragma execution_character_set("utf-8")
#include <WinSock2.h>
#include <ws2bth.h>
#include <stdio.h>
#include <bluetoothapis.h>
#include <string>

#include <ControllerDriver.h>
#include <stdlib.h>

// ESP32 Address
const char* ESP32MACaddress1 = "E8:9F:6D:26:9F:1A";
const char* ESP32MACaddress2 = "E8:9F:6D:2F:27:D6";

SOCKET pc_socket;

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

EVRInitError ControllerDriver::Activate(uint32_t unObjectId)
{
	driverId = unObjectId; //unique ID for your driver

	PropertyContainerHandle_t props = VRProperties()->TrackedDeviceToPropertyContainer(driverId); //this gets a container object where you store all the information about your driver

	VRProperties()->SetStringProperty(props, Prop_InputProfilePath_String, "{example}/input/controller_profile.json"); //tell OpenVR where to get your driver's Input Profile
	VRProperties()->SetInt32Property(props, Prop_ControllerRoleHint_Int32, ETrackedControllerRole::TrackedControllerRole_Treadmill); //tells OpenVR what kind of device this is
	VRDriverInput()->CreateScalarComponent(props, "/input/joystick/y", &joystickYHandle, EVRScalarType::VRScalarType_Absolute,
		EVRScalarUnits::VRScalarUnits_NormalizedTwoSided); //sets up handler you'll use to send joystick commands to OpenVR with, in the Y direction (forward/backward)
	VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/y", &trackpadYHandle, EVRScalarType::VRScalarType_Absolute,
		EVRScalarUnits::VRScalarUnits_NormalizedTwoSided); //sets up handler you'll use to send trackpad commands to OpenVR with, in the Y direction
	VRDriverInput()->CreateScalarComponent(props, "/input/joystick/x", &joystickXHandle, EVRScalarType::VRScalarType_Absolute,
		EVRScalarUnits::VRScalarUnits_NormalizedTwoSided); //Why VRScalarType_Absolute? Take a look at the comments on EVRScalarType.
	VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/x", &trackpadXHandle, EVRScalarType::VRScalarType_Absolute,
		EVRScalarUnits::VRScalarUnits_NormalizedTwoSided); //Why VRScalarUnits_NormalizedTwoSided? Take a look at the comments on EVRScalarUnits.
	
	// Initialize winsock
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	int wsaError = WSAStartup(wVersionRequested, &wsaData);
	// Error checking
	if (wsaError != 0) {
		printf("The Winsock dll not found!\n");
		//return;
	}
	else {
		printf("The Winsock dll found!\n");
		printf("The status: %s.\n", wsaData.szSystemStatus);
	}

	// Error checking to see if it supports 2.2
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		printf("The dll do not support the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
		WSACleanup();
		//return;
	}
	else {
		printf("The dll supports the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
		printf("The highest version this dll can support: %u.%u\n", LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
	}

	// Create socket on pc
	pc_socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	// Error checking
	if (pc_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		//return;
	}
	else {
		printf("socket() is OK!\n");
	}


	// Connect the socket to ESP32
	// See https://learn.microsoft.com/en-us/windows/win32/api/ws2bth/ns-ws2bth-sockaddr_bth for more info
	// Creates a proper socket address for bt service
	SOCKADDR_BTH service = { 0 };
	service.addressFamily = AF_BTH;
	service.serviceClassId = RFCOMM_PROTOCOL_UUID;
	service.port = BT_PORT_ANY;
	str2ba(ESP32MACaddress2, &service.btAddr);
	// Connects and checks for errors
	if (connect(pc_socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
		printf("connect() failed: %ld.\n", WSAGetLastError());
		int close_error = closesocket(pc_socket);
		if (close_error == SOCKET_ERROR)
			wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		//return;
	}
	else {
		printf("connect() is OK!\n");
	}


	return VRInitError_None;
}

DriverPose_t ControllerDriver::GetPose()
{
	DriverPose_t pose = { 0 }; //This example doesn't use Pose, so this method is just returning a default Pose.
	pose.poseIsValid = false;
	pose.result = TrackingResult_Calibrating_OutOfRange;
	pose.deviceIsConnected = true;

	HmdQuaternion_t quat;
	quat.w = 1;
	quat.x = 0;
	quat.y = 0;
	quat.z = 0;

	pose.qWorldFromDriverRotation = quat;
	pose.qDriverFromHeadRotation = quat;

	return pose;
}

void ControllerDriver::RunFrame()
{
	// Receive data from the ESP32
	int recv_result;
	char buffer[6]; // we receive coordinate char and then float data and make room to add null terminator (1 + 4 + 1)
	while (1) {

		recv_result = recv(pc_socket, buffer, sizeof(buffer) - 1, 0);
		if (recv_result > 0) {
			buffer[recv_result] = '\0'; //null terminator added to end of buffer
			printf("Received data: %s\n", buffer);
		}
		else if (recv_result == 0) {
			printf("Connection closed\n");
		}
		else {
			printf("recv failed: %d\n", WSAGetLastError());
		}
	}

	float temp = std::stof(buffer);
	temp = temp / 10.3;
	//Since we used VRScalarUnits_NormalizedTwoSided as the unit, the range is -1 to 1.
	VRDriverInput()->UpdateScalarComponent(joystickYHandle, temp, 0); //move forward
	VRDriverInput()->UpdateScalarComponent(trackpadYHandle, temp, 0); //move foward
	VRDriverInput()->UpdateScalarComponent(joystickXHandle, 0, 0); //change the value to move sideways
	VRDriverInput()->UpdateScalarComponent(trackpadXHandle, 0, 0); //change the value to move sideways
}

void ControllerDriver::Deactivate()
{
	driverId = k_unTrackedDeviceIndexInvalid;
}

void* ControllerDriver::GetComponent(const char* pchComponentNameAndVersion)
{
	//I found that if this method just returns null always, it works fine. But I'm leaving the if statement in since it doesn't hurt.
	//Check out the IVRDriverInput_Version declaration in openvr_driver.h. You can search that file for other _Version declarations 
	//to see other components that are available. You could also put a log in this class and output the value passed into this 
	//method to see what OpenVR is looking for.
	if (strcmp(IVRDriverInput_Version, pchComponentNameAndVersion) == 0)
	{
		return this;
	}
	return NULL;
}

void ControllerDriver::EnterStandby() {}

void ControllerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) 
{
	if (unResponseBufferSize >= 1)
	{
		pchResponseBuffer[0] = 0;
	}
}