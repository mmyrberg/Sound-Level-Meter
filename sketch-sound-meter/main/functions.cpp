#include "header.h"

// Global variables for calculating average
float sumOfLevels = 0;
unsigned int numberOfSamples = 0;

// Create a secure WiFi connection (net) and an MQTT client (client) with a 256-byte buffer
WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

// Function to get decibel level from the microphone sensor
int getDecibel() {
  unsigned long startMillis = millis();
  unsigned long currentMillis = startMillis;
  unsigned int sample;
  unsigned int signalMax = 0;
  unsigned int signalMin = 4095;
  unsigned int peakToPeak = 0;
  
  // Collect sound samples over a specified time window (sampleWindow)
  while (currentMillis - startMillis <= sampleWindow) {
    currentMillis = millis();
    sample = analogRead(SENSOR_ANALOG_PIN);
      
      // Track the maximum and minimum values of the sound sample
      if (sample > signalMax) {
        signalMax = sample;
      } else if (sample < signalMin) {
        signalMin = sample;
      }
    }
  
  // Set peaktopeak value (represents the amplitude)
  peakToPeak = signalMax - signalMin;

  // scale the peakToPeak value from the analog readings range 150-335 to the range 28-100 (dB)
  int dB = map(peakToPeak, 150, 335, 28, 105);
  return dB;
}

// Function to control LEDs based on dB level
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

// Function to calculate and return the average decibel over a time window (sampleWindowAvg)
float calculateAverageDecibel(int dB) {
  unsigned long currentMillis = millis();
  static unsigned long lastPublishTime = 0;

  // Accumulate decibel values and count the number of samples
  sumOfLevels += dB;
  numberOfSamples++;

  // Check if the time window for averaging has elapsed and calculate the average
  if (currentMillis - lastPublishTime >= sampleWindowAvg) {
    float avgdB = sumOfLevels / numberOfSamples;
    lastPublishTime = currentMillis;
    sumOfLevels = 0;
    numberOfSamples = 0;
    return avgdB;
  }

  // Return 0 if the time window has not elapsed
  return 0;
}

// Function to connect to WiFi
void connectToWifi() {
  Serial.print("Connecting to WiFi");

  // Set the WiFi mode to station and initiate connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Record the start time for connection attempt timeout
  unsigned long startAttemptTime = millis();

  // Wait for connection or timeout
  while(WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
      Serial.print(".");
      delay(100);
  }

  // Handle connection success or failure
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Error: Failed to connect!");
    return;
  } else {
    Serial.print("Success! Connected at ");
    Serial.println(WiFi.localIP());
    delay(1000);
  }
}

// Function to connect to AWS
void connectToAWS() {
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Begin connection to AWS MQTT broker
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Set up message handler for incoming MQTT messages
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IoT...");

  // Attempt to connect to AWS IoT broker
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  // Handle connection success or failure
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  } else {
  // Subscribe to a specified topic on the AWS IoT broker
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  }
  
  Serial.println("AWS IoT Connected!");
}

// Function to publish a message to AWS
void publishMessage(float avgdB) {
  // Create a JSON document and serialize it into a string
  StaticJsonDocument<200> doc;
  doc["Avg decibel 20s"] = avgdB;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  // Publish the JSON-formatted message to the AWS IoT broker
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

  // Print published message to Serial Monitor
  Serial.print("Published to AWS: ");
  Serial.println(jsonBuffer);
  Serial.println("\t");
}

// Function to handle incoming MQTT messages
void messageHandler(String &topic, String &payload) {
  // Print details of the incoming MQTT message to Serial Monitor
  Serial.println("incoming: " + topic + " - " + payload);

  // Deserialize the JSON payload and extract the 'message' field
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char *message = doc["message"];

  // Print the extracted 'message' field to Serial Monitor
  Serial.println(message);
  Serial.println("\t");
}