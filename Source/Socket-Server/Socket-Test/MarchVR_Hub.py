from PySide6.QtWidgets import *
from PySide6.QtGui import QColor
from PySide6 import QtCore
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
import sys
import asyncio
from bleak import BleakClient, BleakScanner
import socket

calibrateFlag = False
connectFlag = False
disconnectFlag = True
errorFlag = False

serviceUUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
characteristicUUID = "22d7a034-791d-49f6-a84e-ef78ab2473ad"

def listToString(inputList): # Helps turn a byte stream into a string of characters
    outputString = ""
    for i in inputList:
        outputString += chr(i)
    return outputString

movementString = ""
def notification_handler(sender, data): # Called when we are notified by a tracker via BLE
            global movementString
            movementString = listToString(list(data))

winWidth = 250
winHeight = 400
winX = 280
winY = 160
green = QColor(0, 200, 20)
red = QColor(200, 5, 5)
updateTime = 1000 # in milliseconds - update our UI once every "updateTime" ms
sum = 0

async def main(): # This function loops for the duration the UI is open and handles BLE communication
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect(("localhost", 8080))
    while (True):
        try:
            client_socket.send("some more data")
        except:
            # recreate the socket and reconnect
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect("localhost", 8080)
            client_socket.send("some more data")
        # send message
        message = "This test works"
        client_socket.send(message.encode())
        # receive message
        data = client_socket.recv(128).decode()
        print(data)
        if not data:
            break
        print("Received in python:", data)
        global errorFlag
        if (errorFlag):
            t1error.show()
            t2error.show()
            t1con.hide()
            t2con.hide()
            t1discon.hide()
            t2discon.hide()
            continue
        elif (connectFlag):
            scanner = BleakScanner(service_uuids=serviceUUID, winrt=dict(use_cached_services=False))
            server = await scanner.find_device_by_name("ESP32")
            if (server == None): 
                await asyncio.sleep(10)
                continue
            else:
                t1con.show()
                t2con.show()
                t1discon.hide()
                t2discon.hide()
            try:
                async with BleakClient(server) as client:
                    await client.start_notify(characteristicUUID, notification_handler)
                    while(True):
                        global calibrateFlag
                        if (calibrateFlag and connectFlag):
                            print("Calibrating")
                            calibrateFlag = 0
                            calibrationCommand = "%;GUI;TK1;CAL;0;" + str(sum)
                            calibrationCommand = calibrationCommand.encode('utf-8')
                            await client.write_gatt_char(characteristicUUID, bytearray(calibrationCommand), response=True)
                        elif (calibrateFlag and disconnectFlag):
                            print("Error: Calibrate command sent while disconnected")
                        elif (disconnectFlag):
                            print("Disconnected")
                            if (server != None):
                                await client.disconnect()
                            break
                        await asyncio.sleep(1)
            except Exception as ex:
                template = "An exception of type {0} occurred. Arguments:\n{1!r}"
                message = template.format(type(ex).__name__, ex.args)
                print(message)
                errorFlag = True
                continue
        else:
            t1disconnecting.hide()
            t2disconnecting.hide()
            t1discon.show()
            t2discon.show()


class Worker(QtCore.QObject): # This class and its function help set up/enable threading

    def run(self):
        asyncio.run(main())


class Canvas(FigureCanvas): # A class for our charts

    def __init__(self, parent, title, x, y, w, h):
        fig, self.ax = plt.subplots(figsize = (3,3), dpi=100)
        super().__init__(fig)
        self.setParent(parent)
        self.ax.set_axis_off()

        # Matplotlib Stuff
        t = np.arange(0.0, 2.0, 0.01)
        s = 1 + np.sin(2 * np.pi * t)
        
        self.ax.plot(t, s)

        self.ax.set(xlabel='time (s)', ylabel='voltage (mV)')
        self.ax.set_title(title, fontsize = 10)
        self.ax.grid()
        self.setGeometry(x, y, w, h)


