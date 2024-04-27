#include "modrob.h"

bool Module::setup(uint8_t module_id, uint8_t port_location_bytes_nb) {
    /*
    Module::module_id = module_id;

    Module::neighbours_id = (uint8_t*)pvPortMalloc(0xFF);
    Module::port_rx_pins = (uint8_t*)pvPortMalloc(0xFF);
    Module::neighbours_id_size = 0;

    Module::port_locations = (uint8_t*)pvPortMalloc(0xFF*port_location_bytes_nb);
    Module::port_locations_size = 0;

    Module::module_add_size = 0;

    Module::devices = (Device*)pvPortMalloc(sizeof(Device)*128);
    Module::devices_size = 0;
    */
   return 1;
}

bool Module::add_port(uint8_t rx_pin, uint8_t *port_location) {
    /*
    pinMode(rx_pin, INPUT);
    uint8_t index = Module::neighbours_id_size;
    Module::port_rx_pins[index] = rx_pin;
    memcpy(Module::port_locations, port_location, Module::port_locations_size);
    Module::neighbours_id_size++;
    */
    return 1;
}

/*
bool Module::add_device(Device device) {
    Module::devices[Module::devices_size++] = device;
    return 1;
}
*/

bool Module::set_module_add(uint8_t *additional_bytes, uint16_t additional_bytes_nb) {
    /*
    Module::module_add_size = additional_bytes_nb;
    Module::module_add = additional_bytes;
    */
    return 1;
}

uint16_t Module::udp_write_command(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer) {
    Serial.printf("Write to device %d\n\r", packet[0]&(~(1<<7)));
    udp_tx_buffer[0] = 0x23;
    /*
    uint8_t target_device = packet[0]&(~(1<<7));
    Module::devices[target_device].set_command(packet+1,packet_size-1);
    */
   return 0;
}

uint16_t Module::udp_read_data(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer) {
    Serial.printf("Read from device %d\n\r", packet[0]&(~(1<<7)));
    udp_tx_buffer[0] = 0x78;
    return 0;
}

uint16_t Module::udp_module_disc(uint8_t *udp_tx_buffer) {
    Serial.println("Module discovery");
    udp_tx_buffer[0] = 0x45;
    /*
    //Send number of devices
    Module::udp_buffer_tx[0] = Module::devices_size;
    Module::udp.write(Module::udp_buffer_tx, 1);
    for(int i=0;i<Module::devices_size;i++) {
        uint8_t len = Module::devices[i].udp_fill(Module::udp_buffer_tx);
        Module::udp.write(Module::udp_buffer_tx, len);
    }
    */
   return 1;
}

uint16_t Module::udp_struct_disc(uint8_t *udp_tx_buffer) {
    Serial.println("Structure discovery");
    
    /*
    //Send neighbour ids
    Module::udp_buffer_tx[0] = Module::neighbours_id_size;
    Module::udp.write(Module::udp_buffer_tx, 1);
    Module::udp.write(Module::neighbours_id, Module::udp_buffer_tx[0]);

    //Send port locations
    Module::udp_buffer_tx[0] = Module::port_locations_size * Module::neighbours_id_size;
    Module::udp.write(Module::udp_buffer_tx, 1);
    if(Module::port_locations_size > 0) {
        Module::udp.write(Module::port_locations, Module::udp_buffer_tx[0]);
    }

    //Send additionnal data
    Module::udp_buffer_tx[0] = Module::module_add_size;
    Module::udp.write(Module::udp_buffer_tx, 1);
    if(Module::module_add_size > 0) {
        Module::udp.write(Module::module_add, Module::udp_buffer_tx[0]);
    }
    */
   return 1;
}

uint16_t Module::process_udp(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer) {
    if(packet[0]&(1<<7)) {
        //write command
        return Module::udp_write_command(packet, packet_size, udp_tx_buffer);
    }
    else {
        //read command
        if(packet[0]) {
            //read from device
            return Module::udp_read_data(packet, packet_size, udp_tx_buffer);
        }
        else {
            //read from metadata
            if(packet[1]) {
                //Module discovery
                return Module::udp_module_disc(udp_tx_buffer);
            }
            else {
                //Structure discovery
                return Module::udp_struct_disc(udp_tx_buffer);
            }
        }
    }
    return 0;
}