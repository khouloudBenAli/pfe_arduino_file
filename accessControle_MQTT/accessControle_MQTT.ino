#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 
#include <SPI.h>
#include <MFRC522.h>

//#define lock D8 
#define access D0 
#define denied D8 
#define buzzer D2 

/************* declaration des constanes *************/
constexpr uint8_t RST_PIN = D3;
constexpr uint8_t SS_PIN = D4;     
const char* ssid = "Orange-AAC0";
const char* password = "DRTRG3FH0GM"; //DRTRG3FH0GM //Ben@ali.4
const char* mqtt_server = "192.168.1.103"; //MQTT broker IP address
const int PORT = 8883; 
const char* topic = "acces";      // Replace with your MQTT topic
const char* topic1 = "response";  //***ADDITION***
const unsigned long delayDuration = 500; // 1 second
unsigned long accessStartTime = 0;
unsigned long deniedStartTime = 0;
int deniedCount = 0;




/************* declaration des variables *************/
String  macAdr = "", uid = "", uid1 = "", message = "";
unsigned long previousTime = 0;
unsigned long currentTime = 0;

MFRC522 rfid(SS_PIN, RST_PIN);     // Instance of the class
MFRC522::MIFARE_Key key;

WiFiClient espClient;
PubSubClient client(espClient);

//connect to MQTT broker
void reconnect()
{
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
void callback(char* receivedTopic, byte* payload, unsigned int length) 
{
  message = "";
  Serial.print("Received message [");
  Serial.print(receivedTopic);
  Serial.print("]: ");

  // convert the received message into a string
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
}


void setup() 
{
  Serial.begin(9600);
  pinMode(access, OUTPUT);
  pinMode(denied, OUTPUT);
  
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqtt_server, PORT);

  // Set callback function      //***ADDITION***
  client.setCallback(callback);

  //MAC address
  WiFi.mode(WIFI_STA);
  Serial.print("MAC address: ");
  macAdr = WiFi.macAddress();
  Serial.println(macAdr);

  SPI.begin(); 
  Serial.println();
  Serial.println("Begin");
}


void loop() 
{
  client.loop(); // ***ADDITION*** listen for incoming messages
  currentTime = millis(); // Get the current time

  // Check if the desired delay duration has passed
  if (currentTime - previousTime >= delayDuration) {
    previousTime = currentTime;
  if (!client.connected()) {
    reconnect();
  }

  
  if(message == "accepted"){
    digitalWrite(access, HIGH);
    accessStartTime = currentTime; // Start the timer for access LED
    digitalWrite(denied, LOW);
    message = "";
    
  }else if (message == "denied"){
    digitalWrite(access, LOW);
    deniedStartTime = currentTime; // Start the timer for denied LED
    digitalWrite(denied, HIGH);
    message = "";
  }

  /************************************************************************/
  // Check if access LED duration has elapsed (2 seconds)
    if (digitalRead(access) == HIGH && currentTime - accessStartTime >= 2000) {
      digitalWrite(access, LOW);
    }

    // Check if denied LED duration has elapsed (3 times, 1 second each)
    if (digitalRead(denied) == HIGH && currentTime - deniedStartTime >= 1000) {
      deniedCount++;
      digitalWrite(denied, LOW);
      deniedStartTime = currentTime; // Start the timer for the next denied LED
    }

    if (deniedCount >= 3) {
      digitalWrite(denied, LOW);
    }
  /************************************************************************/

  if (!rfid.PICC_IsNewCardPresent())
    return;
    if (rfid.PICC_ReadCardSerial()) {
      for (byte i = 0; i < 4; i++) {
        uid += String(rfid.uid.uidByte[i], HEX);
      }
      Serial.println(uid);
      
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();

      // Create a JSON object
      StaticJsonDocument<256> doc;
      doc["uid"]    = uid; 
      doc["device"] = macAdr;

      // Serialize the JSON object to a string
      String payload;
      serializeJson(doc, payload);

      Serial.println(payload);

      // Publish the payload to MQTT broker
      client.publish(topic, payload.c_str());

      uid = "";
  }
}
}

