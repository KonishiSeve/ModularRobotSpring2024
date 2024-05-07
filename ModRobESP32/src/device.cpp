#include "device.h"

Device::Device(uint8_t commands_size,
                void (*update_commands)(uint8_t*),
                uint8_t data_size,
                void (*update_data)(uint8_t*),
                uint8_t *attributes,
                uint8_t attributes_size)
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

uint8_t Device::get_data(uint8_t *data_buffer) {
    Device::update_data(data_buffer);
    return Device::data_size;
}

bool Device::set_command(uint8_t *command, uint8_t command_size) {
    Serial.println("setting command");
    if(command_size == Device::commands_size) {
        Device::update_commands(command);
    }
    Serial.println("done setting command");
    return 1;
}