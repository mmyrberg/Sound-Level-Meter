#include "header.h"

float sumOfLevels = 0;
unsigned int numberOfSamples = 0;

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

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
  int dB = map(peakToPeak, 150, 335, 28, 105);
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