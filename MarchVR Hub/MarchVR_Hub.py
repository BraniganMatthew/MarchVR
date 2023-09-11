from PySide6.QtWidgets import *
from PySide6.QtGui import QKeySequence, QPalette, QColor, QAction, QFont
from PySide6.QtCore import QtMsgType
from PySide6 import QtCore
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
import sys
import time
import asyncio
from bleak import BleakClient, BleakScanner
import threading

serviceUUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
characteristicUUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

def listToString(inputList):
    outputString = ""
    for i in inputList:
        outputString += chr(i)
    return outputString

statusString = ""
def notification_handler(sender, data):
            global statusString
            statusString = listToString(list(data))
            print(statusString)

winWidth = 960
winHeight = 540
winX = 280
winY = 160

green = QColor(0, 200, 20)
red = QColor(200, 5, 5)

async def main():
    scanner = BleakScanner(service_uuids=serviceUUID, winrt=dict(use_cached_services=False))
    server = await scanner.find_device_by_name("ESP32")
    async with BleakClient(server) as client:
        print("Printing services: ")
        services = client.services
        for s in services:
            print(s)

        print("Printing characteristic: ")
        try: 
            await client.start_notify(characteristicUUID, notification_handler)
            while(True):
                await asyncio.sleep(1)
                
                
        except:
            print("Exited")
            await client.disconnect()

class Worker(QtCore.QObject):
    def run(self):
        asyncio.run(main())

class Canvas(FigureCanvas):
    def __init__(self, parent, title, x, y, w, h):
        fig, self.ax = plt.subplots(figsize = (3,3), dpi=100)
        super().__init__(fig)
        self.setParent(parent)
        self.ax.set_axis_off()

        """ 
        Matplotlib Script
        """
        t = np.arange(0.0, 2.0, 0.01)
        s = 1 + np.sin(2 * np.pi * t)
        
        self.ax.plot(t, s)

        self.ax.set(xlabel='time (s)', ylabel='voltage (mV)')
        self.ax.set_title(title, fontsize = 10)
        self.ax.grid()
        self.setGeometry(x, y, w, h)

