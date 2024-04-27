#ifndef MODROB_H
#define MODROB_H

#include <Arduino.h>

/*
#define PORT_NB             1
uint8_t neighbours_ip[PORT_NB];
#define PORT_LOC_BYTES_NB   0
uint8_t port_locations[PORT_NB*PORT_LOC_BYTES_NB];
#define MODULE_ADD_NB       1
uint8_t module_add[MODULE_ADD_NB];
*/

class Module {
    private:
        //Module properties
        uint8_t module_id;
        uint8_t *module_add;
        uint8_t module_add_size;

        //Port properties
        uint8_t *neighbours_id;
        uint8_t *port_rx_pins;
        uint8_t neighbours_id_size;

        uint8_t *port_locations;
        uint8_t port_locations_size;


        //Device properties
        /*
        Device *devices;
        uint8_t devices_size;
        */

        //functions mapped to udp commands
        uint16_t udp_struct_disc(uint8_t *udp_tx_buffer);
        uint16_t udp_module_disc(uint8_t *udp_tx_buffer);
        uint16_t udp_write_command(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer);
        uint16_t udp_read_data(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer);

    public:
    //internal properties
        bool setup(uint8_t module_idk, uint8_t port_location_bytes_nb);
        bool add_port(uint8_t rx_pin, uint8_t *port_location);
        //bool add_device(Device device);
        bool set_module_add(uint8_t *additional_bytes, uint16_t additional_bytes_nb);

    //online
        uint16_t process_udp(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer);
};

#endif