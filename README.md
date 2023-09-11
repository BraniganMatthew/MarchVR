# MarchVR
## Completed Work 
### Alpha Build
For our Alpha Build iteration, we have focused on implementing a GUI application for MarchVR as well as switching from Bluetooth Classic to Bluetooth Low Energy (BLE). The GUI application, MarchVR Hub, is a simplistic UI that will allow a user to manage the connection and calibration of the trackers as well as monitor relevant information about battery level and movement data. Switching to BLE will allow us to conserve power of our trackers to lengthen the possible play time of each session of use. BLE also allows us to reduce the latency between a player's physical movement and movement in-game.

### Prototype
- LSM6DS3TR accelerometer/gyrometer module sends data to Adafruit ESP32 Feather V2 microcontroller through I2C over Stemma QT cable
- Microcontroller receives and processes data recorded by the IMU
- Implemented infinite impulse response (IIR) filter to smooth out accelerometer data and eliminate bouncing
- Implemented calibration process to determine the orientation of the device on the user's leg
- Implemented speed calculation based on the frequency of steps taken, which is then sent to the OpenVR driver
- Established Bluetooth Classic communication between tracker and OpenVR driver
- Implemented Bluetooth auto-reconnect feature

### MarchVR Trackers
![image](https://github.com/BraniganMatthew/MarchVR/assets/90282856/9e1a0119-6e87-4d9e-b693-4ab951a2555c)

### MarchVR Hub GUI Application
![uimk1](https://github.com/BraniganMatthew/MarchVR/assets/90282856/3b3854a8-590f-4829-b2a3-c561a4490fde)

### Receiving Step Data
![MicrosoftTeams-image](https://github.com/BraniganMatthew/MarchVR/assets/90282856/bb272718-bd48-44b6-84e8-8b7ea1de91ad)

### Data received from the IMU:
![Screenshot (18)](https://github.com/BraniganMatthew/MarchVR/blob/main/Images/Screenshot%20(974).png)

## Project Architecture
### Alpha Build
As for the alpha build iteration, the main elements of MarchVR are two "trackers" running BLE, a python GUI, the OpenVR driver, and SteamVR. The "trackers" each consist of an Adafruit ESP32 Feather V2 and an LSM6DS3TR IMU to allow us to track and communicate player movement to a PC. These trackers use BLE in order to communicate with each other, the GUI, and the OpenVR driver. One tracker serves as a BLE client and the other serves as a BLE server. The BLE server tracker collects movement data from itself as well as the client tracker and communicates this to the OpenVR driver via BLE which then communicates it to SteamVR which will result in in-game movement. The server tracker also communicates with the Python GUI in order to establish connections, send movement data to the GUI, recalibrate if needed, and monitor battery life.

### Prototype
As for the prototype iteration, the main elements of our design are an Adafruit ESP32 Feather V2 with a 6-bit LSM6DS3TR accelerometer/gyroscope module connected via I2C through a Stemma QT cable. This makes up the tracker portion of our design, which records and processes the data received from the IMU to calculate the speed of movement needed by the OpenVR driver running on a PC. The calculated speed value is then sent from the microcontroller to the PC running the OpenVR driver via Bluetooth Classic/Winsock2 for final processing. Finally, we programmed additional functionality for speed control based on step frequency, as well as a reconnection feature for scenarios where the Bluetooth connection is temporarily lost between the tracker and PC.



## Known Bugs
- Latency issues, which we plan to fix by switching from Bluetooth Classic to Bluetooth Low Energy and making the step counter algorithm computationally faster
- The OpenVR driver currently only works on select games. Games tested include VRChat, SCP Labrat, and Blade & Sorcery, with the controller only working on VRChat.

## Prototype Demo:
https://youtu.be/ZxvRZggzVb4
