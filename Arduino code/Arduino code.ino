#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "DHT.h"
// WiFi
const char *ssid = "Akbar";           // Enter your WiFi name
const char *password = "mseq0583";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "n5081226.us-east-1.emqx.cloud";

const char *topic = "topic_hum";
const char *led = "topic_hum/led";
const char *mqtt_username = "test";
const char *mqtt_password = "test1234";
const int mqtt_port = 15323;

char pub_str[100];

#define DHTPIN D4
#define LED D2

#define DHTTYPE DHT11


DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

#define flameLED D0
#define isFlamePin D1
#define buzzer D5

int isFlame = LOW;  // HIGH MEANS NO FLAME
void setup() {

  pinMode(flameLED, OUTPUT);   // put onboard LED as output
  pinMode(isFlamePin, INPUT);  //flame sensor should be input as it is giving data
  pinMode(buzzer, OUTPUT);     
  // Set software serial baud to 115200;
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  dht.begin();
  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp8266-client";
    // client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void callback(char *topic, byte *message, unsigned int length) {

  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }


  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message

  if (String(topic) == led) {
    if (messageTemp == "false") {
      digitalWrite(LED, LOW);
    }

    if (messageTemp == "true") {

      digitalWrite(LED, HIGH);
    }
  }
}
void reconnect() {
  // Loop until we're reconnected
  String client_id = "esp8266-client";
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(client_id.c_str())) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(led);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  client.subscribe(led);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  String msg = String(t) + ";" + String(h);
  int str_len = msg.length() + 1;
  char char_array[str_len];
  isFlame = digitalRead(isFlamePin); 

  if (isFlame == LOW)                 //if it is low
  {
     digitalWrite(flameLED, LOW);
    
    Serial.println("Alert");
     //LED on
  } else                          //if not
  {
    digitalWrite(flameLED, HIGH);  //off the LED
    tone(buzzer, 1000);           // Send 1KHz sound signal...
    delay(1000);                  // ...for 1 sec
    noTone(buzzer);               // Stop sound...
    delay(1000);                  // ...for 1sec
  }
  
  client.loop();
  client.publish(topic, (char *)msg.c_str());
Serial.println(isFlame);
  delay(3000);
}