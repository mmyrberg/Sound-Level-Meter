#include "header.h"

void setup() {
  Serial.begin(115200);
  connectToWifi();
  delay(1000);  // Add a delay to ensure that WiFi connection is established before attempting AWS connection
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
  
  // Send average dB (10s) to AWS
  float average = calculateAverageDecibel(dB);
  if (average > 0) {
    publishMessage(average);
  }

  client.loop();
}