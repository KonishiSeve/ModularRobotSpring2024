#ifndef DEVICE_H
#define DEVICE_H
#include <Arduino.h>


//The user needs to define an object of this class for each device of the module and pass it to the module object
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
        
        Device(uint8_t commands_size,                   //number of command bytes (can be zero)
                void (*update_commands)(uint8_t*),      //handle of a function that takes new command bytes as input and applies it to an actuator. Called when a "write" command is sent to this device by the python client
                uint8_t data_size,                      //number of data bytes (can be zero)
                void (*update_data)(uint8_t*),          //handle of a function that reads from a sensor and fills the input array which will be sent to the python client. Called when a "read" command is sent to this device by the python client
                uint8_t *attributes,                    //array of attributes for this device
                uint8_t attributes_size                 //number of attribute bytes for this device
                );

        //these functions should not be called by the user
        uint8_t get_data(uint8_t *data_buffer);
        bool set_command(uint8_t *command, uint8_t command_size);
};

#endif