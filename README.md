# MarchVR
## Completed Work (Prototype)
- LSM6DS3TR accelerometer/gyrometer module sends data to Adafruit Feather V2 microcontroller through I2C
- IMU data received by the microcontroller and processed on arduino IDE
- Calibration process implemented to determine the orientation of the device on the user's leg
- Implemented Infinite Impulse Response (IIR) filter to smooth out accelerometer data and prevent bouncing
- Calculate speed based on the frequency of steps taken
- Established Bluetooth communication between OpenVR and tracker to send speed data to determine how fast the user's movement will be
![Screenshot (18)](https://github.com/BraniganMatthew/MarchVR/blob/main/Screenshot%20(974).png)

## Project Architecture
As of today, we have an Adafruit ESP32 Feather with 6-bit IMU connected via I2C. This gives us access to all the data and processing power we need to properly calculate steps/frequency all on the tracker itself. Finally we send the speed data to the PC via bluetooth/winsock2 for final processing by the OpenVR driver. We have recently programmed in additional functionality for speed control based on how fast you step as well as a reconnection feature for cases where connection is temporarily lost between the tracker and PC.

## Known Bugs
- Lagging issues on OpenVR driver, which we plan to fix by switching from Classical Bluetooth to BLE for communication and improving the step counter algorithm to process data faster
- The OpenVR example "controller" currently only works on select games. Games tested include VRChat and Blade & Sorcery, with the controller only working on VRChat.
(https://youtu.be/ZxvRZggzVb4 "Prototype Demo")
