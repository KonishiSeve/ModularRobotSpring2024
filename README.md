# Modular Robot Framework Semester Project Spring 2024

## File structure

.
├── ModRobClient
│   └── ModRobClient.py:   Python libary to allow the controller to interact with the robot
├── ModRobESP32:           ESP32 Arduino PlatformIO project template, firmware side of the framework
└── Examples:              application of the framework for a demonstration robot
    ├── ModRobClient
    │   ├── live_demo.py:  diplays a real-time representation of the demonstration robot by using the ModRobClient.py library
    │   └── Visualizer.py: helps to display the modules on screen for live_demo.py
    └── ModRobESP32:       Firmware side of the framework applied to the demonstration robot, only main.ccp was modified


## Firmware Side
The firmware side is a PlatformIO ESP32 Arduino framework template project.
### main.cpp
This is the only file that should be modified when using the framework. In this file, a module object is created and needs to be configured in the setup loop by calling its different methods in the "void setup()" function. If some setup needs to be ran for the sensors or actuators, it should also be done here.

These methods should only be called once
- setup(): used to set the module unique ID (It is recommended to use the last number of the IP address), to set the size of the port attributes and the maximum number of devices that can be added to the module (a higher number will reserve more RAM)
- set_port_tx(): used to set which pin should be used as TX pin for all the ports
- set_module_attributes(): used to set the module attributes as a list of uint8

These methods can be called multiple times
- add_port_rx(): used to add a port to the module by specifying the pin used as RX and the port attributes as a list of uint8
- add_device(): used to add a device object to the module

### modrob.cpp and modrob.h
These files define the ModRob class. Most methods can be called by the user to configure the module.

### device.cpp and device.h
These files define the Device class. The user should create an instance for each devices that is on the module and pass it to the "module" object (instance of the ModRob class in main.cpp) with the add_device() method.

### multi_uart.cpp and multi_uart.h
These files contain a software implementation of UART that can listen to multiple pins at the same time. It is used for structure discovery by the ModRob class

## Client Side
The controller can interact with the robot by using the ModRobClient.py library. A robot object needs to be instantiated and 4 different methods can be called:

### structure_discovery()
Broadcasts a structure discovery packet to all the modules and returns a list of dictionaries. Returns one dictionary per module with the following keys:
- "module_id": number; id of the module
- "ports_states" list of numbers; length is the number of ports (number of time add_port_rx() was called in the firmware). the number is the ID of the module connected to the port with this index (the first number of the list is the ID of the module connected to port 0). The ID is zero if no module is connected.
- "ports_attributes": list of bytearrays; list of the port attributes (assigned when calling add_port_rx() in the firmware)
- "module_attributes": bytearray; module attributes (assigned with set_module_attributes() in the firmware)

### module_discovery(module_id)
Sends a module discovery packet to the target module and returns a dictionary with a "module_id" key containing the target module ID and a "devices" key which contains a list of dictionaries, the length of this list is equal to the number of devices on the target module (number of times add_device() was called in the firmware). Each device dictionary contains the following keys:
- "id": number; the device ID (starts at 1)
- "command_size" number; number of command bytes the device takes ("commands_size" argument in the Device class constructor in the firmware)
- "data_size" number; number of data bytes the device returns ("data_size" argument in the Device class constructor in the firmware)
- "device_attributes": bytearray; device attributes ("attributes" argument in the Device class constructor in the firmware)

### read_data(target_module, target_device)
Sends a read data packet to the target_device of a target_module. This calls the function whose handle was given to the Device object constructor as the "update_commands" argument in the firmware and does not return anything

### write_command(target_module, target_device, command)
Sends a write data packet to the target_device of a target_module. This calls the function whose handle was given to the Device object constructor as the "update_data" argument in the firmware and the return of this function is returned here as a bytearray


## UDP packets
This section describes the packets that are sent by the client for the 4 different operations on the robot and the response packets that are sent back by the modules.

### Structure discovery
request packet:
<table class="tg"><thead>
  <tr>
    <th class="tg-0lax">Byte 0</th>
    <th class="tg-0lax">Byte 1</th>
  </tr></thead>
