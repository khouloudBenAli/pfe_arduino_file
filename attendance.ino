#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h> //thoes library for ntp server 
#include <WiFiUdp.h>
#include <NTPClient.h>



//define
#define SS_PIN D4  
#define RST_PIN D3  
#define ON_Board_LED 2  //--> Defining an On Board LED, used for indicators when the process of connecting to a wifi router


//declaration des instances
MFRC522 mfrc522(SS_PIN, RST_PIN);   
ESP8266WebServer server(80);                 //--> Server on port 80
//IPAddress timeServerIP;                      // Définissez votre serveur NTP préféré ici
WiFiUDP ntpUDP;
WiFiClient client;                           // create a WiFiClient object




//declaration des constantes
const char*  ssid = "Redmi Note 9 Pro";
const char*  password = "Ben@ali.4";
const String ipServer = "192.168.1.6"; 

const char* ntpServerName = "pool.ntp.org"; // NTP server to be used,the client will use to synchronize its time.
const long  gmtOffset_sec = 3600;           // The time difference between Tunisia and UTC in seconds (GMT+1)
const int   daylightOffset_sec = 0;         // Daylight saving time offset, in seconds

const int heureDebut[] = {8, 10, 11, 13, 15, 16};   // Définir les heures de début et de fin pour chaque séance
const int minuteDebut[] = {15, 0, 40, 20, 0, 45};
const int heureFin[] = {9, 11, 13, 14, 16, 18};
const int minuteFin[] = {45, 30, 10, 50, 30, 15};
const int zeroChar = 48;    //code ascii de '0'

//NTP server access
NTPClient timeClient(ntpUDP, ntpServerName, gmtOffset_sec, daylightOffset_sec);  // Creating a NTP client object


//declaration des variables
String postData = "", seanceSend = "", StrUID = "", Sc = "", currentTime = "" , macAdr = "";

int  seance = -10;
char scChar;
int readsuccess;
byte readcard[4];
char str[32] = "";
String tag;




//Function for reading and obtaining a UID from a card or keychain
int getid() 
{
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
    if (mfrc522.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += mfrc522.uid.uidByte[i];
    }
    StrUID = tag ;
    tag = "";
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
  mfrc522.PICC_HaltA();
  return 1;
}


//Procedure to change the result of reading an array UID into a string
/*void array_to_string(byte array[], unsigned int len, char buffer[]) 
{
  for (unsigned int i = 0; i < len; i++)
  {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len * 2] = '\0';
} */


//retourner la sceance actuel
void seanceCheck()
{
    // Obtenir l'heure actuelle
  int heureActuelle = timeClient.getHours();
  int minuteActuelle = timeClient.getMinutes();
  int minuteActuelleTotale = heureActuelle * 60 + minuteActuelle; // Convertir l'heure actuelle en minutes

  // Afficher l'heure actuelle
  String formattedTime = timeClient.getFormattedTime();  // Getting the formatted time

  Serial.print("Heure actuelle: ");  
  Serial.println(formattedTime);

  // Déterminer la séance actuelle
  seance = 0;; // Initialiser a -1 pour indiquer qu'aucune séance n'a été trouvée
  for (int i = 0; i < sizeof(heureDebut)/sizeof(heureDebut[0]); i++) {
    int heureDebutTotale = heureDebut[i] * 60 + minuteDebut[i];
    int heureFinTotale = heureFin[i] * 60 + minuteFin[i];
    if (heureDebutTotale <= minuteActuelleTotale && heureFinTotale >= minuteActuelleTotale) {
      seance = i + 1; // Ajouter 1 pour obtenir le numéro de la séance à partir de l'indice (qui commence à 0)
      break; // Sortir de la boucle dès qu'une séance est trouvée
    }
  }
  // Afficher la séance actuelle
 if (seance != 0) {
  Serial.print("La séance en cours est la séance ");
  Serial.println(seance);
  } else {
  Serial.println("Il n'y a pas de séance en cours.");
  }
}


void setup() 
{
  Serial.begin(9600); 
  SPI.begin(); 
  mfrc522.PCD_Init(); 
  delay(500);

  //connect to the wifi
  WiFi.begin(ssid, password); 
  Serial.println("");
  pinMode(ON_Board_LED, OUTPUT);
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off Led On Board   
  Serial.print("Connecting");

  //WiFi status
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //Make the On Board Flashing LED on the process of connecting to the wifi router.
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
  }
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off the On Board LED when it is connected to the wifi router.
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);

  //MAC address
  WiFi.mode(WIFI_STA);
  Serial.print("MAC address: ");
  macAdr = WiFi.macAddress();
  Serial.println(macAdr);

  /*---------------------------------------------- Serveur NTP ---------------------------------------------- */  
  // Résolution de l'adresse IP du serveur NTP
  timeClient.begin();   // Starting the NTP client
  timeClient.update();  // Updating the time from the NTP server
  //display message
  Serial.println("Please tag a card or keychain to see the UID !");
  Serial.println("");
}



void loop() 
{
  readsuccess = getid();   
  if (readsuccess) {
    digitalWrite(ON_Board_LED, LOW);
    HTTPClient http;    //Declare object of class HTTPClient
    
    Serial.println("... ... ... ");
    seanceCheck();
    for (int i = 0; i < 7; i++) {
        if (i == seance){
          scChar = static_cast<char>(zeroChar + i);
        }
    }
    
    postData = "{\"UIDresult\":\"" + StrUID + "\",\"Sc\":\"" + scChar+"\",\"MACresult\":\"" + macAdr+"\"}";
    http.begin(client, "http://" + ipServer + "/user/getUID.php"); // use the client object in the begin() call
    http.addHeader("Content-Type", "application/json"); //Specify content-type header , json
    Serial.println("POST DATA contents : ");
    Serial.println(postData);
     


    int httpCode = http.POST(postData);   //Send the request
    
    String payload = http.getString();    //Get the response payload
    Serial.println("payload contents : ");
    Serial.println("THE UID OF THE SCANNED CARD IS : ");
    Serial.println(StrUID);
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
    http.end();  //Close connection
    delay(1000);
    digitalWrite(ON_Board_LED, HIGH);
  }
}

