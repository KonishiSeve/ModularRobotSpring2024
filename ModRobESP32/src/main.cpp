#include <Arduino.h>
#include "modrob.h"

//Wifi UDP
#include "WiFi.h"
#include <WiFiUdp.h>

// --- Wifi parameters ---
const char* wifi_ssid = "ModRob";
const char* wifi_pass = "test1234";
const uint16_t wifi_port = 9999;
WiFiUDP udp;
uint8_t udp_buffer_rx[255];
uint8_t udp_buffer_tx[255];

// --- Module parameters ---
Module module;

void setup() {
    //Debug
    Serial.begin(115200);

    //Wifi setup
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.setMinSecurity(WIFI_AUTH_OPEN);
    WiFi.begin(wifi_ssid, wifi_pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.println(WiFi.localIP()[3]);
    udp.begin(wifi_port);

    //module setup
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
      //Serial.println("packet received");
      int len = udp.read(udp_buffer_rx, 0xFF);

      uint16_t tx_len = module.process_udp(udp_buffer_rx, len, udp_buffer_tx);
      Serial.println(udp.remoteIP());
      Serial.println(udp.remotePort());
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.write(udp_buffer_tx, tx_len);
      udp.endPacket();
  }
}