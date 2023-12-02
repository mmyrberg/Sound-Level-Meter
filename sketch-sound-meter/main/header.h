#ifndef HEADER_H
#define HEADER_H

// Libraries
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "credentials.h"

// Macros and constants
#define THINGNAME "esp32-board"
#define WIFI_TIMEOUT_MS 2000
#define AWS_IOT_PUBLISH_TOPIC "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
#define SENSOR_ANALOG_PIN 35
#define PIN_QUIET 32
#define PIN_MODERATE 33
#define PIN_LOUD 25
const int sampleWindow = 2000;
const int sampleWindowAvg = 20000;

// Variables for averaging decibel value
extern float sumOfLevels;
extern unsigned int numberOfSamples;

// WiFi and MQTT clients
extern WiFiClientSecure net;
extern MQTTClient client;

// Function declarations
int getDecibel();
void controlLEDs(int dB);
float calculateAverageDecibel(int dB);
void connectToWifi();
void connectToAWS();
void publishMessage(float avgdB);
void messageHandler(String &topic, String &payload);

#endif