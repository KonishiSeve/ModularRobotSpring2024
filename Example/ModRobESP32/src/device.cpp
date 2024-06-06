#include "device.h"

//The user needs to define an object of this class for each device of the module and pass it to the module object
        Device(uint8_t commands_size,                   //number of command bytes (can be zero)
                void (*update_commands)(uint8_t*),      //handle of a function that takes new command bytes as input and applies it to an actuator. Called when a "write" command is sent to this device by the python client
                uint8_t data_size,                      //number of data bytes (can be zero)
                void (*update_data)(uint8_t*),          //handle of a function that reads from a sensor and fills the input array which will be sent to the python client. Called when a "read" command is sent to this device by the python client
                uint8_t *attributes,                    //array of attributes for this device
                uint8_t attributes_size                 //number of attribute bytes for this device
                )
{
    Device::commands_size = commands_size;
    if(commands_size > 0) {
        Device::commands = (uint8_t*)pvPortMalloc(commands_size);
        Device::update_commands = update_commands;
    }

    Device::data_size = data_size;
    if(data_size > 0) {
        Device::data = (uint8_t*)pvPortMalloc(data_size);
        Device::update_data = update_data;
    }
    
    Device::attributes = attributes;
    Device::attributes_size = attributes_size;
}

//this functions should not be called by the user
uint8_t Device::get_data(uint8_t *data_buffer) {
    Device::update_data(data_buffer);
    return Device::data_size;
}
//this functions should not be called by the user
bool Device::set_command(uint8_t *command, uint8_t command_size) {
    if(command_size == Device::commands_size) {
        Device::update_commands(command);
    }
    return 1;
}