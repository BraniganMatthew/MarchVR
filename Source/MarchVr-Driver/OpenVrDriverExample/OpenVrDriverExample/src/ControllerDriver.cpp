#include <ControllerDriver.h>
#include <iostream>
#include <vector>
#include <thread>
#include <sstream>
#include <string>
#include <Windows.h>
//#include <winsock2.h>

#include "utils.hpp"
#include "simpleble/SimpleBLE.h"


using namespace std::chrono_literals;

#define serviceUUID     "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define charUUID        "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define charUUID_TRK "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define charUUID_GUI "aad41096-f795-4b3b-83bb-858051e5e284"
#define charUUID_DRV "22d7a034-791d-49f6-a84e-ef78ab2473ad"
#define MAX_TIME 125

void startup_socket_server(const char* command) {
	//ShellExecute(NULL, L"open", L"Socket-Test.exe", NULL, NULL, SW_RESTORE);
	//int returnCode = 0;
	//int returnCode = std::system("start /B .\\Socket-Test.exe");
	int returnCode = std::system(command);
	if (returnCode == 0) {
		std::cout << "Socket Server started successfully." << std::endl;
	}
	else {
		std::cout << "Error starting Socket Server application." << std::endl;
	}
}
double quaternion_to_yaw(double qw, double qx, double qy, double qz) {
	double yaw = std::atan2(2 * ((qy * qz) + (qw * qx)), qw * qw - qx * qx - qy * qy + qz * qz);
	return yaw;
}
SimpleBLE::Peripheral search_for_esp32_and_connect(std::vector<SimpleBLE::Peripheral>& peripherals);
void process_esp32_data(SimpleBLE::ByteArray& bytes);
template < class T >
vr::HmdQuaternion_t HmdQuaternion_FromMatrix(const T& matrix);

