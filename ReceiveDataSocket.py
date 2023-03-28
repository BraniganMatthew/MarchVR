# Jibin Alex
# Incomplete test program that is supposed to receive data through Bluetooth
# This program does not work on Windows systems due to WSL not exposing Bluetooth hardware, thus preventing usage of the socket library
# The PyBluez version of this program will work provided that the libraries are correctly installed.

import socket

ESP32MACaddress = 'E8:9F:6D:26:9F:1A'
s = socket.socket(socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
# socket.AF_BLUETOOTH, 
s.connect((ESP32MACaddress, 1))

try:
    while True:
        data = s.recv(32)
        if (data):
            print(data)

except KeyboardInterrupt:
    socket.close()

# https://blog.kevindoran.co/bluetooth-programming-with-python-3/