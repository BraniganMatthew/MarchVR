# Jibin Alex (Modified by Matthew Branigan)(Modified by Maria Carmona)
# Test program that is supposed to receive data through Bluetooth
# Follow the steps to download the PyBluez library and all of its dependencies prior to running the program

import bluetooth
import numpy as np
import matplotlib.pyplot as plt
import time

ESP32MACaddress = "E8:9F:6D:26:9F:1A"
s = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
s.connect((ESP32MACaddress, 1))
accx = 0.0
accy = 0.0
accz = 0.0
gyrx = 0.0
gyry = 0.0
gyrz = 0.0
tax = 0.0
tay = 0.0
taz = 0.0
tgx = 0.0
tgy = 0.0
tgz = 0.0
start = time.time()
data = s.recv(64)
data = float(data[1:-1])
end = time.time()
if (data and len(data) >= 5 and len(data) < 8):
    accx = data
    tax = end - start
start = time.time()
data = s.recv(64)
data = float(data[2:-1])
end = time.time()
if (data and len(data) >= 5 and len(data) < 8):
    accy = data
    tay = end - start
start = time.time()
data = s.recv(64)
data = float(data[2:-1])
end = time.time()
if (data and len(data) >= 5 and len(data) < 8):
    accz = data
    taz = end - start
start = time.time()
data = s.recv(64)
data = float(data[2:-1])
end = time.time()
if (data and len(data) >= 5 and len(data) < 8):
    gyrx = data
    tgx = end - start
start = time.time()
data = s.recv(64)
data = float(data[2:-1])
end = time.time()
if (data and len(data) >= 5 and len(data) < 8):
    gyry = data
    tgy = end - start
start = time.time()
data = s.recv(64)
data = float(data[2:-1])
end = time.time()
if (data and len(data) >= 5 and len(data) < 8):
    gyrz = data
    tgz = end - start
    
plt.ion()
fig = plt.figure()
ax = fig.add_subplot(211)
xa, = ax.plot(accx, tax, color='r', label='X')
ya, = ax.plot(accy, tay, color='b', label='Y')
za, = ax.plot(accz, taz, color='g', label='Z')

ax1 = fig.add_subplot(212)
xg, = ax1.plot(gyrx, tgx, color='r', label='X')
yg, = ax1.plot(gyry, tgy, color='b', label='Y')
zg, = ax1.plot(gyrz, tgz, color='g', label='Z')

try:
    while True:
        start = time.time()
        data = s.recv(64)
        data = float(data[2:-1])
        end = time.time()
        if (data and len(data) >= 5 and len(data) < 8):
            accx = data
            tax = end - start
        start = time.time()
        data = s.recv(64)
        data = float(data[2:-1])
        end = time.time()
        if (data and len(data) >= 5 and len(data) < 8):
            accy = data
            tay = end - start
        start = time.time()
        data = s.recv(64)
        data = float(data[2:-1])
        end = time.time()
        if (data and len(data) >= 5 and len(data) < 8):
            accz = data
            taz = end - start
        start = time.time()
        data = s.recv(64)
        data = float(data[2:-1])
        end = time.time()
        if (data and len(data) >= 5 and len(data) < 8):
            gyrx = data
            tgx = end - start
        start = time.time()
        data = s.recv(64)
        data = float(data[2:-1])
        end = time.time()
        if (data and len(data) >= 5 and len(data) < 8):
            gyry = data
            tgy = end - start
        start = time.time()
        data = s.recv(64)
        data = float(data[2:-1])
        end = time.time()
        if (data and len(data) >= 5 and len(data) < 8):
            gyrz = data
            tgz = end - start
        xa.set_xdata(accx)
        xa.set_ydata(tax)
        ya.set_xdata(accy)
        ya.set_ydata(tay)
        za.set_xdata(accz)
        za.set_ydata(taz)
        xg.set_xdata(gyrx)
        xg.set_ydata(tgx)
        yg.set_xdata(gyry)
        yg.set_ydata(tgy)      
        zg.set_xdata(gyrz)
        zg.set_ydata(tgz)

        fig.canvas.draw()
        fig.canvas.flush_events()
        time.sleep(0.1)
except KeyboardInterrupt:
    print("Execution interrupted")

# https://blog.kevindoran.co/bluetooth-programming-with-python-3/
