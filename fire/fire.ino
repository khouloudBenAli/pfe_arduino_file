#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 
#include "DHT.h"

#define DHT11_PIN D2
#define flame_pin D1
#define gaz_pin   A0
// Replace the variables with your Wi-Fi credentials and MQTT broker IP address
const char* ssid = "Orange-AAC0";        //Orange-AAC0
const char* password = "DRTRG3FH0GM";           //DRTRG3FH0GM //Ben@ali.4
const char* mqtt_server = "192.168.1.103";
const int PORT = 8883;

// Replace with your MQTT topic
const char* topic = "esp";
const char* topic1 = "alert"; //***ADDITION***

String  macAdr = "";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHT11_PIN, DHT11);



void reconnect(){
  while (!client.connected()) {
    if (client.connect("esp32_client")) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(topic1); // ***ADDITION*** subscribe to the topic
    } else {
      Serial.println("Failed to connect to MQTT broker, try again in 5 seconds");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

 

//***ADDITION***
void callback(char* receivedTopic, byte* payload, unsigned int length) {
  Serial.print("Received message [");
  Serial.print(receivedTopic);
  Serial.print("]: ");

  // convert the received message into a string
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println(message);
}



void setup() {
  Serial.begin(9600);
  dht.begin();
  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqtt_server, PORT);

  //MAC address
    WiFi.mode(WIFI_STA);
    Serial.print("MAC address: ");
    macAdr = WiFi.macAddress();

  Serial.println(macAdr);
  // Set callback function      //***ADDITION***
  client.setCallback(callback);
  pinMode(flame_pin, INPUT);
  pinMode(gaz_pin,   INPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop(); // ***ADDITION*** listen for incoming messages

  // Read temperature and humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float gaz = analogRead(gaz_pin);
  float flame = !digitalRead(flame_pin);

  Serial.print("temp : "); 
  Serial.println(temperature);
  Serial.print("hum : ");
  Serial.println(humidity);
  Serial.print("flame : "); 
  Serial.println(flame);
  Serial.print("gaz : ");
  Serial.println(gaz);
  // Create a JSON object
  StaticJsonDocument<256> doc;
  doc["temperature"] = temperature;
  doc["humidity"]    = humidity;
  doc["gaz"]         = gaz;
  doc["flame"]       = flame;  
  doc["device"]      = macAdr;

  // Serialize the JSON object to a string
  String payload;
  serializeJson(doc, payload);

  Serial.println(payload);

   //Publish the payload to MQTT broker
   client.publish(topic, payload.c_str());

  // Wait a few seconds between measurements.
  delay(5000);
}

