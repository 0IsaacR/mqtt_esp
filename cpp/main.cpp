//-------------------------
// Title: main.cpp
//-------------------------
// Program Details:
//-------------------------
// Purpose: This program runs on the esp8266. It subscribes and publishes data to the broker. 
//           It also controls LEDs depending on the data sent.

// Inputs:  mqtt subscription, buttons
// Outputs: mqtt publisher, LED
// Date:  12/4/25
// Author:  Isaac Rodriguez
// Versions:
//            V1 - passes voltage data and control to broker
//-------------------------
// File Dependancies:
      // #include <ESP8266WiFi.h>
      // #include <ESP8266HTTPClient.h>
      // #include "Arduino.h"
      // #include <WiFiClientSecure.h>
      // #include <PubSubClient.h>
//-------------------------
// Main Program
//-------------------------
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "Arduino.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* HIVEMQ_HOST = ""; 
const int   MQTT_PORT = 8883;

const char* HIVEMQ_USER = "";
const char* HIVEMQ_PASS = "";

const char* pub_voltage   = "testtopic/temp/outTopic/voltage";
const char* sub_led    = "testtopic/temp/inTopic";         
const char* pub_button  = "testtopic/temp/outTopic/button";

#define LED_BLUE 16
#define LED_GREEN 5
#define BUTTON 10
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];

WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

int buttonState = LOW;
int voltageState = LOW;
unsigned long buttonReset = 0;
unsigned long voltageReset = 0;
unsigned long charReset = 0;
unsigned long serialWait = 1000;
float tempVoltage = 0;

// const char* ssid     = "guest"; 
// const char* password = "3232039599";

const char* ssid     = "Alio"; 
const char* password = "STS@5793!";

bool wifiStatus = false;

void callback(char* topic, byte* payload, unsigned int length) 
{
  if (payload[0] == 49)
    {
      Serial.print("MESSAGE - 1: Turn ON LED");
      digitalWrite(LED_GREEN, HIGH);
    }
  if (payload[0] == 48)
    {
      Serial.print("MESSAGE - 0: Turn OFF LED");
      digitalWrite(LED_GREEN, LOW);
    }
}

void reconnectMQTT() 
{
  //Loop until we successfully connect
  while (!mqttClient.connected()) 
  {
    //Attempt connection with username/password authentication
    if (!mqttClient.connect("ESP32Client", HIVEMQ_USER, HIVEMQ_PASS)) 
      {
        delay(5000);
      }
  }
}

void initMQTT() 
{
  mqttClient.setServer(HIVEMQ_HOST, MQTT_PORT);
  mqttClient.setCallback(callback);

  secureClient.setInsecure();

  secureClient.setTimeout(20000);

  mqttClient.connect("ESP32Client", HIVEMQ_USER, HIVEMQ_PASS);
  mqttClient.subscribe(sub_led,0);
}

void publishVoltage(float voltage) 
{
  mqttClient.publish(pub_voltage, String(voltage, 2).c_str());
}

void publishZero()
{
  mqttClient.publish(pub_button, "0"); //getLightLevel().c_str()
}

void publishOne()
{
  mqttClient.publish(pub_button, "1"); //getLightLevel().c_str()
}

void handleMQTT() 
{
  //If connection dropped, reconnect
  if (!mqttClient.connected()) 
  {
    reconnectMQTT();
  }
  //Processes incoming messages and keeps alive with broker
  mqttClient.loop();
}



void setup() {
  Serial.begin(9600);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  // pinMode(RESET_BUTTON, INPUT_PULLUP);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);

  WiFi.mode(WIFI_STA); // SETS TO STATION MODE!
  //WiFi.disconnect();
  Serial.println();
  Serial.println();
  WiFi.hostname("ESP-host");
  delay(100);
  Serial.println("Connecting to WiFi");
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".+.");
  }
  Serial.println(WiFi.getHostname());

  initMQTT();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) 
    {
      wifiStatus = true;
      digitalWrite(LED_BLUE, HIGH);
    } 
  else 
    {
      wifiStatus = false;
      digitalWrite(LED_BLUE, LOW);
    }

   handleMQTT();

  // potentiometer
  delay(5);
  int potentiometer = analogRead(A0);
  float voltage = potentiometer * (3.3 / 1023.0);
  if ((voltage < tempVoltage - 0.03)|(voltage > tempVoltage + 0.03))
  {
    Serial.print("Current Voltage: ");
    Serial.print(voltage);
    Serial.println("");
    tempVoltage = voltage;
    voltageState = HIGH;
  }


  if (Serial.available()) // Check if data is available to read
    { 
      Serial.println("Entered 1");
      int num = Serial.parseInt(); // Read the incoming character
      if  (num == 1)
      {
        digitalWrite(LED_GREEN, HIGH);
      }
    }
    

  if (!digitalRead(BUTTON))  // print 1 when button is pressed
    {
      delay(80);
      if (!digitalRead(BUTTON))
      {
        while(!digitalRead(BUTTON))
          {
            // just wait for button to release
          }
        buttonState = HIGH;
        Serial.println("SWITCH:   1");
        publishOne();
        buttonReset = millis();
      }
    }
  if (millis() - voltageReset > 15000) // sends voltage every 15 seconds
    {  
      if (voltageState == HIGH)
        {
          publishVoltage(voltage);
          voltageState = LOW;
          voltageReset = millis();
        }
    }
  if (millis() - buttonReset > 5000) // sends button reset after 5 seconds
    {  
      if (buttonState == HIGH)
        {
          publishZero();
          Serial.println("SWITCH:   0");
          buttonState = LOW;
          digitalWrite(LED_GREEN, LOW);
          buttonReset = millis();
        }
    }
}
