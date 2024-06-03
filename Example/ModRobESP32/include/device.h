#ifndef DEVICE_H
#define DEVICE_H
#include <Arduino.h>

class Device {
    public:
        //attributes
        uint8_t *attributes;
        uint8_t attributes_size;

        //commands related (writable commands)
        uint8_t *commands;
        uint8_t commands_size;
        void (*update_commands)(uint8_t*);

        //data related (readable data)
        uint8_t *data;
        uint8_t data_size;
        void (*update_data)(uint8_t*); //takes *data as input and needs to add the data into it
        
        Device(uint8_t commands_size,
                void (*update_commands)(uint8_t*),
                uint8_t data_size,
                void (*update_data)(uint8_t*),
                uint8_t *attributes,
                uint8_t attributes_size
                );

        uint8_t get_data(uint8_t *data_buffer);
        bool set_command(uint8_t *command, uint8_t command_size);
};

#endif