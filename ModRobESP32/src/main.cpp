#include <Arduino.h>

#include "modrob.h"
#include "multi_uart.h"

//Wifi UDP
#include "WiFi.h"
#include <WiFiUdp.h>

// --- Wifi parameters ---
// need to be changed by the user
const char* wifi_ssid = "ModRob";
const char* wifi_pass = "test1234";
const uint16_t wifi_port = 9999;

// === UDP definitions ===
WiFiUDP udp;
uint8_t udp_buffer_rx[255];
uint8_t udp_buffer_tx[255];

// === Module object declaration ===
// methods of this object should be called by the user to configure it
ModRob module;

void setup() {
    // === Wifi setup ==
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setMinSecurity(WIFI_AUTH_OPEN);
    WiFi.begin(wifi_ssid, wifi_pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    udp.begin(wifi_port);

    // --- module setup ---
    // -- ADD YOUR CODE HERE --
    //
}

void loop() {
    // Reads a UDP packet and passes it to the module object
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