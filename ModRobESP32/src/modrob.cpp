#include "modrob.h"
#include "multi_uart.h"
#include "device.h"

bool ModRob::setup(uint8_t module_id, uint8_t bytes_per_port_attribute, uint8_t max_devices_number) {
    ModRob::module_id = module_id;

    ModRob::neighbours_id = (uint8_t*)pvPortMalloc(0xFF);
    ModRob::neighbours_id_size = 0;

    ModRob::ports_attributes = (uint8_t*)pvPortMalloc(0xFF*bytes_per_port_attribute);
    ModRob::ports_attributes_size = 0;
    ModRob::bytes_per_port_attribute = bytes_per_port_attribute;

    ModRob::module_attributes_size = 0;

    ModRob::devices = (Device*)pvPortMalloc(sizeof(Device)*max_devices_number);
    ModRob::devices_size = 0;
    ModRob::devices_size_max = max_devices_number;
    return 1;
}

bool ModRob::add_port_rx(uint8_t rx_pin, uint8_t *port_attributes) {
    ModRob::multi_uart.add_receiver(rx_pin);
    memcpy( ModRob::ports_attributes + ModRob::ports_attributes_size,
            port_attributes,
            ModRob::bytes_per_port_attribute);

    ModRob::ports_attributes_size += ModRob::bytes_per_port_attribute;
    ModRob::neighbours_id_size++;
    return 1;
}

bool ModRob::set_port_tx(uint8_t tx_pin) {
    ModRob::multi_uart.set_transmitter(tx_pin);
    return 1;
}

bool ModRob::add_device(Device device) {
    if(ModRob::devices_size < ModRob::devices_size_max) {
        ModRob::devices[ModRob::devices_size++] = device;
        return 1;
    }
    return 0;
}

bool ModRob::set_module_attributes(uint8_t *module_attributes, uint16_t module_attributes_size) {
    ModRob::module_attributes_size = module_attributes_size;
    ModRob::module_attributes = module_attributes;
    return 1;
}

uint16_t ModRob::udp_write_command(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer) {
    uint8_t target_device = packet[0]&(~(1<<7));
    ModRob::devices[target_device-1].set_command(packet+1,packet_size-1);
   return 0;
}

uint16_t ModRob::udp_read_data(uint8_t *packet, uint8_t packet_size, uint8_t *udp_tx_buffer) {
    uint8_t target_device = packet[0]&(~(1<<7));
    if(target_device <= ModRob::devices_size) {
        return ModRob::devices[target_device-1].get_data(udp_tx_buffer);
    }
    return 0;
}

uint16_t ModRob::udp_module_disc(uint8_t *udp_tx_buffer) {
    uint8_t index = 0;
    for(int i=0;i<ModRob::devices_size;i++) {
        udp_tx_buffer[index++] = ModRob::devices[i].attributes_size + 2;
        udp_tx_buffer[index++] = ModRob::devices[i].commands_size;
        udp_tx_buffer[index++] = ModRob::devices[i].data_size;
        memcpy(udp_tx_buffer + index, ModRob::devices[i].attributes, ModRob::devices[i].attributes_size);
        index += ModRob::devices[i].attributes_size;
    }
   return index;
}

uint16_t ModRob::udp_struct_disc(uint8_t *udp_tx_buffer) {
    //Update the neighbours id
    ModRob::multi_uart.xfer(9600, 5000, ModRob::module_id, 5000);
    for(int i=0;i<ModRob::neighbours_id_size;i++) {
        ModRob::neighbours_id[i] = ModRob::multi_uart.get_value(i);
    }

    uint8_t buffer_length = 0;

    //Send neighbour ids
    udp_tx_buffer[buffer_length++] = ModRob::neighbours_id_size;
    memcpy(udp_tx_buffer+buffer_length, ModRob::neighbours_id, ModRob::neighbours_id_size);
    buffer_length += ModRob::neighbours_id_size;

    //Send port locations
    udp_tx_buffer[buffer_length++] = ModRob::ports_attributes_size;
    if(ModRob::ports_attributes_size > 0) {
        memcpy(udp_tx_buffer+buffer_length, ModRob::ports_attributes, ModRob::ports_attributes_size);
        buffer_length += ModRob::ports_attributes_size;
    }

    //Send additionnal data
    udp_tx_buffer[buffer_length++] = ModRob::module_attributes_size;
    if(ModRob::module_attributes_size > 0) {
        memcpy(udp_tx_buffer+buffer_length, ModRob::module_attributes, ModRob::module_attributes_size);
        buffer_length += ModRob::module_attributes_size;
    }

   return buffer_length;
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