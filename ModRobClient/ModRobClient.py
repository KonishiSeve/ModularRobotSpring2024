import socket

#create an object of this class and use the methods to interact with the robot
class ModRob:
    def __init__(self, ip, module_udp_port, timeout_milliseconds):
        #save the ip address except the last number
        self.ip_base = "{0}.{1}.{2}".format(ip.split(".")[0], ip.split(".")[1], ip.split(".")[2])
        #save the udp port number the ESP32 are listening to (9999 by default)
        self.module_udp_port = module_udp_port

        #configuring the socket
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.settimeout(timeout_milliseconds/1000.0)
        #enable broadcasting
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    

    #Calls a module discovery on the targeted module
    #returns a a list of dictionaries (one for each device) with format:
    #   {"id": id of device (start from 1) ,
    #    "command_size": number of command bytes for device,
    #    "data_size": number of data bytes for device,
    #    "device_attributes": a bytearray of the attributes defined in the firmware for this device}

    def module_discovery(self, module_id):
        #send packet that corresponds to module discovery (which is [0x00, 0x01])
        self.socket.sendto(b'\x00\x01', ("{0}.{1}".format(self.ip_base, module_id), self.module_udp_port))
        #receive the response packet
        try:
            data, server = self.socket.recvfrom(1024)

        except:
            #if timed out
            return None
        return self.parse_model_discovery(server[0].split(".")[-1], data)
    
    #should not be called by user, parses the raw UDP packet
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

    #Calls a structure discovery on the whole robot
    #returns a a list of dictionaries (one for each module) with format:
    #   {"module_id": id of module (last number of IP) ,
    #    "ports_states": list of module ids (length is equal to the number of ports, id is zero if nothing is connected to this port),
    #    "ports_attributes": list of bytearrays (one bytearray for each port),
    #    "module_attributes": a bytearray of the module attributes defined in the firmware}

    def structure_discovery(self, timeout=1):
        #send packet that corresponds to structure discovery ([0x00,0x00]) via the broadcast address (255)
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
    
    #should not be called by user, parses the raw UDP packet
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

    #write a bytearray to a target device in a target module, it will be passed to the command function handle defined in the firmware
    def write_command(self, target_module, target_device, command):
        byte0 = (1<<7) + (target_device&(0b01111111)) #put the first bit to 1 to indicate a command write packet
        payload = bytearray([byte0]) + command
        self.socket.sendto(payload, ("{0}.{1}".format(self.ip_base, target_module), self.module_udp_port))

    #read a bytearray from a target device in a target module, the data function handle defined in the firmware will be ran to find this bytearray
    def read_data(self, target_module, target_device):
        byte0 = target_device&(0b01111111) #put the first bit to 0 to indicate a data read packet
        self.socket.sendto(bytearray([byte0]), ("{0}.{1}".format(self.ip_base, target_module), self.module_udp_port))
        try:
            data, server = self.socket.recvfrom(1024)
            return data
        except:
            return None