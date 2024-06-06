from tkinter import *
from tkinter import messagebox
import tkinter as tk
import numpy as np

#class that makes it easier to draw modules for the demonstration robot

class ModuleGraphics:
    #Transform the local attributes of the module to match the global frame of reference
    def __init__(self, module_data, port_nb, mating_port_positions, mating_port_orientation, graphic_scaling):
        self.id = module_data["id"]
        self.ports_positions = module_data["ports_positions"]
        self.ports_orientations = module_data["ports_orientations"]
        self.outlines = module_data["outlines"]

        port_position = np.array(self.ports_positions[port_nb])
        port_orientation = np.array(self.ports_orientations[port_nb])
        port_orientation = port_orientation/np.linalg.norm(port_orientation)

        new_position = mating_port_positions
        new_orientation = -mating_port_orientation

        displacement = new_position - port_position

        angle = np.arctan2(np.linalg.det([port_orientation, new_orientation]), np.dot(port_orientation, new_orientation))
        A = np.array([[np.cos(angle), -np.sin(angle)],[np.sin(angle), np.cos(angle)]])
        for i in range(len(self.ports_positions)):
            self.ports_positions[i] = (graphic_scaling*A) @ (np.array(self.ports_positions[i]) - port_position) + port_position + displacement

        for i in range(len(self.ports_orientations)):
            self.ports_orientations[i] = np.array(self.ports_orientations[i])
            self.ports_orientations[i] = A @ (self.ports_orientations[i]/np.linalg.norm(self.ports_orientations[i]))

        for i in range(len(self.outlines)):
            for j in range(len(self.outlines[i])):
                self.outlines[i][j] = (graphic_scaling*A) @ (np.array(self.outlines[i][j]) - port_position) + port_position + displacement

    def draw(self, canvas, color):
        module_center = [0,0]
        nb_points = 0
        for i in range(len(self.outlines)):
            points = []
            for j in range(len(self.outlines[i])):
                points += [self.outlines[i][j][0], self.outlines[i][j][1]]
                module_center[0] += self.outlines[i][j][0]
                module_center[1] += self.outlines[i][j][1]
                nb_points += 1
            canvas.create_polygon(points, fill=color, outline="black", width=1)
        canvas.create_line(self.ports_positions[0][0], self.ports_positions[0][1], (self.ports_positions[0][0]+self.ports_orientations[0][0]*30), (self.ports_positions[0][1]+self.ports_orientations[0][1]*30), arrow=tk.LAST)
        
        module_center[0] = int(module_center[0]/nb_points)
        module_center[1] = int(module_center[1]/nb_points)
        canvas.create_text(module_center[0], module_center[1], text=str(self.id), fill="white")
