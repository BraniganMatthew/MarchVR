import serial
ser = serial.Serial('/dev/rfcomm0' , 115200)
print(ser.name)

try:
    while True:
        ser_bytes = ser.readline()
        print(ser_bytes)

except KeyboardInterrupt:
    print("Execution interrupted")