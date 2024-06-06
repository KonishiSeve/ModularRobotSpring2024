#ifndef MODROB_H
#define MODROB_H

#include <Arduino.h>
#include "multi_uart.h"
#include "device.h"

class ModRob {
    private:
        //Module attributes
        uint8_t module_id;
        uint8_t *module_attributes;
        uint8_t module_attributes_size;

        //Port attributes
        uint8_t *neighbours_id;
        uint8_t neighbours_id_size;

        uint8_t *ports_attributes;
        uint8_t ports_attributes_size;
        uint8_t bytes_per_port_attribute;

        //MulitUART driver
        MultiUART multi_uart;

        //Device attributes
        Device *devices;
        uint8_t devices_size;
        uint8_t devices_size_max;

        //functions mapped to udp commands, called in process_udp method
        uint16_t udp_struct_disc(uint8_t *udp_tx_buffer);
        uint16_t udp_module_disc(uint8_t *udp_tx_buffer);
        uint16_t udp_write_command(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer);
        uint16_t udp_read_data(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer);

    public:
    //Should be called by the user to configure the module
        bool setup(uint8_t module_id, uint8_t bytes_per_port_attribute, uint8_t max_devices_number);

        //add a docking port with corresponding pin number and attributes
        bool add_port_rx(uint8_t rx_pin, uint8_t *port_attributes);
        //set the TX pin to use for all the ports
        bool set_port_tx(uint8_t tx_pin);

        //add a device to the module
        bool add_device(Device device);
        //define the module attributes
        bool set_module_attributes(uint8_t *module_attributes, uint16_t module_attributes_size);

    //Called in the loop() of the main to process a UDP packet and act accordingly
        uint16_t process_udp(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer);
};

#endif