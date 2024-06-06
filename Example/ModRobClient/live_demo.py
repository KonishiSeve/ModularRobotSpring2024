import tkinter as tk
import numpy as np
import json
import threading
import time
import socket

from Visualizer import ModuleGraphics
from ModRobClient import ModRob

#Running this file displays a representation of the demo robot with the pressure sensor states (module is green if sensor if pressed, orange otherwise). The module ID is indicated as well as the port0 location (with an arrow)
focus_module_id = 130 #The visualization will draw the robot starting from this module, nothing is drawn if it is not detected
focus_module_position = [500,500] #where the port0 of the focus module will be drawn on screen
focus_module_orientation = [0,-1] #port orientation to mate the focus module port0 with when drawn on screen

#Convert the different bytearrays of attributes to numbers
def convert_port_positions(ports_attributes):
    return_list = []
    for attribute in ports_attributes:
        #convert from int16 variables
        return_list.append([int.from_bytes(attribute[:2], "big", signed=True),
                            int.from_bytes(attribute[2:4], "big", signed=True)])
    return return_list

def convert_port_orientation(ports_attributes):
    return_list = []
    for attribute in ports_attributes:
        #convert from in8 variables
        return_list.append([int.from_bytes(attribute[4:5], "big", signed=True),
                            int.from_bytes(attribute[5:], "big", signed=True)])
    return return_list

def convert_module_outline(module_attributes):
    #convert from int16 variables
    return [[int.from_bytes(module_attributes[(i*4):(i*4+2)],"big",signed=True),
             int.from_bytes(module_attributes[(i*4+2):(i*4+4)],"big",signed=True)] for i in range(int(len(module_attributes)/4))]


#thread that runs the structure discovery and sensor reading on the robot and draws it to the Tkinter canvas
def interface(canvas):
    robot = ModRob(ip=socket.gethostbyname(socket.gethostname()), module_udp_port=9999,timeout_milliseconds=500)
    while(1):
        sensor_values = {}
        module_list = robot.structure_discovery()
        if module_list == None:
            print("Lost connection to robot...")
            continue
        root_module = None
        for module_i in module_list:
            sensor_raw = robot.read_data(module_i["module_id"], 1) #read the sensor pressure sensor of all the modules (pressure sensor is device number 1)
            if(sensor_raw != None):
                sensor_values[module_i["module_id"]] = int.from_bytes(sensor_raw, "little")
            else:
                sensor_values[module_i["module_id"]] = 0
            if module_i["module_id"] == focus_module_id:
                root_module = module_i
        if root_module == None:
            print("Lost connection to main module...")
            continue
        else:
            print(sensor_values)
        visited_id = []
        # next call -> [module_data, port_number, port_position_of_caller, port_orientation of caller]
        next_calls = [[root_module, 0, np.array(focus_module_position), np.array(focus_module_orientation)]]
        visited_id.append(root_module["module_id"])
        canvas.delete("all")
        #draw all the modules starting from the "focus" module and then the ones that are directly connected to it
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