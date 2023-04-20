# MarchVR
## Completed Work (Prototype)
- LSM6DS3TR accelerometer/gyrometer module sends data to Adafruit ESP32 Feather V2 microcontroller through I2C over Stemma QT cable
- Microcontroller receives and processes data recorded by the IMU
- Implemented infinite impulse response (IIR) filter to smooth out accelerometer data and eliminate bouncing
- Implemented calibration process to determine the orientation of the device on the user's leg
- Implemented speed calculation based on the frequency of steps taken, which is then sent to the OpenVR driver
- Established Bluetooth Classic communication between tracker and OpenVR driver
- Implemented Bluetooth auto-reconnect feature

## Data received from the IMU:
![Screenshot (18)](https://github.com/BraniganMatthew/MarchVR/blob/main/Images/Screenshot%20(974).png)

## Project Architecture
As of now, the main elements of our design are an Adafruit ESP32 Feather V2 with a 6-bit LSM6DS3TR accelerometer/gyroscope module connected via I2C through a Stemma QT cable. This makes up the tracker portion of our design, which records and processes the data received from the IMU to calculate the speed of movement needed by the OpenVR driver running on a PC. The calculated speed value is then sent from the microcontroller to the PC running the OpenVR driver via Bluetooth Classic/Winsock2 for final processing. Finally, we programmed additional functionality for speed control based on step frequency, as well as a reconnection feature for scenarios where the Bluetooth connection is temporarily lost between the tracker and PC.

## Known Bugs
- Latency issues, which we plan to fix by switching from Bluetooth Classic to Bluetooth Low Energy and making the step counter algorithm computationally faster
- The OpenVR driver currently only works on select games. Games tested include VRChat, SCP Labrat, and Blade & Sorcery, with the controller only working on VRChat.

## Prototype Demo:
https://youtu.be/ZxvRZggzVb4
