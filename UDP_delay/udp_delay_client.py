import socket
import time
import serial
import threading

method = "broadcast" #"threaded" "single"
timeout = 2

def udp_tx(sock, ip):
    sock.sendto(b'\x00\x01', ("10.42.0.{0}".format(ip), 9999))

def udp_tx_rx(ip):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    sock.sendto(b'\x00\x01', ("10.42.0.{0}".format(ip), 9999))
    print(sock.recvfrom(1024))


if method == "broadcast":
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.sendto(b'\x00\x01', ("10.42.0.255", 9999))
    loop = True
    while loop:
        try:
            print(sock.recvfrom(1024))
        except:
            loop = False
            break

elif method == "single":
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    for i in range(2,255):
        sock.sendto(b'\x00\x01', ("10.42.0.{0}".format(i), 9999))
    loop = True
    while loop:
        try:
            print(sock.recvfrom(1024))
        except:
            loop = False
            break

elif method == "threaded_tx":
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    threads = []
    for i in range(2,255):
        threads.append(threading.Thread(target=udp_tx, args=(sock,i,)))
        threads[-1].start()
    for thread in threads:
        thread.join()
    loop = True
    while loop:
        try:
            print(sock.recvfrom(1024))
        except:
            loop = False
            break

elif method == "threaded_tx_rx":
    threads = []
    for i in range(2,255):
        threads.append(threading.Thread(target=udp_tx_rx, args=(i,)))
        threads[-1].start()
    for thread in threads:
        thread.join()

    

#ser = serial.Serial("COM10", 9600)
#ser.write(bytearray('S','ascii'))
#time.sleep(0.2)