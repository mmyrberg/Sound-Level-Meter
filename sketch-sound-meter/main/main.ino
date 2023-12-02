#include "header.h"

void setup() {
  Serial.begin(115200);

  // Connect to Wifi and AWS
  connectToWifi();
  delay(1000);
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

  // Print current dB to Serial Monitor
  Serial.print("Decibel now: ");
  Serial.println(dB);

  // Control LEDs based on dB level
  controlLEDs(dB);
  
  // Send average dB (20s) to AWS
  float average = calculateAverageDecibel(dB);
  if (average > 0) {
    publishMessage(average);
  }

  client.loop();
}