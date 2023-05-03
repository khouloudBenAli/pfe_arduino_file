#include <ESP8266WiFi.h>  
#include <WiFiUdp.h>      
#include <NTPClient.h>    

const char* ssid = "Redmi Note 9 Pro";     
const char* password = "Ben@ali.4";        

const char* ntpServerName = "pool.ntp.org"; // NTP server to be used,the client will use to synchronize its time.
const long  gmtOffset_sec = 3600;           // The time difference between Tunisia and UTC in seconds (GMT+1)
const int   daylightOffset_sec = 0;         // Daylight saving time offset, in seconds

WiFiUDP ntpUDP;                             // object used for sending and receiving NTP packets over the network.
NTPClient timeClient(ntpUDP, ntpServerName, gmtOffset_sec, daylightOffset_sec);  // Creating a NTP client object

void setup() {
  Serial.begin(9600);   
  WiFi.begin(ssid, password);  

  while (WiFi.status() != WL_CONNECTED) {   
    delay(1000);             
    Serial.println("Connexion au WiFi...");  
  }
  Serial.println("Connecte au WiFi");  
  timeClient.begin();   // Starting the NTP client
}

void loop() {
  timeClient.update();  // Updating the time from the NTP server

  String formattedTime = timeClient.getFormattedTime();  // Getting the formatted time

  Serial.print("Heure exacte en Tunisie: ");  
  Serial.println(formattedTime);

  delay(5000);  
}
