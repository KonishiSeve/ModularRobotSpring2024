#include <Arduino.h>

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

#define DEBUG_PIN   13

void setup() {
    //Debug
    Serial.begin(115200);
    pinMode(DEBUG_PIN, OUTPUT);
    digitalWrite(DEBUG_PIN, 0);

    //Wifi setup
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.setMinSecurity(WIFI_AUTH_OPEN);
    WiFi.setSleep(false);
    WiFi.begin(wifi_ssid, wifi_pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    udp.begin(wifi_port);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
      digitalWrite(DEBUG_PIN, 1);
      //Serial.println("packet received");
      int len = udp.read(udp_buffer_rx, 0xFF);
      udp_buffer_tx[0] = 0x09;
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.write(udp_buffer_tx, 1);
      udp.endPacket();
      digitalWrite(DEBUG_PIN, 0);
  }
}