SimpleBLE::Peripheral TK1_peripheral;
SOCKET client_socket;
struct timeval timeout;
fd_set rfd;
float speed = 0.0;
float direction = 0.0;
float diff_a, diff_b = 0.0;
float yaw = 0.0;
float yaw_offset = 0.0;
float headset_offset = 0.0;
int reset_time = MAX_TIME;
bool hasStartedSocket = false;
bool isConnectedToSocket = false;
bool ConnectToHost() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		VRDriverLog()->Log("WSAStartup failed.");
		return false;
	}

	SOCKADDR_IN server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8081); // Use the port you want
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == INVALID_SOCKET) {
		VRDriverLog()->Log("Invalid client_socket");
		WSACleanup();
		return false;
	}

	if (connect(client_socket, (SOCKADDR*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
		VRDriverLog()->Log("Connection failed.");
		closesocket(client_socket);
		WSACleanup();
		return false;
	}

	timeout.tv_sec = 0;
	timeout.tv_usec = 100;

	VRDriverLog()->Log("Connected to server.");
	return true;
}

bool CloseConnection() {
	if (client_socket) {
		closesocket(client_socket);
		return false;
	}
	WSACleanup();
	return true;
}


EVRInitError ControllerDriver::Activate(uint32_t unObjectId)
{


	driverId = unObjectId; //unique ID for your driver

	PropertyContainerHandle_t props = VRProperties()->TrackedDeviceToPropertyContainer(driverId); //this gets a container object where you store all the information about your driver

	VRProperties()->SetStringProperty(props, Prop_InputProfilePath_String, "{marchvr}/input/controller_profile.json"); //tell OpenVR where to get your driver's Input Profile
	VRProperties()->SetInt32Property(props, Prop_ControllerRoleHint_Int32, ETrackedControllerRole::TrackedControllerRole_Treadmill); //tells OpenVR what kind of device this is
	VRDriverInput()->CreateScalarComponent(props, "/input/joystick/y", &joystickYHandle, EVRScalarType::VRScalarType_Absolute,
		EVRScalarUnits::VRScalarUnits_NormalizedTwoSided); //sets up handler you'll use to send joystick commands to OpenVR with, in the Y direction (forward/backward)
	VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/y", &trackpadYHandle, EVRScalarType::VRScalarType_Absolute,
		EVRScalarUnits::VRScalarUnits_NormalizedTwoSided); //sets up handler you'll use to send trackpad commands to OpenVR with, in the Y direction
	VRDriverInput()->CreateScalarComponent(props, "/input/joystick/x", &joystickXHandle, EVRScalarType::VRScalarType_Absolute,
		EVRScalarUnits::VRScalarUnits_NormalizedTwoSided); //Why VRScalarType_Absolute? Take a look at the comments on EVRScalarType.
	VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/x", &trackpadXHandle, EVRScalarType::VRScalarType_Absolute,
		EVRScalarUnits::VRScalarUnits_NormalizedTwoSided); //Why VRScalarUnits_NormalizedTwoSided? Take a look at the comments on EVRScalarUnits.

	if (!isConnectedToSocket) {
		VRDriverLog()->Log("Error in ConnectToHost() function");
		isConnectedToSocket = ConnectToHost();
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

	//if (hasStartedSocket == false) {
	//	hasStartedSocket = true;
	//	//==============START SOCKET SERVER==================
	//	const char* command = "start /B drivers/marchvr/bin/win64/Socket-Test.exe";
	//	//const char* command = "cd && pause";
	//	std::thread t2(startup_socket_server, command);
	//	//std::this_thread::sleep_for(20s);
	//}

	// update head pose
	vr::TrackedDevicePose_t hmd_pose{};
	vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0.f, &hmd_pose, 1);
	const vr::HmdQuaternion_t hmd_orientation = HmdQuaternion_FromMatrix(hmd_pose.mDeviceToAbsoluteTracking);

	float active_speed = 0.0;
	// check if there is data to receive
	FD_ZERO(&rfd);
	FD_SET(client_socket, &rfd);
	int ret = select(client_socket + 1, &rfd, NULL, NULL, &timeout);
	char buffer[64];
	if (ret > 0) {

		//get data into buffer
		memset(buffer, 0, sizeof(buffer)); //clear buffer
		int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
		VRDriverLog()->Log(buffer);

		std::istringstream iss(buffer);
		std::vector<std::string> tokens;
		std::string token;

		// parse data from trackers
		while (std::getline(iss, token, ';')) {
			tokens.push_back(token);
		}
		float t_speed = 0;
		if (tokens.size() >= 8 && tokens[1] == "TK1" && tokens[3] == "MOT") {
			t_speed = std::stof(tokens[8]);
			if (t_speed == 0.0) {
				VRDriverLog()->Log("here");
				yaw = std::stof(tokens[5]);
				yaw_offset = yaw;
				headset_offset = (2 * acos(hmd_orientation.y) / 3.1415926f);
				//headset_offset += (headset_offset < 0) ? 2 : 0;
			}
			else {
				if (t_speed > 1.0) t_speed = 1.0;
				yaw = std::stof(tokens[5]);
				speed = t_speed;
				reset_time = MAX_TIME;
				//if (std::stof(tokens[7]) < 0) {
					//speed *= -1;
				//}
				float hmd_yaw = (2 * acos(hmd_orientation.y) / 3.1415926f);
				float diff_h = (hmd_yaw - headset_offset);
				float diff_t = (yaw - yaw_offset);
				diff_h += (diff_h < 0) ? 2 : 0;
				diff_t += (diff_t < 0) ? 2 : 0;
				diff_t = 2 - diff_t;
				diff_a = diff_h - diff_t;
				diff_b = diff_t - diff_h;
				direction = (diff_b < 0) ? diff_b + 2.0 : diff_b;
			}
		}
		else if (tokens.size() >= 3 && tokens[3] == "CAL") {
			VRDriverLog()->Log("Yaw_offset: ");
			VRDriverLog()->Log(std::to_string(yaw_offset).c_str());
			return;
		}


	}

	if (reset_time <= 20) {
		active_speed = 0.0;
	}
	else {
		active_speed = speed * (log(reset_time) / log(MAX_TIME));
		reset_time--;
	}
	if (active_speed > 1.0) active_speed = 1;

	if (ret > 0) {
		VRDriverLog()->Log("\n\nBatch\n");
		VRDriverLog()->Log("yaw_offset: ");
		VRDriverLog()->Log(std::to_string(yaw_offset).c_str());
		VRDriverLog()->Log("headset_offset: ");
		VRDriverLog()->Log(std::to_string(headset_offset).c_str());
		VRDriverLog()->Log("direction: ");
		VRDriverLog()->Log(std::to_string(direction).c_str());

	}



	//Since we used VRScalarUnits_NormalizedTwoSided as the unit, the range is -1 to 1.

	VRDriverInput()->UpdateScalarComponent(joystickYHandle, active_speed * cos(direction * 3.1415926f), 0); //move forward
	VRDriverInput()->UpdateScalarComponent(trackpadYHandle, active_speed * cos(direction * 3.1415926f), 0); //move foward
	VRDriverInput()->UpdateScalarComponent(joystickXHandle, active_speed * sin(direction * 3.1415926f), 0); //change the value to move sideways
	VRDriverInput()->UpdateScalarComponent(trackpadXHandle, active_speed * sin(direction * 3.1415926f), 0); //change the value to move sideways
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

SimpleBLE::Peripheral search_for_esp32_and_connect(std::vector<SimpleBLE::Peripheral>& peripherals) {
	VRDriverLog()->Log("Entered search_for_esp32_and_connect");
	for (auto& peripheral : peripherals) {
		for (auto& service : peripheral.services()) {
			if (service.uuid() == serviceUUID) {
				peripheral.connect();
				VRDriverLog()->Log("Connecting to peripheral via serviceUUID");
				//std::cout << "Connecting to " << peripheral.identifier() << " [" << peripheral.address() << "]" << " via serviceUUID: " << serviceUUID << std::endl;
				return peripheral;
			}
		}
	}
}

void process_esp32_data(SimpleBLE::ByteArray& bytes) {
	std::stringstream ss;
	for (auto b : bytes) {
		ss << (char)b;
	}
}

template < class T >
vr::HmdQuaternion_t HmdQuaternion_FromMatrix(const T& matrix)
{
	vr::HmdQuaternion_t q{};

	q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;

	q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
	q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
	q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);

	return q;
}


// add logs to check when each function gets called
// once angles are from -1 to 1 set them to be in radians.