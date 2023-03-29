# Jibin Alex
# Test program that is supposed to receive data through Bluetooth
# Follow the steps to download the PyBluez library and all of its dependencies prior to running the program

import bluetooth

ESP32MACaddress = "E8:9F:6D:26:9F:1A"
s = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
s.connect((ESP32MACaddress, 1))

try:
    while True:
        data = s.recv(1)
        if (data):
            print(data)

except KeyboardInterrupt:
    print("Execution interrupted")

# https://blog.kevindoran.co/bluetooth-programming-with-python-3/

# pip install git+https://github.com/pybluez/pybluez.git#egg=pybluez