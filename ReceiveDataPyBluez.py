# Jibin Alex
# Test program that is supposed to receive data through Bluetooth

import bluetooth

ESP32MACaddress = "E8:9F:6D:26:9F:1A"
s = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
s.connect((ESP32MACaddress, 1))

try:
    while True:
        data = s.recv(1024)
        if (data):
            print(data)

except KeyboardInterrupt:
    print("Execution interrupted")

# https://blog.kevindoran.co/bluetooth-programming-with-python-3/