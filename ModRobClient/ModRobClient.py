import time
import socket

def to_8bits_signed(value):
    if value > 127:
        return value -256
    return value

class ModRob:
    def __init__(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self.socket.settimeout(5)
    
    def module_discovery(self):
        pass

    def structure_discovery(self, timeout=0.2):
        self.socket.sendto(b'\x00', ("10.42.0.255", 9999))
        module_list = []
        start_time = time.time()
        while(time.time()-start_time < timeout):
            try:
                data, server = self.socket.recvfrom(1024)
                print(server)
                print(data)
                module_list.append(self.parse_structure_discovery(server[0].split(".")[-1], data))
            except:
                return module_list
        return module_list
    
    def parse_structure_discovery(self, module_id, data):
        data_index = 0
        ports_number = data[data_index]
        data_index += 1
        neighbours = [data[(i+data_index)] for i in range(ports_number)]
        data_index += ports_number

        ports_attributes_size = data[data_index]
        attribute_bytes_per_port = int(ports_attributes_size/ports_number)
        data_index += 1
        ports_attributes = []
        for _ in range(ports_number):
            ports_attributes.append([to_8bits_signed(data[data_index+i]) for i in range(attribute_bytes_per_port)])
            data_index += attribute_bytes_per_port

        module_attributes_size = data[data_index]
        data_index += 1
        module_attributes = [to_8bits_signed(data[data_index+i]) for i in range(module_attributes_size)]

        return {"module_id": int(module_id), "neighbours": neighbours, "ports_attributes": ports_attributes, "module_attributes": module_attributes}

    def write_command(self):
        pass

    def read_data(self):
        pass


if __name__ == "__main__":
    robot = ModRob()
    module_list = robot.structure_discovery()
    for module in module_list:
        print(module)