# Jibin Alex
# Test program using the PyBluez and Numpy libraries that receives 4 uints then pieces them together into a float
# Attempted method to transfer floating point data
# Commented code below is C++ method for converting an array of uints to a float in this manner

import bluetooth
import numpy

ESP32MACaddress = "E8:9F:6D:26:9F:1A"
s = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
s.connect((ESP32MACaddress, 1))

try:
    while True:
        while data != "\0":
            data = s.recv(64)
        data = s.recv(1)
        if (data):
            data1 = numpy.uint8(data)
        data = s.recv(1)
        if (data):
            data2 = numpy.uint8(data)
        data = s.recv(1)
        if (data):
            data3 = numpy.uint8(data)
        data = s.recv(1)
        if (data):
            data4 = numpy.uint8(data)
        unint8_data = numpy.array([data1, data2, data3, data4], dtype='uint8')

except KeyboardInterrupt:
    print("Execution interrupted")
    s.close()

# https://blog.kevindoran.co/bluetooth-programming-with-python-3/

# pip install git+https://github.com/pybluez/pybluez.git#egg=pybluez



# float IEEE11073_2float(uint8_t *dat)
# {
#   int32_t Mantissa = (dat[2] << 16 | dat[1] << 8 | dat[0]);
#   uint8_t Neg = bitRead(dat[2],7);
#   int8_t fExp = dat[3];
#   if (Neg) Mantissa |= 255 << 24;
#   return (float(Mantissa) * pow(10, fExp));
# }