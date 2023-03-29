# Jibin Alex (Modified by Matthew Branigan)
# Test program that is supposed to receive data through Bluetooth
# Follow the steps to download the PyBluez library and all of its dependencies prior to running the program

import bluetooth

ESP32MACaddress = "E8:9F:6D:26:9F:1A"
s = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
s.connect((ESP32MACaddress, 1))

try:
    while True:
        data = s.recv(64)
        if (data and len(data) >= 5 and len(data) < 8):
            print(data)
        #else:
            #print(len(data))

except KeyboardInterrupt:
    print("Execution interrupted")

# https://blog.kevindoran.co/bluetooth-programming-with-python-3/