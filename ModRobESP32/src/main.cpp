#include <Arduino.h>

#include "modrob.h"
#include "multi_uart.h"

//Wifi UDP
#include "WiFi.h"
#include <WiFiUdp.h>

//Module type
//#define MODULE_OCTA
#define MODULE_JOINT
//#define MODULE_BRANCH


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
uint8_t module_attributes[] = {0,2, 16,2, 16,(uint8_t)-2, 0,(uint8_t)-2};
#elif defined(MODULE_JOINT)
//convert list of 2D points in int16 format in millimeters to list of uint8 MSB first
uint8_t module_attributes[] = {//(0,20) int16
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
uint8_t module_attributes[] = {0,2, 16,2, 16,(uint8_t)-2, 0,(uint8_t)-2};
#endif

// === ports configuations ===
//TX pin for all ports
#define TX_PIN      13
#define RX_PIN_0    27
#define RX_PIN_1    26
#define RX_PIN_2    25
#define RX_PIN_3    17
#define RX_PIN_4    18
#define RX_PIN_5    19
#define RX_PIN_6    21
#define RX_PIN_7    22

#if defined(MODULE_OCTA)
#elif defined(MODULE_JOINT)
//2D point (int16) and 2D vector (int8)
uint8_t port0_location[] = {
                            //(0,0) 2D point
                            0,0,
                            0,0,
                            //(1,0) 2D vector
                            1,0};

uint8_t port1_location[] = {
                            //(160,0) 2D point
                            (uint8_t)(160>>8), (uint8_t)(160&(0xFF)),
                            0,0,
                            //(-1,0) 2D vector
                            (uint8_t)-1,0};
#elif defined(MODULE_BRANCH)
#endif

// === devices definitions ===
uint8_t servo_angle = 0;
xTaskHandle servo_task_handle = NULL;
static void servo_task(void *parameters) {
    while(1) {
        if(servo_angle > 0) {
            analogWrite(32, 200);
            servo_angle--;
        } else {
            analogWrite(32,0);
        }
        vTaskDelay(20);
    }
}
void servo_write(uint8_t *command) {
    servo_angle = command[0];
    return;
}
uint8_t device_servo_attributes[] = {5,6,7};
Device device1 = Device(1,servo_write, 0,NULL, device_servo_attributes,sizeof(device_servo_attributes));

void force_read(uint8_t *data) {
    data[0] = (uint8_t)(analogRead(34)%(0xFF));
    data[1] = (uint8_t)(analogRead(34)>>8);
    return;
}
uint8_t device_force_attributes[] = {7,8,9};
Device device2 = Device(0,NULL,2,force_read,device_force_attributes,sizeof(device_force_attributes));


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

    //module setup
    module.setup(WiFi.localIP()[3], sizeof(port0_location), 5);
    module.set_port_tx(TX_PIN);
    module.add_port_rx(RX_PIN_0, port0_location);
    module.add_port_rx(RX_PIN_1, port1_location);
    module.set_module_attributes(module_attributes, sizeof(module_attributes));

    //device adding
    pinMode(32, OUTPUT);
    xTaskCreate(servo_task, "ServoTask", 1024, NULL, 1, &servo_task_handle);
    module.add_device(device1);
    module.add_device(device2);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    //Serial.println("packet received");
    int len = udp.read(udp_buffer_rx, 0xFF);

    uint16_t tx_len = module.process_udp(udp_buffer_rx, len, udp_buffer_tx);
    if(tx_len > 0) {
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.write(udp_buffer_tx, tx_len);
        udp.endPacket();
    }
  }
}