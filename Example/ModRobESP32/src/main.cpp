#include <Arduino.h>

#include "modrob.h"
#include "multi_uart.h"

//Wifi UDP
#include "WiFi.h"
#include <WiFiUdp.h>

// === Module type ===
#define MODULE_OCTA
//#define MODULE_JOINT
//#define MODULE_BRANCH

//utility, used to convert an int16 to two uint8
#define INT16_TO_UINT8(value)  (uint8_t)((value)>>8),(uint8_t)((value)&0xFF)

// --- Wifi parameters ---
const char* wifi_ssid = "ModRob";
const char* wifi_pass = "test1234";
const uint16_t wifi_port = 9999;
WiFiUDP udp;
uint8_t udp_buffer_rx[255];
uint8_t udp_buffer_tx[255];

// === Module parameters ===
ModRob module;

#if defined(MODULE_OCTA)
//Outline of octagonal module (octagon) as list of 2D points in millimeters
uint8_t module_attributes[] = { //List of 2D points
                                INT16_TO_UINT8(0), INT16_TO_UINT8(-24),
                                INT16_TO_UINT8(33), INT16_TO_UINT8(-57),
                                INT16_TO_UINT8(80), INT16_TO_UINT8(-57),
                                INT16_TO_UINT8(113), INT16_TO_UINT8(-24),
                                INT16_TO_UINT8(113), INT16_TO_UINT8(24),
                                INT16_TO_UINT8(80), INT16_TO_UINT8(57),
                                INT16_TO_UINT8(33), INT16_TO_UINT8(57),
                                INT16_TO_UINT8(0), INT16_TO_UINT8(24)
                            };

#elif defined(MODULE_JOINT)
//Outline of joint module (rectangle) as list of 2D points in millimeters
uint8_t module_attributes[] = { //List of 2D points
                                0,0,
                                0,20,
                                //(160,20) int16
                                (uint8_t)(160>>8), (uint8_t)(160&(0xFF)),
                                0,20,

                                //(160,-20) int16
                                (uint8_t)(160>>8), (uint8_t)(160&(0xFF)),
                                (uint8_t)((-20)>>8), (uint8_t)((-20)&(0xFF)),

                                //(0,-20) int16
                                0,0,
                                (uint8_t)((-20)>>8), (uint8_t)((-20)&(0xFF))
                                };

#elif defined(MODULE_BRANCH)
//Outline of branch module (rectangle) as list of 2D points in millimeters
uint8_t module_attributes[] = { //List of 2D points
                                INT16_TO_UINT8(0), INT16_TO_UINT8(31),
                                INT16_TO_UINT8(84), INT16_TO_UINT8(31),
                                INT16_TO_UINT8(84), INT16_TO_UINT8(-31),
                                INT16_TO_UINT8(0), INT16_TO_UINT8(-31)
                            };
#endif

// === ports configuations ===
//TX pin for all ports
#define TX_PIN      13
//RX pin for each port
#define RX_PIN_0    27
#define RX_PIN_1    26
#define RX_PIN_2    25
#define RX_PIN_3    17
#define RX_PIN_4    18
#define RX_PIN_5    19
#define RX_PIN_6    21
#define RX_PIN_7    22

#if defined(MODULE_OCTA)
//port location and orientation for each port
//location is a 2D point of int16, orientation is a 2D vector of int8
uint8_t port0_location[] = {
                            //(0,0) 2D point
                            INT16_TO_UINT8(0),
                            INT16_TO_UINT8(0),
                            //(1,0) 2D vector
                            1,0};
uint8_t port1_location[] = {
                            //(0,0) 2D point
                            INT16_TO_UINT8(17),
                            INT16_TO_UINT8(-40),
                            //(1,0) 2D vector
                            1,1};
uint8_t port2_location[] = {
                            //(0,0) 2D point
                            INT16_TO_UINT8(57),
                            INT16_TO_UINT8(-57),
                            //(1,0) 2D vector
                            0,1};
uint8_t port3_location[] = {
                            //(0,0) 2D point
                            INT16_TO_UINT8(97),
                            INT16_TO_UINT8(-40),
                            //(1,0) 2D vector
                            (uint8_t)(-1),1};
uint8_t port4_location[] = {
                            //(0,0) 2D point
                            INT16_TO_UINT8(113),
                            INT16_TO_UINT8(0),
                            //(1,0) 2D vector
                            (uint8_t)(-1),0};
uint8_t port5_location[] = {
                            //(0,0) 2D point
                            INT16_TO_UINT8(97),
                            INT16_TO_UINT8(40),
                            //(1,0) 2D vector
                            (uint8_t)(-1), (uint8_t)(-1)};
uint8_t port6_location[] = {
                            //(0,0) 2D point
                            INT16_TO_UINT8(57),
                            INT16_TO_UINT8(57),
                            //(1,0) 2D vector
                            0,(uint8_t)(-1)};
