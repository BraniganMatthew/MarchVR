# MarchVR
## Completed Work
- LSM6DS3TR accelerometer/gyrometer module sends data to Adafruit Feather V2 microcontroller through I2C
- Bluetooth Classic connection established between microcontroller and PC via a Python script
- Data is transmitted through the aforementioned Bluetooth connection and filtered by the Python script
- Accelerometer and gyroscope data is then received and plotted using Python and the Matplotlib library

## Project Architecture
The LSM6DS3TR-C module sends the recorded XYZ accelerometer and gyrometer data to the ESP32 Feather V2 microcontroller using the I2C serial communication protocol and the LSM6DS3TR library provided by Adafruit. This connection is set up using the code found in the Examples folder. The microcontroller is then connected to through the Bluetooth Classic communication standard using an external Python script and the transmission of data is begun. This, as well as the functionality for the receiver was accomplished through the PyBluez library. 1000 points of this data is then plotted using the matplotlib Python library in order to verify the data being received.

## Known Bugs
