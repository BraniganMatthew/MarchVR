# Jibin Alex (Modified by Matthew Branigan)(Modified by Maria Carmona)
# Test program that is supposed to receive data through Bluetooth
# Follow the steps to download the PyBluez library and all of its dependencies prior to running the program
# Program uses matplotlib python library to graph data points for accelerometer and gyroscope data received from the LSM6DSTR
import bluetooth
import numpy as np
import matplotlib.pyplot as plt
import time
import dataConversions
# Creating connection with ESP32
ESP32MACaddress = "E8:9F:6D:26:9F:1A"
DeviceName = "MarchVR Best Team"
print("Looking for device...")
DeviceFound = False
DeviceAddr = ""

while True:
    res = bluetooth.discover_devices(3, False, True, False)
    for s in res:
        if s[1] == DeviceName:
            DeviceFound = True
            DeviceAddr = s[0]
            break
    if DeviceFound:
        print("Device Found!")
        break
    else:
        print("Device Missing...Retrying in 2 seconds")
        time.sleep(2)
        print("Retrying now...")


s = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
s.connect((DeviceAddr, 1))
def conv(s):
    try:
        s=float(s)
        #print('Correct'+str(s))
        return True
    except ValueError:
        #print('Error'+str(s))
        return False
def recdata(dat, t): # function that calls recv() to obtain data from ESP32 and then performs filtering to make sure no incorrect data is used
    start = time.time()
    #data = s.recv(64)
    data = s.recv(4)
    arr = []
    if (len(data) == 4):
        for i in range(4):
            #print(hex(data[i]), end=" ")
            arr.append(hex(data[i])[2::])
        #print(str(arr) + "-")
        print(dataConversions.hexPairsToFloat(arr[3], arr[2], arr[1], arr[0]))
    #print("")
    #print(str(data))
    #decode(data)
    end = time.time()
    dat = dataConversions.hexPairsToFloat(arr[3], arr[2], arr[1], arr[0])
    # if (data and len(data) >= 5 and len(data) < 8):
    #     bl = conv(data)
    #     if (bl == True):
    #         if (float(data)>10):
    #             dat = 10.0
    #         else:
    #             dat = float(data)
    t = end - start
    return dat, t

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
bl = False

accxx = []
taxx = []
accyy = []
tayy = []
acczz = []
tazz = []
gyrxx = []
tgxx = []
gyryy = []
tgyy = []
gyrzz = []
tgzz = []

try:
    #collecting data points
    for i in range(1000):
        accx, tax = recdata(accx, tax)
        accy, tay = recdata(accy, tay)
        accz, taz = recdata(accz, taz)
        gyrx, tgx = recdata(gyrx, tgx)
        gyry, tgy = recdata(gyry, tgy)
        gyrz, tgz = recdata(gyrz, tgz)
        accxx.append(accx)
        accyy.append(accy)
        acczz.append(accz)
        taxx.append(tax)
        tayy.append(tay)
        tazz.append(taz)
        gyrxx.append(gyrx)
        gyryy.append(gyry)
        gyrzz.append(gyrz)
        tgxx.append(tgx)
        tgyy.append(tgy)
        tgzz.append(tgz)

    # Graphing scatter subplots
    fig = plt.figure()
    #Accelerometer plot
    ax1 = fig.add_subplot(211)
    ax1.scatter(taxx, accxx, c='b')
    ax1.scatter(tayy, accyy, c='r')
    ax1.scatter(tazz, acczz, c='g')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('m/s^2')
    ax1.set_title('Accelerometer Data')
    plt.legend(["x","y","z"])
    #Gyroscope plot
    ax2 = fig.add_subplot(212)
    ax2.scatter(tgxx, gyrxx, c='b')
    ax2.scatter(tgyy, gyryy, c='r')
    ax2.scatter(tgzz, gyrzz, c='g')
    ax2.set_xlabel('Time (s)')
    ax2.set_ylabel('degrees/s')
    ax2.set_title('Gyroscope Data')
    plt.legend(["x","y","z"])
    plt.show()
except KeyboardInterrupt:
    print("Execution interrupted")

# https://blog.kevindoran.co/bluetooth-programming-with-python-3/
