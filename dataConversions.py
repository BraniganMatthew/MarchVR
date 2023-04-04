# Author - Patrick Harris
# Last modified 3/29/2023 by Patrick Harris
import struct

def hexPairsToFloat(a, b, c, d):
    binString = hexPairsToBinary(a, b, c, d)
    #binString = a << 8*3 + b << 8*2 + c << 8*1 + d << 8*0
    f = int(binString, 2)
    #f = int('0100 0001 1010 1100 0111 1010 1110 0001', 2)
    #          4     1    a   c     7    a    e    1
    #print(struct.unpack('f', struct.pack('I', f))[0])
    
    return struct.unpack('f', struct.pack('I', f))[0]

def hexPairsToBinary(a, b, c, d):
    return hexPairToBinaryString(a) + hexPairToBinaryString(b) + hexPairToBinaryString(c) + hexPairToBinaryString(d)

def hexPairToBinaryString(a):
    #print(len(a))
    if(len(a)==2):
        #print(decimalToBinaryString(hexCharToDecimal(a[0])*16 + hexCharToDecimal(a[1])))
        return decimalToBinaryString(hexCharToDecimal(a[0])*16 + hexCharToDecimal(a[1]))
    else:
        #return decimalToBinaryString(hexCharToDecimal([a[0]])*16 + hexCharToDecimal('0'))
        # print(a[0])
        # print(hexCharToDecimal(a[0]))
        # print(decimalToBinaryString(hexCharToDecimal(str(a[0]))))
        return decimalToBinaryString(hexCharToDecimal(str(a[0])))

def hexCharToDecimal(x):
    match x:
        case "0":
            return 0
        case "1":
            return 1
        case "2":
            return 2
        case "3":
            return 3
        case "4":
            return 4
        case "5":
            return 5
        case "6":
            return 6
        case "7":
            return 7
        case "8":
            return 8
        case "9":
            return 9
        case "a":
            return 10
        case "b":
            return 11
        case "c":
            return 12
        case "d":
            return 13
        case "e":
            return 14
        case "f":
            return 15
        case _:
            return -1
    
def decimalToBinaryString(x): # for 8 bits
    ans = bin(x)
    ans = ans[2::]
    numLeftTil8 = (len(ans) % 8)
    if(numLeftTil8 > 0):
        for i in range(8 - numLeftTil8):
            ans = "0" + ans
    return ans

# def decode():
#     for i in range(4):
#         data += s.recv(1)

# def decode(a):
#     if (len(a)):
#         //grab last 2 char and convert
#     else:
#         //


# print(hexCharToDecimal("f"))
# print(decimalToBinaryString(17))
# print(hexPairToBinaryString("02"))
# print(hexPairsToBinary("0f", "0f", "0f", "0f"))
print(hexPairsToFloat("ce", "f", "ed", "40")) # the actual function we need
#print(decimalToBinaryString(hexCharToDecimal('f')))
