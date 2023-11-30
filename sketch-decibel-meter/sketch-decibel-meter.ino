// Libraries
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "credentials.h"

// Constants
#define WIFI_TIMEOUT_MS 2000
#define AWS_IOT_PUBLISH_TOPIC "esp32/pub" // The MQTT topics that this device should publish/subscribe
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
#define SENSOR_ANALOG_PIN 35 // Define the analog pin to which the microphone sensor is connected (pin 27).
#define PIN_QUIET 32 // Define output pins for LEDs indicating different sound levels (pins 32, 33, 25, respectively).
#define PIN_MODERATE 33
#define PIN_LOUD 25 
const int sampleWindow = 1000; // Set the duration (in milliseconds) for collecting sound samples
const int sampleWindowAvg = 10000; // Set the duration (in milliseconds) for collecting the average of the sound samples

// Declare static variables for averaging decibel value
static float sumOfLevels = 0;
static unsigned int numberOfSamples = 0;

// WiFi and MQTT clients
WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

// Function declarations
int getDecibel();
void controlLEDs(int dB);
float calculateAverageDecibel(int dB);
void connectToWifi();
void connectToAWS();
void publishMessage(float avgdB);
void messageHandler(String &topic, String &payload);

void setup() {
  Serial.begin(115200);
  connectToWifi();
  delay(2000);  // Add a delay to ensure that WiFi connection is established before attempting AWS connection
  connectToAWS();

  // Set the sensor pin and LED pins as input and output, respectively
  pinMode(SENSOR_ANALOG_PIN, INPUT);
  pinMode(PIN_QUIET, OUTPUT);
  pinMode(PIN_MODERATE, OUTPUT);
  pinMode(PIN_LOUD, OUTPUT);
  
  // Turns off all LEDs initially
  digitalWrite(PIN_QUIET, LOW);
  digitalWrite(PIN_MODERATE, LOW);
  digitalWrite(PIN_LOUD, LOW);
}

void loop() {
  int dB = getDecibel();

  // Print instantaneous dB to Serial Monitor
  Serial.print("Decibel now: ");
  Serial.println(dB);

  controlLEDs(dB);
  
  float average = calculateAverageDecibel(dB);
  if (average > 0) {
    publishMessage(average);
  }
  
  client.loop();
}

int getDecibel() {
  unsigned long startMillis = millis();
  unsigned long currentMillis = startMillis;
  unsigned int sample;
  unsigned int signalMax = 0;
  unsigned int signalMin = 4095;
  float peakToPeak = 0;
  
  // Collect sound samples over a specified time window (sampleWindow)
  while (currentMillis - startMillis <= sampleWindow) {
    currentMillis = millis(); // Update currentMillis inside the loop
    sample = analogRead(SENSOR_ANALOG_PIN); // // Read analog value from the microphone sensor
      
      if (sample > signalMax) {
        signalMax = sample; // Update the maximum value of the sound sample
      } else if (sample < signalMin) {
        signalMin = sample; // Update the minimum value of the sound sample
      }
    }
  
  // set peaktopeak value (represents the amplitude)
  peakToPeak = signalMax - signalMin;
  // scale the peakToPeak value from the analog readings range 150-335 to the range 28-100 (dB)
  int dB = map(peakToPeak, 150, 335, 28, 100);
  return dB;
}

void controlLEDs(int dB) {
  if (dB >= 30 && dB < 50) {
    Serial.println("Quiet");
    digitalWrite(PIN_QUIET, HIGH);
    digitalWrite(PIN_MODERATE, LOW);
    digitalWrite(PIN_LOUD, LOW);
  } else if (dB >= 50 && dB < 80) {
    Serial.println("Moderate");
    digitalWrite(PIN_QUIET, LOW);
    digitalWrite(PIN_MODERATE, HIGH);
    digitalWrite(PIN_LOUD, LOW);
  } else if (dB >= 80 && dB <= 100) {
    Serial.println("Loud");
    digitalWrite(PIN_QUIET, LOW);
    digitalWrite(PIN_MODERATE, LOW);
    digitalWrite(PIN_LOUD, HIGH);
  } else {
    Serial.println("Off");
    digitalWrite(PIN_QUIET, LOW);
    digitalWrite(PIN_MODERATE, LOW);
    digitalWrite(PIN_LOUD, LOW);
  }
  Serial.println("\t");
}

float calculateAverageDecibel(int dB) {
  unsigned long currentMillis = millis();
  static unsigned long lastPublishTime = 0;

  sumOfLevels += dB;
  numberOfSamples++;

  if (currentMillis - lastPublishTime >= sampleWindowAvg) {
    float avgdB = sumOfLevels / numberOfSamples;
    lastPublishTime = currentMillis;
    sumOfLevels = 0;
    numberOfSamples = 0;
    return avgdB;
  }
  return 0;
}

void connectToWifi() {
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();

  while(WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
      Serial.print(".");
      delay(100);
  }
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Error: Failed to connect!");
    return;
  } else {
    Serial.print("Success! Connected at ");
    Serial.println(WiFi.localIP());
    delay(1000);
  }
}

void connectToAWS() {
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage(float avgdB) {
  StaticJsonDocument<200> doc;
  doc["Average decibel level"] = avgdB;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);  // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

  Serial.print("Published to AWS: ");
  Serial.println(jsonBuffer);
  Serial.println("\t");
}

void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char *message = doc["message"];

  Serial.println(message);
  Serial.println("\t");
}