# Modular Robot Framework Semester Project Spring 2024

## File structure

- ModRobESP32 ("server"/firmware side of the framework): contains a ESP32 PlatformIO project in the Arduino framework that needs to be filled in and flashed to a module
- ModRobClient (client side of the framework): Python library that the controller can call to interact with the robot
- Example
    - ModRobESP32: a modified firmware that was used to flash 3 types of modules for the demonstration robot
    - ModRobClient
        - Visualizer.py: library for this specific example that helps displaying a module on screen
        - live_demo.py: Can be run to display a visualization of the demonstration robot in real time


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


## UDP packets
This section describes the packets that are sent by the client for the 4 different operations on the robot and the response packets that are sent back by the modules.

### Structure discovery
request packet:
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-0lax{text-align:left;vertical-align:top}
</style>
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
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-0lax{text-align:left;vertical-align:top}
</style>
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
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-0lax{text-align:left;vertical-align:top}
</style>
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
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-baqh{text-align:center;vertical-align:top}
.tg .tg-0lax{text-align:left;vertical-align:top}
</style>
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
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-baqh{text-align:center;vertical-align:top}
</style>
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
<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  overflow:hidden;padding:10px 5px;word-break:normal;}
.tg th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;
  font-weight:normal;overflow:hidden;padding:10px 5px;word-break:normal;}
.tg .tg-baqh{text-align:center;vertical-align:top}
</style>
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
    <td class="tg-baqh">1</td>
    <td class="tg-baqh">device ID<br>(1 to 127)</td>
  </tr>
</tbody>
</table>

No reponse packet