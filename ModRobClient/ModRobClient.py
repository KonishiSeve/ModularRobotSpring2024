import time
import socket

class ModRob:
    def __init__(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self.socket.settimeout(0.2)
    
    def module_discovery(self):
        pass

    def structure_discovery(self, timeout=0.1):
        self.socket.sendto(b'\x00', ("10.42.0.255", 9999))
        start_time = time.time()
        while(time.time()-start_time < timeout):
            try:
                data, server = self.socket.recvfrom(1024)
                print(data)
                print(server)
            except:
                return

    def write_command(self):
        pass

    def read_data(self):
        pass


if __name__ == "__main__":
    robot = ModRob()
    robot.structure_discovery()