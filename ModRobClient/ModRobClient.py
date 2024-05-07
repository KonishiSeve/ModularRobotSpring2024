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
        self.socket.settimeout(0.1)
    
    def module_discovery(self, module_id, timeout=1):
        self.socket.sendto(b'\x00\x01', ("10.42.0.{0}".format(module_id), 9999))
        start_time = time.time()
        try:
            data, server = self.socket.recvfrom(1024)
        except:
            return None
        return self.parse_model_discovery(server[0].split(".")[-1], data)
    
    def parse_model_discovery(self, module_id, data):
        device_list = []
        index = 0
        id_counter = 1
        while index < len(data):
            device_length = data[index]
            device_list.append({"id": id_counter,
                                "command_size": data[index+1],
                                "data_size": data[index+2],
                                "device_attributes":data[index+3:index+device_length+1]})
            id_counter += 1
            index += device_length + 1
        return {"module_id":int(module_id) , "devices":device_list}

    def structure_discovery(self, timeout=1):
        self.socket.sendto(b'\x00\x00', ("10.42.0.255", 9999))
        module_list = []
        start_time = time.time()
        while(time.time()-start_time < timeout):
            try:
                data, server = self.socket.recvfrom(1024)
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

    def write_command(self, target_module, target_device, command):
        byte0 = (1<<7) + (target_device&(0b01111111))
        print(byte0)
        payload = bytearray([byte0] + command)
        self.socket.sendto(payload, ("10.42.0.{0}".format(target_module), 9999))

    def read_data(self, target_module, target_device):
        byte0 = target_device&(0b01111111)
        self.socket.sendto(bytearray([byte0]), ("10.42.0.{0}".format(target_module), 9999))
        try:
            data, server = self.socket.recvfrom(1024)
            return data
        except:
            return None

if __name__ == "__main__":
    robot = ModRob()
    module_list = robot.structure_discovery()
    for module in module_list:
        print(module)