class Window(QMainWindow): # This is our actual window for the UI

    def runner(self): # Setting up and starting up the thread
        self.thread = QtCore.QThread()
        self.worker = Worker()
        self.worker.moveToThread(self.thread)
        self.thread.started.connect(self.worker.run)
        self.thread.finished.connect(self.thread.deleteLater)
        self.thread.start() # Starting the thread


    def __init__(self): # The window's init function, handles several start up tasks
        super().__init__()
  
        # setting title
        self.setWindowTitle("MarchVR Hub")
  
        # setting geometry
        self.setGeometry(winX, winY, winWidth, winHeight)
        self.setFixedSize(winWidth, winHeight)
  
        # calling method
        self.UiComponents()
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update)
        self.timer.start(updateTime)
  
        # showing all the widgets
        self.show()

        # call our function that starts the thread to handle BLE communication on the side
        self.runner()

        # Calculates calibration checksum
        sumList = ["GUI", "TK1", "CAL", "0"]
        for x in sumList:
            for i in range(0, len(x)):
                global sum
                sum = sum ^ ord(x[i])

    def UiComponents(self): # This function creates every UI component within the window

        ### ----------------------------------- UI COMPONENTS ---------------------------------------- ###

        # CALIBRATE BUTTON -------------------------------------------------------------------------------

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

        # DISCONNECT BUTTON ---------------------------------------------------------------------------------

        disconButton = QPushButton("Disconnect", self)

        disconButtonWidth = 100
        disconButtonHeight = 40
        disconButtonX = 125
        disconButtonY = 150
  
        # setting geometry of button
        disconButton.setGeometry(disconButtonX, disconButtonY, disconButtonWidth, disconButtonHeight)
        disconButton.setStyleSheet("QPushButton{font-size: 12pt;}")
  
        # adding action to a button
        disconButton.clicked.connect(self.clickDiscon)

        # CONNECT BUTTON ---------------------------------------------------------------------------------

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

        # MAIN TEXT -------------------------------------------------------------------------------------
        mainX = winWidth/2 - 100
        mainY = 0
        mainText = QLabel("MarchVR Hub", self)
        mainText.setGeometry(mainX, mainY, 200, 50)
        mainText.setStyleSheet("QLabel{font-size: 18pt;}")
        mainText.setAlignment(QtCore.Qt.AlignCenter)

        # WARNING TEXT -------------------------------------------------------------------------------------
        warningX = 0
        warningY = 35
        warningText = QLabel("Stand still while calibrating.\nIf an error occurs, please restart MarchVR Hub.", self)
        warningText.setGeometry(warningX, warningY, 250, 50)
        warningText.setStyleSheet("QLabel{font-size: 8pt;}")
        warningText.setAlignment(QtCore.Qt.AlignCenter)

        # TRACKER 1 INFO --------------------------------------------------------------------------------
        t1x = 25
        t1y = 200
        t1 = QLabel("Tracker 1: ", self)
        t1.setGeometry(t1x, t1y, 200, 50)
        t1.setStyleSheet("QLabel{font-size: 12pt;}")

        # connected text
        global t1con
        t1con = QLabel("Connected", self)
        t1con.setGeometry(t1x + 70, t1y, 200, 50)
        t1con.setStyleSheet("QLabel{font-size: 12pt; color: green}")
        t1con.hide()

        # not connected text
        global t1discon
        t1discon = QLabel("Disconnected", self)
        t1discon.setGeometry(t1x + 70, t1y, 200, 50)
        t1discon.setStyleSheet("QLabel{font-size: 12pt; color: red;}")

        # not connected text
        global t1disconnecting
        t1disconnecting = QLabel("Disconnecting...", self)
        t1disconnecting.setGeometry(t1x + 70, t1y, 200, 50)
        t1disconnecting.setStyleSheet("QLabel{font-size: 12pt; color: red;}")
        t1disconnecting.hide()

        # error text
        global t1error
        t1error = QLabel("ERROR", self)
        t1error.setGeometry(t1x + 70, t1y, 200, 50)
        t1error.setStyleSheet("QLabel{font-size: 12pt; color: red}")
        t1error.hide()

        # TRACKER 2 INFO  -------------------------------------------------------------------------------
        t2x = 25
        t2y = 225
        t2 = QLabel("Tracker 2: ", self)
        t2.setGeometry(t2x, t2y, 200, 50)
        t2.setStyleSheet("QLabel{font-size: 12pt;}")

        # connected text
        global t2con
        t2con = QLabel("Connected", self)
        t2con.setGeometry(t2x + 70, t2y, 200, 50)
        t2con.setStyleSheet("QLabel{font-size: 12pt; color: green}")
        t2con.hide()

        # not connected text
        global t2discon
        t2discon = QLabel("Disconnected", self)
        t2discon.setGeometry(t2x + 70, t2y, 200, 50)
        t2discon.setStyleSheet("QLabel{font-size: 12pt; color: red}")
        t2discon.hide()

         # not connected text
        global t2disconnecting
        t2disconnecting = QLabel("Disconnecting...", self)
        t2disconnecting.setGeometry(t2x + 70, t2y, 200, 50)
        t2disconnecting.setStyleSheet("QLabel{font-size: 12pt; color: red}")
        t2disconnecting.hide()

        # error text
        global t2error
        t2error = QLabel("ERROR", self)
        t2error.setGeometry(t2x + 70, t2y, 200, 50)
        t2error.setStyleSheet("QLabel{font-size: 12pt; color: red}")
        t2error.hide()

        # MOVEMENT DATA  -------------------------------------------------------------------------------
        global movementLabel
        global movementString
        movementString = ""
        movementLabel = QLabel("Movement Data", self)
        movementLabel.setGeometry(t2x, t2y - 40, 200, 200)
        movementLabel.setStyleSheet("QLabel{font-size: 12pt; color: black}")
        movementLabel.setAlignment(QtCore.Qt.AlignCenter)

        # Yaw  -------------------------------------------------------------------------------
        global yawLabel
        yawLabel = QLabel("Yaw: ", self)
        yawLabel.setGeometry(t2x, t2y - 20, 200, 200)
        yawLabel.setStyleSheet("QLabel{font-size: 12pt; color: black}")

        # Pitch  -------------------------------------------------------------------------------
        global pitchLabel
        pitchLabel = QLabel("Pitch: ", self)
        pitchLabel.setGeometry(t2x, t2y, 200, 200)
        pitchLabel.setStyleSheet("QLabel{font-size: 12pt; color: black}")

        # Roll  -------------------------------------------------------------------------------
        global rollLabel
        rollLabel = QLabel("Roll: ", self)
        rollLabel.setGeometry(t2x, t2y + 20, 200, 200)
        rollLabel.setStyleSheet("QLabel{font-size: 12pt; color: black}")

        # Speed  -------------------------------------------------------------------------------
        global speedLabel
        speedLabel = QLabel("Speed: ", self)
        speedLabel.setGeometry(t2x, t2y + 40, 200, 200)
        speedLabel.setStyleSheet("QLabel{font-size: 12pt; color: black}")

    def update(self): # Called once every "updateTime" ms to update the UI
        global movementString
        movementList = movementString.split(";")
        if (len(movementList) >= 4):
            if (movementList[3] == "MOT"):
                yawLabel.setText("Yaw: " + movementList[5])
                pitchLabel.setText("Pitch: " + movementList[6])
                rollLabel.setText("Roll: "  + movementList[7])
                speedLabel.setText("Speed: "  + movementList[8])

    def clickCal(self): # Called when the calibration button is clicked
        print("Recalibrate clicked")
        global calibrateFlag
        calibrateFlag = True

    def clickCon(self): # Called when the connect button is clicked
        print("Connect clicked")
        global connectFlag
        connectFlag = True
        global disconnectFlag
        disconnectFlag = False

    def clickDiscon(self): # Called when the connect button is clicked
        print("Disconnect clicked")
        global connectFlag
        connectFlag = False
        global disconnectFlag
        disconnectFlag = True
        t1con.hide()
        t2con.hide()
        t1disconnecting.show()
        t2disconnecting.show()

# create pyqt5 app
App = QApplication(sys.argv)
  
# create the instance of our Window
window = Window()

# start the app
sys.exit(App.exec())