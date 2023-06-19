#include <ESP8266WiFi.h>

String macAdr = "";

void setup() {
  //MAC address
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  Serial.print("MAC address: ");
  macAdr = WiFi.macAddress();
  Serial.println(macAdr);
}

void loop() {
  // your code here
}