<tbody>
  <tr>
    <td class="tg-0lax">0x00</td>
    <td class="tg-0lax">0x00</td>
  </tr>
</tbody>
</table>

response packet:
<table class="tg"><thead>
  <tr>
    <th class="tg-0lax">Byte</th>
    <th class="tg-0lax" colspan="3">N Bytes</th>
    <th class="tg-0lax">Byte</th>
    <th class="tg-0lax" colspan="3">P*N Bytes</th>
    <th class="tg-0lax">Byte</th>
    <th class="tg-0lax">M Bytes</th>
  </tr></thead>
<tbody>
  <tr>
    <td class="tg-0lax">number of ports (N)</td>
    <td class="tg-0lax">ID of port0 neighbor</td>
    <td class="tg-0lax">...</td>
    <td class="tg-0lax">ID of portN neighbor</td>
    <td class="tg-0lax">number of bytes per port attributes (P)</td>
    <td class="tg-0lax">port0 attributes</td>
    <td class="tg-0lax">...</td>
    <td class="tg-0lax">portN attributes</td>
    <td class="tg-0lax">number of module attribute bytes (M)</td>
    <td class="tg-0lax">module attributes</td>
  </tr>
</tbody>
</table>

### Module discovery

request packet:
<table class="tg"><thead>
  <tr>
    <th class="tg-0lax">Byte 0</th>
    <th class="tg-0lax">Byte 1</th>
  </tr></thead>
<tbody>
  <tr>
    <td class="tg-0lax">0x00</td>
    <td class="tg-0lax">0x01</td>
  </tr>
</tbody>
</table>

response packet:
<table class="tg"><thead>
  <tr>
    <th class="tg-baqh" colspan="6">D0 Bytes</th>
    <th class="tg-0lax"></th>
    <th class="tg-baqh" colspan="6">DN Bytes</th>
  </tr></thead>
<tbody>
  <tr>
    <td class="tg-0lax">Device0 byte length (D0)</td>
    <td class="tg-0lax">Command register size</td>
    <td class="tg-0lax">Data register size</td>
    <td class="tg-0lax">Device attribute byte 0</td>
    <td class="tg-0lax">...</td>
    <td class="tg-0lax">Device attribute byte D0-3</td>
    <td class="tg-0lax">...</td>
    <td class="tg-0lax">DeviceN byte length (DN)</td>
    <td class="tg-0lax">Command register size</td>
    <td class="tg-0lax">Data register size</td>
    <td class="tg-0lax">Device attribute byte 0</td>
    <td class="tg-0lax">...</td>
    <td class="tg-0lax">Device attribute byte DN-3</td>
  </tr>
</tbody>
</table>

### Read
request packet:
<table class="tg"><thead>
  <tr>
    <th class="tg-baqh" colspan="2">Byte 0</th>
  </tr></thead>
<tbody>
  <tr>
    <td class="tg-baqh">bit 7</td>
    <td class="tg-baqh">bit 6:0</td>
  </tr>
  <tr>
    <td class="tg-baqh">0</td>
    <td class="tg-baqh">device ID<br>(1 to 127)</td>
  </tr>
</tbody>
</table>

response packet: The raw bytes only, the length should be known from the module discovery

### Write
request packet:
<table class="tg"><thead>
  <tr>
    <th class="tg-c3ow">Byte 0</th>
    <th class="tg-c3ow">Byte 1</th>
    <th class="tg-c3ow">Byte 2</th>
    <th class="tg-c3ow"></th>
    <th class="tg-c3ow">Byte N</th>
  </tr></thead>
<tbody>
  <tr>
    <td class="tg-c3ow">bit 7</td>
    <td class="tg-c3ow">bit 6:0</td>
    <td class="tg-c3ow" rowspan="2">First command byte</td>
    <td class="tg-c3ow" rowspan="2">...</td>
    <td class="tg-c3ow" rowspan="2">Last command byte</td>
  </tr>
  <tr>
    <td class="tg-c3ow">1</td>
    <td class="tg-c3ow">device ID<br>(1 to 127)</td>
  </tr>
</tbody>
</table>

No reponse packet