uint8_t port7_location[] = {
                            //(0,0) 2D point
                            INT16_TO_UINT8(17),
                            INT16_TO_UINT8(40),
                            //(1,0) 2D vector
                            1,(uint8_t)(-1)};

#elif defined(MODULE_JOINT)
//port location and orientation for each port
//location is a 2D point of int16, orientation is a 2D vector of int8
uint8_t port0_location[] = {
                            //(0,0) 2D point
                            0,0,
                            0,0,
                            //(1,0) 2D vector
                            1,0};

uint8_t port1_location[] = {
                            //(160,0) 2D point
                            INT16_TO_UINT8(160),
                            0,0,
                            //(-1,0) 2D vector
                            (uint8_t)-1,0};
                            
#elif defined(MODULE_BRANCH)
//port location and orientation for each port
//location is a 2D point of int16, orientation is a 2D vector of int8
uint8_t port0_location[] = {
                            //(0,0) 2D point
                            INT16_TO_UINT8(0), INT16_TO_UINT8(0),
                            //(1,0) 2D vector
                            1,0};

uint8_t port1_location[] = {
                            //(0,0) 2D point
                            INT16_TO_UINT8(42), INT16_TO_UINT8(-31),
                            //(1,0) 2D vector
                            0,1};

uint8_t port2_location[] = {
                            //(0,0) 2D point
                            INT16_TO_UINT8(84), INT16_TO_UINT8(0),
                            //(1,0) 2D vector
                            (uint8_t)-1,0};

uint8_t port3_location[] = {
                            //(0,0) 2D point
                            INT16_TO_UINT8(42), INT16_TO_UINT8(31),
                            //(1,0) 2D vector
                            0,(uint8_t)-1};
#endif

// ===========================
// === devices definitions ===
// ===========================

// === Pressure sensor device definition ===
#define PRESSURE_PIN        34

//function that will be called when a read command is called on this device
void force_read(uint8_t *data) {
    data[0] = (uint8_t)(analogRead(PRESSURE_PIN)%(0xFF));
    data[1] = (uint8_t)(analogRead(PRESSURE_PIN)>>8);
    return;
}
//dummy attributes
uint8_t device_force_attributes[] = {7,8,9};
//defining a device with 0 command bytes and  2 data bytes (int18 = 2 x uint8)
Device device1 = Device(0,NULL,2,force_read,device_force_attributes,sizeof(device_force_attributes));


// === Servomotor device definition ===
#define SERVO_PIN        32

//function that will be called when a write command is called on this device
void servo_write(uint8_t *command) {
    analogWrite(SERVO_PIN, command[0]);
    return;
}
//dummy attributes
uint8_t device_servo_attributes[] = {5,6,7};
//defining a device with 1 command bytes and  0 data bytes
Device device2 = Device(1,servo_write, 0,NULL, device_servo_attributes,sizeof(device_servo_attributes));

void setup() {
    //Debug
    Serial.begin(115200);

    //Wifi setup
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setMinSecurity(WIFI_AUTH_OPEN);
    WiFi.begin(wifi_ssid, wifi_pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.println(WiFi.localIP()[3]);
    udp.begin(wifi_port);

    //module setup, set the ID as last number of IP address, port attributes sizes and max number of devices for this module (5)
    module.setup(WiFi.localIP()[3], sizeof(port0_location), 5);
    //set the pin number to use for the TX signal of all docking ports
    module.set_port_tx(TX_PIN);

    //add all the dockings ports to the module with their pin and attributes
    module.add_port_rx(RX_PIN_0, port0_location);
    module.add_port_rx(RX_PIN_1, port1_location);
    #if defined(MODULE_OCTA) || defined(MODULE_BRANCH)
    module.add_port_rx(RX_PIN_2, port2_location);
    module.add_port_rx(RX_PIN_3, port3_location);
    #endif
    #if defined(MODULE_OCTA)
    module.add_port_rx(RX_PIN_4, port4_location);
    module.add_port_rx(RX_PIN_5, port5_location);
    module.add_port_rx(RX_PIN_6, port6_location);
    module.add_port_rx(RX_PIN_7, port7_location);
    #endif
    //set the module attributes
    module.set_module_attributes(module_attributes, sizeof(module_attributes));

    //setting up stuff for the devices
    pinMode(PRESSURE_PIN, INPUT);
    pinMode(SERVO_PIN, OUTPUT);
    //adding the devices to the module
    module.add_device(device1);
    module.add_device(device2);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(udp_buffer_rx, 0xFF);
    uint16_t tx_len = module.process_udp(udp_buffer_rx, len, udp_buffer_tx);
    if(tx_len > 0) {
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.write(udp_buffer_tx, tx_len);
        udp.endPacket();
    }
  }
}