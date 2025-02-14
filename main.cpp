#include <Arduino.h>
#include <WiFi.h>
#include "PubSubClient.h"
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>
#include <Wire.h>

#define led_pin 18
#define gassensor_pin 32
#define rainsensor_pin  25 
#define servopin        5
#define ldr_pin 36 

// create servo object to control a servo
Servo servo; 

// variables will change:
int prev_rain_state;    // the previous state of rain sensor
int rain_state; // the current state of rain sensor


// WiFi credentials
char ssid[] = "Meryem";
char pass[] = "jjjjjjjj";

//---- HiveMQ Cloud Broker settings
const char* mqtt_server = "7df5cda1fb364c7f9e8563844866e7c9.s1.eu.hivemq.cloud";
const char* mqtt_username = "my_cloud";
const char* mqtt_password = "Cloudinsky2";
const int mqtt_port = 8883;

WiFiClientSecure espClient;  
PubSubClient client(espClient);

// HiveMQ Cloud Let's Encrypt CA certificate
static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

unsigned long lastMsg = 0;
int value = 0;



void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we’re reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection… ");
    String clientId = "ESP32Client";
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected!");
      // Once connected, publish an announcement...
      client.publish("ESP32/status", "connected"); // IMPORTANT FOR PUBLISHING DATA
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}





void setup() {

  Serial.begin(115200);
  pinMode(gassensor_pin, INPUT);
  pinMode(rainsensor_pin, INPUT); 
  pinMode(ldr_pin, INPUT);
  servo.attach(servopin);
  servo.write(0);
  rain_state = digitalRead(rainsensor_pin); 
  delay(500);
  setup_wifi();
  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

}

void loop() {
 int gasState = digitalRead(gassensor_pin);

  if (gasState == HIGH){
    Serial.println("The gas is NOT present");
    digitalWrite(led_pin,LOW);
  }else{
    Serial.println("The gas is present");
    digitalWrite(led_pin,HIGH);
    delay(5000);
  }  
  prev_rain_state = rain_state;             // save the last state
  rain_state = digitalRead(rainsensor_pin); // read new state

  int ldr_reading = analogRead(ldr_pin);
  Serial.print("The AO value: ");
  Serial.println(ldr_reading);

  if ((rain_state == LOW && prev_rain_state == HIGH ) || ldr_reading>3500 ||  (rain_state == LOW && prev_rain_state == LOW )) { 
    servo.write(90);
  }
  else
  if ((rain_state == HIGH && prev_rain_state == LOW) || ldr_reading<3500||(rain_state == HIGH && prev_rain_state == HIGH) ){ 
    servo.write(0);
  }

   if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg=now;
    if(gasState==HIGH){
      Serial.print("Publish gas message: ");
      Serial.println(0);
      client.publish("ESP/Gas",String(0).c_str());
    }
    else{
      Serial.print("Publish gas message: ");
      Serial.println(1);
      client.publish("ESP/Gas",String(1).c_str());
    }
    if(rain_state == LOW && prev_rain_state == LOW){
      Serial.print("Publish rain message: ");
      Serial.println(1);
      client.publish("ESP/Rain",String(1).c_str());
    }else{
      Serial.print("Publish rain message: ");
      Serial.println(0);
      client.publish("ESP/Rain",String(0).c_str());

    }
  }
  delay(1000);
}







  
