# MarchVR
## Milestone Summary
### Final Report
After switching to Bluetooth Low Energy in the Beta build, the next priority was to get everything working on a single device. A large roadblock that was experienced during the Beta build was that the MarchVR Hub and OpenVR driver was not able to be run on the same device due to attempting to utilize the same Bluetooth hardware. This was remedied in the release candidate by finally implementing a socket server application that would handle all Bluetooth communication with the trackers and simply distribute the received data to the driver and Hub. The usability of the trackers was greatly improved in the release candidate as well through the improvement of the status LEDs on the trackers, improved 3D printed casings, a robust auto-reconnect function for both the socket application and trackers, and the full implementation of a magnetometer on each tracker to determine orientation. By the final release of the project, features such as being able to walk in a direction independent of headset orientation had become robust and fully implemented. The implementation of backwards movement and the final iteration of the casing were completed in this milestone as well.

## Usage Instructions
### Installation
1. Download the zipped release file from the "Releases" tab of this repo.
2. Move the zip file to ```{Steam installation location}\steamapps\common\SteamVR\drivers```
3. Extract the zip file within that directory using a zip utility application.

### Gameplay Instructions
- Strap one tracker to each ankle and make sure that both trackers are now connected to each other (the status LEDs are not red) and facing forward. If any issues are noticed, please push the reset button on each tracker.
- Once in-game, launch the extracted socket server application, ```Socket-Server.exe```. Leave the socket server application and MarchVR Hub open during gameplay.
- Calibrate the trackers using the MarchVR Hub. Please make sure that both feet and the headset are facing forward during calibration. 
- March in place to move forward in game. Conversely, move the leg with the server tracker strapped onto it back, with the tip of the toe touching the ground, and hold that position to automatically move backwards.
- When done playing, both trackers will enter sleep mode after some time.

### Adding the MarchVR Driver
If a game does not recognize the MarchVR driver:

1. Go to the Settings page in SteamVR
2. Select the "Controllers" tab
3. Select the Manage Controller Bindings option
4. Choose which game to add the MarchVR driver to
5. Select custom then choose Another
6. On the top middle click on the controller and select ```marchvr_controller```
7. Click create new binding
8. Select add new trackpad and joystick for left controller
9. Save the binding

### Status LED Colors
- Red: Disconnected

Once connected, battery status is displayed:
- Orange: Low battery level
- Blue: Medium battery level
- Green: High battery level

### MarchVR Trackers
![March VR Trackers](https://github.com/BraniganMatthew/MarchVR/blob/main/Images/MarchVR_Hardware.png)

### MarchVR Hub GUI Application
![March VR Hub](https://github.com/BraniganMatthew/MarchVR/blob/main/Images/MarchVR_Hub2.png)

## Project Architecture
### Final Report
The main elements of MarchVR are two "trackers", a PyQT GUI known colloqially as the MarchVR Hub, the OpenVR driver, and a socket server application. The "trackers" each consist of an Adafruit ESP32 Feather V2, LSM6DS3TR IMU, LIS3MDL magnetometer, 3D printed casing, and a battery, allowing us to track and communicate player movement and orientation to a PC. These trackers utilize Bluetooth Low Energy (BLE) in order to communicate with each other and a user's PC, which runs the the socket server application. One tracker serves as a BLE client while the other serves as a BLE server, splitting up tasks between them and allowing for data to be sent to the driver without any desynchronization. The socket server application utilizes the SimpleBLE library to communicate with the trackers and then distributes movement information to the driver and PyQT GUI application through a local socket. The OpenVR driver then uses this movement data to translate the user's physical movement to virtual movement by simulating joystick movement. When the Recalibrate button on the MarchVR Hub is clicked, a calibration command is sent to the socket server through the socket, then to the server tracker through BLE, and then shared with the client tracker in order to recalibrate both trackers before and during gameplay.

![Project Architecture](https://github.com/BraniganMatthew/MarchVR/blob/main/Images/MarchVR_Schematic2.png)

## Work Completed in this Milestone
### Walking Direction Orientation
One limitation that was noticed early on in development is that the  user would be forced to walk in the direction they are facing. To resolve this, the direction that the user’s feet are facing is compared with the direction that the headset is facing in order to determine the walking direction in virtual reality. This required the integration of a magnetometer on the trackers as discussed above. This also required modifications to the OpenVR driver to handle these calculations upon receiving the orientation and step data from the trackers. Upon properly calculating the direction from the data, the driver sends the movement control to SteamVR, resulting in a step in the virtual reality space in the direction of the user's feet.
### Backwards Movement
Backwards movement was finally implemented prior to the final release. By angling the user's leg backwards, a constant backwards joystick input will move the user backwards in the virtual space. This prevents the user from having to turn their entire body backwards in order to take a step back, for example.
### Final Iteration of 3D Casing
In order to make room for the addition of the magnetometer and stabilize all equipment within the 3D printed case, the casing was further improved and expanded during the release candidate. To achieve this, a "skeleton" for the electronic components was developed with shelves to contain different components. This skeleton is enclosed within the box-like structure of the case and a Velcro strap is looped underneath it to allow the tracker to be strapped to a user's leg. For the final release, the button for resetting the device is now integrated into the case, eliminating the need for an external button that may fall out of the tracker during gameplay.

## Known Bugs
- SteamVR watchdog prevents automatic opening of socket application from OpenVR driver. Thus the user must start up the socket server application manually.

## Release Candidate:
[Demo Video](https://youtu.be/IjgnNKbYWoc)

[Timelog](https://docs.google.com/spreadsheets/d/1unugdZlc-4rDBkXROHAn1-_Av18swAkoabrYpZTAJ7A/edit?usp=sharing)

## Contributors
Matthew Branigan
Jibin Alex
Patrick Harris
Richard Cusolito
Maria Carmona
