import tkinter as tk
import numpy as np
import json
import threading
import time
import socket

from Visualizer import ModuleGraphics
from ModRobClient import ModRob

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


#wireshark filter: udp && data && udp.port==9999
def interface(canvas):
    robot = ModRob(ip=socket.gethostbyname(socket.gethostname()), module_udp_port=9999,timeout_milliseconds=500)
    #module_list = robot.structure_discovery()
    #root_module = module_list[0]
    while(1):
        sensor_values = {}
        module_list = robot.structure_discovery()
        if module_list == None:
            print("Lost connection to robot...")
            continue
        root_module = None
        for module_i in module_list:
            sensor_raw = robot.read_data(module_i["module_id"], 1)
            if(sensor_raw != None):
                sensor_values[module_i["module_id"]] = int.from_bytes(sensor_raw, "little")
            else:
                sensor_values[module_i["module_id"]] = 0
            if module_i["module_id"] == 130:
                root_module = module_i
        if root_module == None:
            print("Lost connection to main module...")
            continue
        else:
            print(sensor_values)
        visited_id = []
        # next call -> [module_data, port_number, port_position_of_caller, port_orientation of caller]
        next_calls = [[root_module, 0, np.array([500,500]), np.array([0,-1])]]
        visited_id.append(root_module["module_id"])
        canvas.delete("all")
        while len(next_calls):
            new_next_calls = []
            for call in next_calls:
                current_module = call[0]
                graph_data = {"id":current_module["module_id"],
                                "ports_positions": convert_port_positions(current_module["ports_attributes"]),
                                "ports_orientations": convert_port_orientation(current_module["ports_attributes"]),
                                "outlines": [convert_module_outline(current_module["module_attributes"])]
                                }
                grap_module = ModuleGraphics(graph_data, call[1], call[2], call[3], 0.8)
                grap_module.draw(canvas, "green" if sensor_values[current_module["module_id"]] else "orange")
                for next_module in module_list:
                    if (next_module["module_id"] in current_module["ports_states"]) and not (next_module["module_id"] in visited_id):
                        visited_id.append(next_module["module_id"])
                        current_module_port = current_module["ports_states"].index(next_module["module_id"])
                        new_next_calls.append([next_module,
                                                next_module["ports_states"].index(current_module["module_id"]),
                                                grap_module.ports_positions[current_module_port],
                                                grap_module.ports_orientations[current_module_port]
                                                ])
            next_calls = new_next_calls

if __name__ == "__main__":
    root = tk.Tk()
    canvas = tk.Canvas(root, height=1080, width=1080)
    canvas.pack()

    window_thread = threading.Thread(target=interface, args=(canvas,) ,daemon=True)
    window_thread.start()

    root.mainloop()