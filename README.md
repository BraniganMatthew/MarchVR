# MarchVR
## Milestone Summary
### Release Candidate
After switching to Bluetooth Low Energy in the Beta build, the next priority was to get everything working on a single device. A large roadblock that was experienced during the Beta build was that the MarchVR Hub and OpenVR driver was not able to be run on the same device due to attempting to utilize the same Bluetooth hardware. This was remedied in this milestone by finally implementing a socket server application that would handle all Bluetooth communication with the trackers and simply distribute the received data to the driver and Hub. The usability of the trackers was greatly improved in this milestone as well through the improvement of the status LEDs on the trackers, improved 3D printed casings, a robust auto-reconnect function for both the socket application and trackers, and the full implementation of a magnetometer on each tracker to determine orientation.

### MarchVR Trackers
![March VR Trackers](https://github.com/BraniganMatthew/MarchVR/blob/main/Images/MarchVR_Trackers2.png)

### MarchVR Hub GUI Application
![March VR Hub](https://github.com/BraniganMatthew/MarchVR/blob/main/Images/MarchVR_Hub2.png)

## Project Architecture
### Release Candidate
The main elements of the Release Candidate for MarchVR are two "trackers", a PyQT GUI known colloqially as the MarchVR Hub, the OpenVR driver, and the new addition of a socket server application. The "trackers" each consist of an Adafruit ESP32 Feather V2, LSM6DS3TR IMU, LIS3MDL magnetometer, 3D printed casing, and a battery, allowing us to track and communicate player movement and orientation to a PC. These trackers utilize BLE in order to communicate with each other and a user's PC, which runs the the socket server application. One tracker serves as a BLE client while the other serves as a BLE server, splitting up tasks between them and allowing for data to be sent to the driver without any desynchronization. The socket server application utilizes the SimpleBLE library to communicate with the trackers and then distributes movement information to the driver and PyQT GUI application through a local socket. The OpenVR driver then uses this movement data to translate the user's physical movement to virtual movement by simulating joystick movement. When the Recalibrate button on the MarchVR Hub is clicked, a calibration command is sent to the socket server through the socket, then to the server tracker through BLE, and then shared with the client tracker in order to recalibrate both trackers before and during gameplay.

![Project Architecture](https://github.com/BraniganMatthew/MarchVR/blob/main/Images/MarchVR_Schematic2.png)

## Work Completed in this Milestone
### Socket Server Application
In order to remedy the issue experienced in the Beta build where the OpenVR driver and MarchVR Hub had to be run on separate devices due to attemping to share the same Bluetooth hardware, a socket server application was developed. This application was written in C++ and used the SimpleBLE library to connect to the server tracker. An auto-reconnect function was implemented in case the connection to the tracker was lost, as well. This socket server was then developed to distribute movement information to the OpenVR driver and MarchVR Hub, as well as relay the calibration command sent from the Hub to the trackers, circumventing the need for the Hub to connect to the tracker directly.
### Implementation of the Magnetometers
In order to accurately derive the orientation of the user, a magnetometer was added to each tracker. By measuring the Earth’s magnetic field through the magnetometer, the cardinal direction of the user’s feet can be determined. This will allow the user's walking direction to be independent of the headset direction when moving in virtual reality.
### Walking Direction Orientation
One limitation that was noticed early on in development is that the  user would be forced to walk in the direction they are facing. To resolve this, the direction that the user’s feet are facing is compared with the direction that the headset is facing in order to determine the walking direction in virtual reality. This required the integration of a magnetometer on the trackers as discussed above. This also required modifications to the OpenVR driver to handle these calculations upon receiving the orientation and step data from the trackers. Upon properly calculating the direction from the data, the driver sends the movement control to SteamVR, resulting in a step in the virtual reality space in the direction of the user's feet.
### Tracker Improvements
An auto-reconnect function was developed for the server and client trackers, requiring zero user input for ease of use. After moving them sufficiently far apart, a red light can be noticed on the LEDs of both trackers. Once brought together, the indicator LED turns yellow/blue/green and shows that they are connected to each other. If at this point they still do not connect, the newly implemented 3D printed reset button on each tracker can be used. The yellow/blue/green status LEDs indicate low/medium/high battery level and are a newly implemented feature in this milestone. The frequency calculation used for step detection was modified during this milestone in order to give the user a smoother and more responsive experience during this milestone as well, specifically making improvements to movement at low speed.
### New Iteration of 3D Casing
In order to make room for the addition of the magnetometer and stabilize all equipment within the 3D printed case, the casing was further improved and expanded. To achieve this, a "skeleton" for the electronic components was developed with shelves to contain different components. This skeleton is enclosed within the box-like structure of the case and a Velcro strap is looped underneath it to allow the tracker to be strapped to a user's leg.

## Known Bugs
- User must start up socket server manually when using SteamVR (issues have arisen when attemping to launch automatically from driver)
- Closing the socket server during gameplay can cause the driver to crash
- The improved casing is currently not implemented due to numerous 3D printing errors

## Release Candidate:
[Demo Video]()
