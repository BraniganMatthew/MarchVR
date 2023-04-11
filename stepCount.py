# Jibin Alex (Modified by Matthew Branigan)(Modified by Maria Carmona)
# Test program that is supposed to receive data through Bluetooth
# Follow the steps to download the PyBluez library and all of its dependencies prior to running the program
# Program uses matplotlib python library to graph data points for accelerometer and gyroscope data received from the LSM6DSTR
import bluetooth
import numpy as np
import matplotlib.pyplot as plt
import time
import dataConversions
from collections import deque
from itertools import count
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
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
        print("Device Found.")
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
def recdata(dat): # function that calls recv() to obtain data from ESP32 and then performs filtering to make sure no incorrect data is used
    #start = time.time()
    #data = s.recv(64)
    data = s.recv(4).decode(float)
    dat = 0.0
    arr = []
    if (len(data) == 4):
        for i in range(4):
            #print(hex(data[i]), end=" ")
            arr.append(hex(data[i])[2::])
        #print(str(arr) + "-")
        #print(dataConversions.hexPairsToFloat(arr[3], arr[2], arr[1], arr[0]))
    #sprint("")
    #print(str(data))
    #decode(data)
        dat = dataConversions.hexPairsToFloat(arr[3], arr[2], arr[1], arr[0])
    return dat

accx = 0.0
accy = 0.0
accz = 0.0
gyrx = 0.0
gyry = 0.0
gyrz = 0.0

lower = 8
upper = 12
stepCount = 0
above = True

while True:
    accx = recdata(accx)
    accy = recdata(accy)
    accz = recdata(accz)
    gyrx = recdata(gyrx)
    gyry = recdata(gyry)
    gyrz = recdata(gyrz)

    # This is the step counting/detection, checks if we are crossing a boundary
    if(above):
        if accz < lower:
            # send step
            #stepCount+=1
            above = False
    else:
        if accz > upper:
            # send step
            stepCount+=1
            above = True
            print("STEP COUNT: " + str(stepCount))
    