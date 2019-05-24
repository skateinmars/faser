# -*- coding: utf-8 -*-

# LDUR sensis: 700, 550, 875, 900

import serial

ser = serial.Serial("/dev/ttyACM0", timeout=1)
ser.setDTR(1)

buf = ""

command = "0750\n".encode()
ser.write(command)
print(ser.read(92).decode())

command = "1750\n".encode()
ser.write(command)
# buf = buf + ser.readline(128).decode()
print(ser.read(92).decode())

command = "2875\n".encode()
ser.write(command)
print(ser.read(92).decode())

command = "3925\n".encode()
ser.write(command)
print(ser.read(92).decode())

# print(buf)

ser.close()
