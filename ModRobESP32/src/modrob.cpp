#include "modrob.h"
#include "multi_uart.h"

bool ModRob::setup(uint8_t module_id, uint8_t port_location_bytes_nb) {
    ModRob::module_id = module_id;

    ModRob::neighbours_id = (uint8_t*)pvPortMalloc(0xFF);
    ModRob::port_rx_pins = (uint8_t*)pvPortMalloc(0xFF);
    ModRob::neighbours_id_size = 0;

    ModRob::port_locations = (uint8_t*)pvPortMalloc(0xFF*port_location_bytes_nb);
    ModRob::port_locations_size = 0;

    ModRob::module_add_size = 0;
    /*
    ModRob::devices = (Device*)pvPortMalloc(sizeof(Device)*128);
    ModRob::devices_size = 0;
    */
   return 1;
}

bool ModRob::add_port_rx(uint8_t rx_pin, uint8_t *port_location) {
    ModRob::multi_uart.add_receiver(rx_pin);
    /*
    pinMode(rx_pin, INPUT);
    uint8_t index = ModRob::neighbours_id_size;
    ModRob::port_rx_pins[index] = rx_pin;
    memcpy(ModRob::port_locations, port_location, ModRob::port_locations_size);
    ModRob::neighbours_id_size++;
    */
    return 1;
}

bool ModRob::set_port_tx(uint8_t tx_pin) {
    ModRob::multi_uart.set_transmitter(tx_pin);
    return 1;
}

/*
bool ModRob::add_device(Device device) {
    ModRob::devices[ModRob::devices_size++] = device;
    return 1;
}
*/

bool ModRob::set_module_add(uint8_t *additional_bytes, uint16_t additional_bytes_nb) {
    /*
    ModRob::module_add_size = additional_bytes_nb;
    ModRob::module_add = additional_bytes;
    */
    return 1;
}

uint16_t ModRob::udp_write_command(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer) {
    Serial.printf("Write to device %d\n\r", packet[0]&(~(1<<7)));
    udp_tx_buffer[0] = 0x23;
    /*
    uint8_t target_device = packet[0]&(~(1<<7));
    ModRob::devices[target_device].set_command(packet+1,packet_size-1);
    */
   return 0;
}

uint16_t ModRob::udp_read_data(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer) {
    Serial.printf("Read from device %d\n\r", packet[0]&(~(1<<7)));
    udp_tx_buffer[0] = 0x78;
    return 0;
}

uint16_t ModRob::udp_module_disc(uint8_t *udp_tx_buffer) {
    Serial.println("Module discovery");
    udp_tx_buffer[0] = 0x45;
    /*
    //Send number of devices
    ModRob::udp_buffer_tx[0] = ModRob::devices_size;
    ModRob::udp.write(ModRob::udp_buffer_tx, 1);
    for(int i=0;i<ModRob::devices_size;i++) {
        uint8_t len = ModRob::devices[i].udp_fill(ModRob::udp_buffer_tx);
        ModRob::udp.write(ModRob::udp_buffer_tx, len);
    }
    */
   return 1;
}

uint16_t ModRob::udp_struct_disc(uint8_t *udp_tx_buffer) {
    Serial.println("Structure discovery");
    ModRob::multi_uart.xfer(9600, 5000, ModRob::module_id, 5000);

    /*
    //Send neighbour ids
    ModRob::udp_buffer_tx[0] = ModRob::neighbours_id_size;
    ModRob::udp.write(ModRob::udp_buffer_tx, 1);
    ModRob::udp.write(ModRob::neighbours_id, ModRob::udp_buffer_tx[0]);

    //Send port locations
    ModRob::udp_buffer_tx[0] = ModRob::port_locations_size * ModRob::neighbours_id_size;
    ModRob::udp.write(ModRob::udp_buffer_tx, 1);
    if(ModRob::port_locations_size > 0) {
        ModRob::udp.write(ModRob::port_locations, ModRob::udp_buffer_tx[0]);
    }

    //Send additionnal data
    ModRob::udp_buffer_tx[0] = ModRob::module_add_size;
    ModRob::udp.write(ModRob::udp_buffer_tx, 1);
    if(ModRob::module_add_size > 0) {
        ModRob::udp.write(ModRob::module_add, ModRob::udp_buffer_tx[0]);
    }
    */
   return 1;
}

uint16_t ModRob::process_udp(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer) {
    if(packet[0]&(1<<7)) {
        //write command
        return ModRob::udp_write_command(packet, packet_size, udp_tx_buffer);
    }
    else {
        //read command
        if(packet[0]) {
            //read from device
            return ModRob::udp_read_data(packet, packet_size, udp_tx_buffer);
        }
        else {
            //read from metadata
            if(packet[1]) {
                //Module discovery
                return ModRob::udp_module_disc(udp_tx_buffer);
            }
            else {
                //Structure discovery
                return ModRob::udp_struct_disc(udp_tx_buffer);
            }
        }
    }
    return 0;
}