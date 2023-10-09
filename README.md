# MarchVR
## Completed Work 
### Beta Build
After switching from Bluetooth Classic to Bluetooth Low Energy (BLE) in the Alpha build, the Beta build has largely focused on expanding the functionality of the project, moving on from the "vertical slice" of the Alpha. The GUI application, MarchVR Hub, has a simplistic UI that allows a user to easily manage the connection and calibration of the trackers. The OpenVR driver now features less latency than ever before after switching to BLE. Through the alpha test plan, however, it was discovered that the MarchVR Hub and OpenVR driver cannot be connected at the same time on the same device, due to both functioning as BLE clients and utilizing the same Bluetooth hardware. Thus, it has been determined that the two must be integrated in the form of a socket server such that only one BLE client is active on a user's PC at any given time. A sleep mode feature for the trackers has also been implemented to extend the battery life of the trackers. After initial testing following the alpha test plan, it was determined that the trackers have a battery life of about 10 hours, which is more than enough for a VR gameplay session.

### MarchVR Trackers
![March VR Trackers](https://github.com/BraniganMatthew/MarchVR/blob/main/Images/MarchVR_Trackers.png)

### MarchVR Hub GUI Application
![March VR Hub](https://github.com/BraniganMatthew/MarchVR/blob/main/Images/MarchVR_Hub.png)

## Project Architecture
### Beta Build
As in the Alpha build iteration, the main elements of the Beta build for MarchVR are two "trackers", a PyQT GUI, and the OpenVR driver. The "trackers" each consist of an Adafruit ESP32 Feather V2 and an LSM6DS3TR IMU, allowing us to track and communicate player movement to a PC. These trackers utilize BLE in order to communicate with each other and a user's PC, which runs the OpenVR driver and PyQT GUI. One tracker serves as a BLE client while the other serves as a BLE server, splitting up tasks between them and allowing for data to be sent to the driver without any desynchronization. The server tracker communicates both the movement data for itself as well as the client tracker to the OpenVR driver, which then uses APIs to result in in-game movement. The server tracker also communicates with the PyQT GUI in order to receive a recalibration command if needed and monitor the status of the trackers.

![Project Architecture](https://github.com/BraniganMatthew/MarchVR/blob/main/Images/MarchVR_Schematic2.png)

## Work Completed in this Milestone
- Functionality for the GUI has been completely implemented
- Sleep mode functionality for the trackers
- Transmission data format has been finalized and implemented
- Power and connection indicator light has been implemented
- 3D printed casing has been manufactured and implemented over the course of three design iterations
- Discovered a solution to implement the project on all VR games (a longstanding issue that was experienced was that MarchVR would only work with VRChat)
- BLE notification system has been implemented for the OpenVR driver, significantly reducing latency by removing polling
- Error checking for the MarchVR Hub has been implemented
- Alpha test plan has discovered that the battery life for each tracker is about 10 hours
- Alpha test plan has determined that running two BLE clients on one device is unfeasible
- Alpha test plan has determined that a magnetometer is required for determining the user’s orientation due to gyroscope reading instability
- Alpha test plan has discovered that the latency has been reduced compared to using Bluetooth Classic, which was the goal of the swap to Bluetooth Low Energy in the Alpha build


## Known Bugs
- The PyQT GUI and OpenVR driver cannot be run concurrently on the same device due to sharing the same Bluetooth hardware. Fixes include using the UI to calibrate the trackers prior to launching a game or using one device for the GUI and one device for running VR games on, neither of which are ideal. Thus, it has been determined that the best course of action would be the creation of a socket server to integrate both BLE clients into one and handle all incoming/outgoing BLE traffic to the user's PC for MarchVR.
- The yaw data received from the IMU has been determined to be unstable due to the lack of a magnetometer. This is a necessary component due to needing the user's orientation for the expansion of walking capabilities, so an additional magnetometer module will be acquired as soon as possible.
- The Bleak Python library that is used for BLE communication occasionally causes errors, especially when attempting to calibrate while the tracker is currently transmitting that a step has occurred. A "GATT Services Unreachable" error occasionally occurs when attempting to connect on certain Windows 10 machines. After resolving the bug listed below, this issue may be resolved but further testing is required.
- ~~The ESP32's effective range for BLE communication is only about one meter. Beyond this distance, communication may be unstable. This is suitable for communication between the server and client trackers, but not ideal for communication with the user's PC.~~ This issue was resolved by swapping the tracker used for the BLE server, identifying the most likely culprit as faulty hardware. 

## Beta Build Demo:
[Demo Video](https://youtu.be/ynCtCZZmBgc)
