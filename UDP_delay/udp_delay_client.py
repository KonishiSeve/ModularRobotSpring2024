import socket
import time
import serial


sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.settimeout(2)

#ser = serial.Serial("COM10", 9600)

sock.sendto(b'\x00', ("10.42.0.255", 9999))
#ser.write(bytearray('S','ascii'))
time.sleep(0.2)