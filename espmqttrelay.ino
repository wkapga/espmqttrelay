/*
D0	Relay1   
D1	Relay2

D6	Button1	toggles Relay1
D7	Button2 toggles Relay2
D4	Button3   
D8	Led1 	Indicators(on/off)

*/

#include "secret.h" 
#include <ESP8266WiFi.h>
#include <PubSubClient.h> 
#include <Bounce2.h>

#define BUTTON_PIN1 D6
#define BUTTON_PIN2 D7 
#define BUTTON_PIN3 D4
// #define LED_PIN1 D5
// #define LED_PIN1 D4
// #define LED_PIN2 D6
 #define LED_PIN3 D8
#define RELAY1 D0
#define RELAY2 D1

// WIFI parameters 
const char* SSID = MYWIFISSID;
const char* PASSWORD = MYWIFIPASS;

// MQTT Config
const char* BROKER_MQTT = MYMQTTBROKER; // MQTT Broker IP 
int BROKER_PORT = 1883;
WiFiClient espClient;
PubSubClient MQTT(espClient); 

int ledState1 = LOW;
int ledState2 = LOW;
int ledState3 = HIGH;

Bounce debouncer1 = Bounce(); // Instantiate a Bounce object
Bounce debouncer2 = Bounce(); // Instantiate a Bounce object
Bounce debouncer3 = Bounce(); // Instantiate a Bounce object

void setup() {
  
 
  initPins();
  initSerial();
  initWiFi();
  initMQTT();
}


void initPins() {
/*  pinMode(LED_PIN1,OUTPUT); // Setup the LED
  digitalWrite(LED_PIN1,ledState1);
  pinMode(LED_PIN2,OUTPUT); // Setup the LED
  digitalWrite(LED_PIN2,ledState2); */
  pinMode(LED_PIN3,OUTPUT); // Setup the LED 
  digitalWrite(LED_PIN3,ledState3); 
  
  pinMode(RELAY1,OUTPUT); 
  digitalWrite(RELAY1,ledState1);
  pinMode(RELAY2,OUTPUT);
  digitalWrite(RELAY2,ledState2);

  debouncer1.attach(BUTTON_PIN1,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer1.interval(25); // Use a debounce interval of 25 milliseconds
  debouncer2.attach(BUTTON_PIN2,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer2.interval(25); // Use a debounce interval of 25 milliseconds
  debouncer3.attach(BUTTON_PIN3,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer3.interval(25); // Use a debounce interval of 25 milliseconds
}

void initSerial() {
  Serial.begin(9600);
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
 

  if (strcmp(topic,"haus/light/lamp1")==0){ // 0 means there is a match
    if ((message == "1") && (ledState1 == LOW) ) {    toggle1(); Serial.println("debug1"); }
    if ((message == "0") && (ledState1 == HIGH)) {    toggle1();  }
  }
  if (strcmp(topic,"haus/light/lamp2")==0){ // 0 means there is a match
    if ((message == "1") && (ledState2 == LOW)) {    toggle2();  }
    if ((message == "0") && (ledState2 == HIGH)) {    toggle2();  }
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
      MQTT.subscribe("haus/light/lamp1"); 
      MQTT.subscribe("haus/light/lamp2"); 
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

void toggle1(){
   ledState1 = !ledState1;
  // digitalWrite(LED_PIN1,ledState1); // Apply new LED state
   digitalWrite(RELAY1,ledState1); // Apply new LED state
   Serial.println("toggle1");
} 

void toggle2(){
   ledState2 = !ledState2;
   //digitalWrite(LED_PIN2,ledState2); // Apply new LED state
   digitalWrite(RELAY2,ledState2); // Apply new LED state
   Serial.println("toggle2");
}

void toggle3(){
   ledState3 = !ledState3;
   digitalWrite(LED_PIN3,ledState3); // Apply new LED state
}
 
void loop() {

   debouncer1.update(); // Update the Bounce instance
   debouncer2.update(); // Update the Bounce instance
   debouncer3.update(); // Update the Bounce instance
   
   if ( debouncer1.rose() ) { toggle1(); } // button normal closed
   if ( debouncer2.rose() ) { toggle2(); } // 
   if ( debouncer3.fell() ) { toggle3(); } // Call code if button transitions from HIGH to LOW
  
   if (ledState3 == HIGH) {
      if (!MQTT.connected()) {
         reconnectMQTT(); // Retry Worker MQTT Server connection
      }
      recconectWiFi(); // Retry WiFi Network connection
      MQTT.loop();
   }
}
