import tkinter as tk
import numpy as np
import json
import threading
import time

from Visualizer import ModuleGraphics
from ModRobClient import ModRob

#wireshark filter: udp && data && udp.port==9999

def sd_draw(module_list, canvas, focus_module_id):
    root_module = None
    smallest_module_id = 255
    print(*module_list, sep="\n")
    #look for the module to focus on
    for module in module_list:
        if(module["module_id"] == focus_module_id):
            root_module = module
            break
        elif(module["module_id"] < smallest_module_id):
            smallest_module_id = module["module_id"]
            root_module = module

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
def shell(canvas):
    robot = ModRob()
    focus_module_id = None
    while(1):
        #sd -> structure discovery
        #md %module_id% -> module discovery
        #w %module_id% %n,n,n,n% -> write command
        #r %module_id%

        #f %module_id% visualization centered around this module

        text = input("[ModRobVis shell]#: ")
        command = text.split(" ")

        #Structure discovery and refresh the display
        if(command[0] == "sd"):
            module_list = robot.structure_discovery()
            if len(module_list) > 0:
                print(*module_list, sep="\n")
                #sd_draw(module_list, canvas, focus_module_id)
            else:
                print("[ERROR] No module found")

        elif(command[0] == "md"):
            if(len(command) != 2):
                print("[ERROR] need 1 argument for md")
                continue
            device_list = robot.module_discovery(int(command[1]))
            if len(device_list):
                for dev in device_list['devices']:
                    print(dev)
            else:
                print("[ERROR] Module could not be reached")
        
        elif(command[0] == "w"):
            if(len(command) <= 3):
                print("[ERROR] need at least 3 argument for writing (target module, target device, command)")
                continue
            robot.write_command(int(command[1]), int(command[2]), [int(i) for i in command[3:]])

        elif(command[0] == "r"):
            if(len(command) <= 2):
                print("[ERROR] need 2 argument for reading (target module, target device)")
                continue
            print(robot.read_data(int(command[1]), int(command[2])))



                    



root = tk.Tk()
canvas = tk.Canvas(root, height=720, width=480)
canvas.pack()

window_thread = threading.Thread(target=shell, args=(canvas,) ,daemon=True)
window_thread.start()

root.mainloop()