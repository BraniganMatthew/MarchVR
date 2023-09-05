from PySide6.QtWidgets import *
from PySide6.QtGui import QKeySequence, QPalette, QColor, QAction, QFont
from PySide6.QtCore import QtMsgType
from PySide6 import QtCore, QtGui, QtWidgets
import pyqtgraph
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from BlurWindow.blurWindow import blur
import socket
import sys

# SOCKET STUFF
# socky = socket.create_connection(('10.245.26.51', 5555))
# socky = socket.create_connection(('10.245.26.51', 5555))
# socky.settimeout(2.0)
# socky.send((":SOUR1:FREQ 100\n").encode('utf-8'))

# # Create a socket object
# server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# # Define the server address and port
# server_address = ('localhost', 12345)  # Change this to the actual server address and port

# # Connect to the server
# server_socket.connect(server_address)

# Define a buffer size for receiving data
buffer_size = 1024

winWidth = 960
winHeight = 540
winX = 280
winY = 160

green = QColor(0, 200, 20)
red = QColor(200, 5, 5)

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
    def __init__(self):
        super().__init__()
  
        # setting title
        self.setWindowTitle("March VR Hub")
        #self.setStyleSheet("background-color: white;")

        # BLURRING 
        #blur(self.winId())
        #self.setStyleSheet("background-color: rgba(0, 0, 0, 0)")
  
        # setting geometry
        self.setGeometry(winX, winY, winWidth, winHeight)
        self.setFixedSize(winWidth, winHeight)
  
        # calling method
        self.UiComponents()
  
        # showing all the widgets
        self.show()
  
    # method for widgets
    def UiComponents(self):
        
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

        # TRACKER 1 TEXT -------------------------------------------------------------------------------
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
        t1discon.setStyleSheet("QLabel{font-size: 12pt color: red;}")
        t1discon.hide()

        # TRACKER 2 TEXT -------------------------------------------------------------------------------
        t2x = 25
        t2y = 225
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

        chart1a = Canvas(self, "Tracker 1 Accelerometer", 400, 50, 200, 200)
        chart2a = Canvas(self, "Tracker 2 Accelerometer", 650, 50, 200, 200)
        chart1g = Canvas(self, "Tracker 1 Gyroscope", 400, 300, 200, 200)
        chart2g = Canvas(self, "Tracker 2 Gyroscope", 650, 300, 200, 200)
  
    
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

# start the app
sys.exit(App.exec())