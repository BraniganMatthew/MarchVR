import socket
import sys

HOST = "0.0.0.0"  # Standard loopback interface address (localhost)
PORT = int(sys.argv[1])  # Port to listen on (non-privileged ports are > 1023)

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((HOST, PORT))
    print("Waiting for connection")
    s.listen(5)
    conn, addr = s.accept()
    print("Accepted connection")
    conn.send("Hello client")
    data = conn.recv(1024)
    print(data)

except KeyboardInterrupt:
    print("Exiting")
    s.close()