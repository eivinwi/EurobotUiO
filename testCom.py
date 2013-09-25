#!/usr/bin/python
import serial

ser = serial.Serial('/dev/ttyACM0', 9600, timeout=2)

i = 0

while(1):
	line = ser.readline()
	print i
	i += 1
	if i == 5:
		ser.write('-')
		i = 0