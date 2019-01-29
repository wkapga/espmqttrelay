/*
D0	Relay1   
D1	Relay2

D2	Button1	toggles Relay1
D3	Button2 toggles Relay2
D4	Button3 toggles Listening to mqtt

D5	Led1 	Indicators(on/off)
D6	Led2
D7	Led3
*/

// This example toggles the debug LED (pin 13) on or off
// when a button on pin 2 is pressed.

// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce2

#include "secret.h" 
#include <ESP8266WiFi.h>
#include <PubSubClient.h> 
#include <Bounce2.h>

#define BUTTON_PIN1 D2
#define BUTTON_PIN2 D3
#define BUTTON_PIN3 D4
//#define LED_PIN1 D5
#define LED_PIN1 D4
#define LED_PIN2 D6
#define LED_PIN3 D7
#define RELAY1 D0
#define RELAY2 D1

// WIFI parameters 
const char* SSID = MYWIFISSID;
const char* PASSWORD = MYWIFIPASS;

// MQTT Config
const char* BROKER_MQTT = MYMQTTBROKER; // MQTT Broker IP 
int BROKER_PORT = 1883;
WiFiClient espClient;
PubSubClient MQTT(espClient); // Instanciar Cliente MQTT

int ledState1 = LOW;
int ledState2 = LOW;
int ledState3 = LOW;

Bounce debouncer = Bounce(); // Instantiate a Bounce object

void setup() {
  
  debouncer.attach(BUTTON_PIN1,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer.interval(25); // Use a debounce interval of 25 milliseconds
  
  
  pinMode(LED_PIN1,OUTPUT); // Setup the LED
  digitalWrite(LED_PIN1,ledState1);
 
  initPins();
  initSerial();
  initWiFi();
  initMQTT();
}



void initPins() {
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
}

void initSerial() {
  Serial.begin(115200);
}

void initWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting: ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASSWORD); // Wifi Connect
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print(SSID);
  Serial.println(" | IP ");
  Serial.println(WiFi.localIP());
}

// MQTT Broker connection
void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}

// Receive messages
void mqtt_callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    message += c;
  }
  Serial.print("Topic ");
  Serial.print(topic);
  Serial.print(" | ");
  Serial.println(message);

  if (message == "1") {
    digitalWrite(LED_PIN1, 1);
    } else {
      digitalWrite(LED_PIN1, 0);
    }
    message = "";
    Serial.println();
    Serial.flush();
  }

  void reconnectMQTT() {
    while (!MQTT.connected()) {
      Serial.print("Connection to MQTT Broker: ");
      Serial.println(BROKER_MQTT);
      if (MQTT.connect("ESP8266")) {
        Serial.println("Connected");
      MQTT.subscribe("haus/light/esplamp"); 
      } else {
        Serial.println("Connection failed");
        Serial.println("Retry in 2 secs");
        delay(2000);
      }
    }
  }

  void recconectWiFi() {
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
    }
  }

void loop() {

   debouncer.update(); // Update the Bounce instance
   
   if ( debouncer.fell() ) {  // Call code if button transitions from HIGH to LOW
     ledState1 = !ledState1; // Toggle LED state
     digitalWrite(LED_PIN1,ledState1); // Apply new LED state
   }
    if (!MQTT.connected()) {
    reconnectMQTT(); // Retry Worker MQTT Server connection
  }
  recconectWiFi(); // Retry WiFi Network connection
  MQTT.loop();
}