class Window(QMainWindow):
    def runner(self):
        self.thread = QtCore.QThread()
        self.worker = Worker()
        self.worker.moveToThread(self.thread)
        self.thread.started.connect(self.worker.run)
        #self.worker.finished.connect(self.thread.quit)
        #self.worker.finished.connect(self.worker.deleteLater)
        self.thread.finished.connect(self.thread.deleteLater)
        #self.worker.progress.connect(self.reportProgress)
        # Step 6: Start the thread
        self.thread.start()

    def __init__(self):
        super().__init__()
  
        # setting title
        self.setWindowTitle("March VR Hub")
  
        # setting geometry
        self.setGeometry(winX, winY, winWidth, winHeight)
        self.setFixedSize(winWidth, winHeight)
  
        # calling method
        self.UiComponents()
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update)
        self.timer.start(1000)
  
        # showing all the widgets
        self.show()
        self.runner()
  
    # method for widgets
    def UiComponents(self):
        
        # CALIBRATE BUTTON

        # creating a push button
        calButton = QPushButton("Recalibrate", self)

        calButtonWidth = 100
        calButtonHeight = 40
        calButtonX = 25
        calButtonY = 100
  
        # setting geometry of button
        calButton.setGeometry(calButtonX, calButtonY, calButtonWidth, calButtonHeight)
        calButton.setStyleSheet("QPushButton{font-size: 12pt;}")
  
        # adding action to a button
        calButton.clicked.connect(self.clickCal)

        # CONNECT BUTTON -------------------------------------------------------------------------------

        conButton = QPushButton("Connect", self)

        conButtonWidth = 100
        conButtonHeight = 40
        conButtonX = 25
        conButtonY = 150
  
        # setting geometry of button
        conButton.setGeometry(conButtonX, conButtonY, conButtonWidth, conButtonHeight)
        conButton.setStyleSheet("QPushButton{font-size: 12pt;}")
  
        # adding action to a button
        conButton.clicked.connect(self.clickCon)

        # MAIN TEXT -------------------------------------------------------------------------------
        mainX = winWidth/2 - 80
        mainY = 0
        mainText = QLabel("March VR Hub", self)
        mainText.setGeometry(mainX, mainY, 200, 50)
        mainText.setStyleSheet("QLabel{font-size: 18pt;}")
        mainText.setAlignment(QtCore.Qt.AlignCenter)

        # TRACKER 1 INFO -------------------------------------------------------------------------------
        t1x = 25
        t1y = 200
        t1 = QLabel("Tracker 1: ", self)
        t1.setGeometry(t1x, t1y, 200, 50)
        t1.setStyleSheet("QLabel{font-size: 12pt;}")

        # connected text
        t1con = QLabel("Connected", self)
        t1con.setGeometry(t1x + 70, t1y, 200, 50)
        t1con.setStyleSheet("QLabel{font-size: 12pt; color: green}")

        # not connected text
        global t1discon
        t1discon = QLabel("Disonnected", self)
        t1discon.setGeometry(t1x + 70, t1y, 200, 50)
        t1discon.setStyleSheet("QLabel{font-size: 12pt; color: red;}")
        t1discon.hide()

        # battery info
        battery1 = -1
        t1bat = QLabel("Battery Life: 100%", self)
        t1bat.setGeometry(t1x, t1y + 30, 200, 50)
        t1bat.setStyleSheet("QLabel{font-size: 12pt; color: black;}")

        # TRACKER 2 INFO  -------------------------------------------------------------------------------
        t2x = 25
        t2y = 275
        t2 = QLabel("Tracker 2: ", self)
        t2.setGeometry(t2x, t2y, 200, 50)
        t2.setStyleSheet("QLabel{font-size: 12pt;}")

        # connected text
        t2con = QLabel("Connected", self)
        t2con.setGeometry(t2x + 70, t2y, 200, 50)
        t2con.setStyleSheet("QLabel{font-size: 12pt; color: green}")

        # not connected text
        t2discon = QLabel("Disonnected", self)
        t2discon.setGeometry(t2x + 70, t2y, 200, 50)
        t2discon.setStyleSheet("QLabel{font-size: 12pt; color: red}")
        t2discon.hide()

        # battery info
        battery2 = -1
        t2bat = QLabel("Battery Life: 100%", self)
        t2bat.setGeometry(t2x, t2y + 30, 200, 50)
        t2bat.setStyleSheet("QLabel{font-size: 12pt; color: black;}")

        # STATUS UPDATES
        global statusLabel
        global statusString
        statusString = ""
        statusLabel = QLabel(statusString, self)
        statusLabel.setGeometry(t2x, t2y, 400, 150)
        statusLabel.setStyleSheet("QLabel{font-size: 12pt; color: black}")

        chart1a = Canvas(self, "Tracker 1 Accelerometer", 400, 50, 200, 200)
        chart2a = Canvas(self, "Tracker 2 Accelerometer", 650, 50, 200, 200)
        chart1g = Canvas(self, "Tracker 1 Gyroscope", 400, 300, 200, 200)
        chart2g = Canvas(self, "Tracker 2 Gyroscope", 650, 300, 200, 200)

    def update(self):
        global statusString
        statusLabel.setText(statusString)
        pass
        #print("sdgs")
  
    
    # action method
    def clickme(self):
  
        # printing pressed
        print("pressed")
    
    def clickCal(self):
        print("Recalibrate pressed")

    def clickCon(self):
        print("Connect pressed")   

#App = QApplication(sys.argv)        
    
  
# create pyqt5 app
App = QApplication(sys.argv)

# demo = AppDemo()
# demo.show()
  
# create the instance of our Window
window = Window()

#--------------------------

# THREADING ATTEMPT ---------------------------------------

# def x():
#     asyncio.run(main())

# def y():
#     sys.exit(App.exec())

# threads = []
# thread = threading.Thread(target=y)
# threads.append(thread)
# thread2 = threading.Thread(target=x)
# threads.append(thread2)
# for thread in threads:
#     thread.start()

# for thread in threads:
#     thread.join()

#-------------------------------------
#---------------

#asyncio.run(main())

# start the app
sys.exit(App.exec())