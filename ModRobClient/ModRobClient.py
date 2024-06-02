import time
import socket

class ModRob:
    def __init__(self, ip, module_udp_port, timeout_milliseconds):
        #save the ip address except the last number
        self.ip_base = "{0}.{1}.{2}".format(ip.split(".")[0], ip.split(".")[1], ip.split(".")[2])
        #save the udp port number the ESP32 are listening to
        self.module_udp_port = module_udp_port

        #configurint the socket
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.settimeout(timeout_milliseconds/1000.0)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    
    def module_discovery(self, module_id):
        #send packet that corresponds to module discovery
        self.socket.sendto(b'\x00\x01', ("{0}.{1}".format(self.ip_base, module_id), self.module_udp_port))
        #receive the response packet
        try:
            data, server = self.socket.recvfrom(1024)

        except:
            #if timed out
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
        #send packet that corresponds to structure discovery via the broadcast address (255)
        self.socket.sendto(b'\x00\x00', ("{0}.{1}".format(self.ip_base, 255), self.module_udp_port))
        module_list = []
        #receive the response packets
        while(1):
            try:
                data, server = self.socket.recvfrom(1024)
                module_list.append(self.parse_structure_discovery(server[0].split(".")[-1], data))
            except:
                return module_list
        return module_list
    
    def parse_structure_discovery(self, module_id, data):
        #extract the ports number and the neighbors
        data_index = 0
        ports_number = data[data_index]
        data_index += 1
        neighbours = [data[(i+data_index)] for i in range(ports_number)]
        data_index += ports_number

        #extract the ports attributes
        ports_attributes_size = data[data_index]
        attribute_bytes_per_port = int(ports_attributes_size/ports_number)
        data_index += 1
        ports_attributes = []
        for _ in range(ports_number):
            ports_attributes.append(data[data_index:(data_index+attribute_bytes_per_port)])
            data_index += attribute_bytes_per_port
        
        #extract the module attributes
        module_attributes_size = data[data_index]
        data_index += 1
        module_attributes = data[data_index:(data_index+module_attributes_size)]

        return {"module_id": int(module_id), "ports_states": neighbours, "ports_attributes": ports_attributes, "module_attributes": module_attributes}

    def write_command(self, target_module, target_device, command):
        byte0 = (1<<7) + (target_device&(0b01111111))
        payload = bytearray([byte0]) + command
        self.socket.sendto(payload, ("{0}.{1}".format(self.ip_base, target_module), self.module_udp_port))

    def read_data(self, target_module, target_device):
        byte0 = target_device&(0b01111111)
        self.socket.sendto(bytearray([byte0]), ("{0}.{1}".format(self.ip_base, target_module), self.module_udp_port))
        try:
            data, server = self.socket.recvfrom(1024)
            return data
        except:
            return None

def convert_port_positions(ports_attributes):
    return_list = []
    for attribute in ports_attributes:
        return_list.append([int.from_bytes(attribute[:2], "big", signed=True),
                            int.from_bytes(attribute[2:4], "big", signed=True)])
    return return_list

def convert_port_orientation(ports_attributes):
    return_list = []
    for attribute in ports_attributes:
        return_list.append([int.from_bytes(attribute[4:5], "big", signed=True),
                            int.from_bytes(attribute[5:], "big", signed=True)])
    return return_list

def convert_module_outline(module_attributes):
    return [[int.from_bytes(module_attributes[(i*4):(i*4+2)],"big",signed=True),
             int.from_bytes(module_attributes[(i*4+2):(i*4+4)],"big",signed=True)] for i in range(int(len(module_attributes)/4))]

if __name__ == "__main__":
    robot = ModRob(ip=socket.gethostbyname(socket.gethostname()), module_udp_port=9999,timeout_milliseconds=500)
    module_list = robot.structure_discovery()
    #device_list = robot.module_discovery(module_list[0]["module_id"])
    print(*module_list, sep="\n")
    print(convert_port_positions(module_list[0]['ports_attributes']))
    print(convert_port_orientation(module_list[0]['ports_attributes']))
    print(convert_module_outline(module_list[0]['module_attributes']))
    #print("Modules:")
    #print(*module_list, sep="\n")
    #print("Devices of first module")
    #print(*(device_list['devices']), sep="\n")