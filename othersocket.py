import socket

portnum = 5555
connected = False

while(not connected):
    try:
        connected = True
        socky = socket.create_connection(('localhost', portnum))
        socky.settimeout(2.0)
    except:
        connected = False
        portnum += 1
        print("Connection failed, trying port: " + str(portnum))

    socky.send(str(portnum) + "\n")
    if (socky.recv(1023) == "Correct Port Num"):
        print("Successful Connection on port : " + str(portnum))
    else:
        portnum += 1
        print("Connection failed... restarting connection on port: " + str(portnum))
        connected = False
