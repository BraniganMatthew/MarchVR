# Jibin Alex
# Incomplete test program that is supposed to receive data through Bluetooth

import socket

ESP32MACaddress = 'E8:9F:6D:26:9F:1A'
s = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
s.connect((ESP32MACaddress, 1))

try:
    while True:
        data = s.recv(1024)
        if (data):
            print(data)

except KeyboardInterrupt:
    socket.close()