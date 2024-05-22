import tkinter as tk
import numpy as np
import json
import threading
import time
import socket

from Visualizer import ModuleGraphics
from ModRobClient import ModRob

def to_8bits_signed(value):
    if value > 127:
        return value -256
    return value

def convert_port_attributes(ports_attributes):
    return_list = []
    for attribute in ports_attributes:
        return_list.append([int.from_bytes(attribute[:2], "little", signed=True),
                            int.from_bytes(attribute[2:4], "little", signed=True)])
    return return_list

def parse_port_orientation(ports_attributes):
    return_list = []
    for attribute in ports_attributes:
        return_list.append([int.from_bytes(attribute[4:5], "little", signed=True),
                            int.from_bytes(attribute[5:], "little", signed=True)])
    return return_list

def parse_module_outline(module_attributes):
    return [[int.from_bytes(module_attributes[(i*4):(i*4+2)],"little",signed=True),
             int.from_bytes(module_attributes[(i*4+2):(i*4+4)],"little",signed=True)] for i in range(len(module_attributes)/4)]

#wireshark filter: udp && data && udp.port==9999
def interface(canvas):
    robot = ModRob(ip=socket.gethostbyname(socket.gethostname()), module_udp_port=9999,timeout_milliseconds=500)
    module_list = robot.structure_discovery()
    root_module = module_list[0]
    while(1):
        visited_id = []
        # next call -> [module_data, port_number, port_position_of_caller, port_orientation of caller]
        next_calls = [[root_module, 0, np.array([100,100]), np.array([0,-1])]]
        visited_id.append(root_module["module_id"])
        canvas.delete("all")
        while len(next_calls):
            new_next_calls = []
            for call in next_calls:
                current_module = call[0]
                graph_data = {"id":current_module["module_id"],
                                "ports_positions": [port[:2] for port in current_module["ports_attributes"]],
                                "ports_orientations": [port[2:] for port in current_module["ports_attributes"]],
                                "outlines": [[ current_module["module_attributes"][i*2:i*2+2] for i in range(int(len(current_module["module_attributes"])/2)) ]]
                                }
                grap_module = ModuleGraphics(graph_data, call[1], call[2], call[3], 5)
                grap_module.draw(canvas, "green")
                for next_module in module_list:
                    if (next_module["module_id"] in current_module["neighbours"]) and not (next_module["module_id"] in visited_id):
                        visited_id.append(next_module["module_id"])
                        current_module_port = current_module["neighbours"].index(next_module["module_id"])
                        new_next_calls.append([next_module,
                                                next_module["neighbours"].index(current_module["module_id"]),
                                                grap_module.ports_positions[current_module_port],
                                                grap_module.ports_orientations[current_module_port]
                                                ])
            next_calls = new_next_calls

if __name__ == "__main__":
    root = tk.Tk()
    canvas = tk.Canvas(root, height=720, width=480)
    canvas.pack()

    window_thread = threading.Thread(target=interface, args=(canvas,) ,daemon=True)
    window_thread.start()

    root.mainloop()