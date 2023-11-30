# Sound Monitoring System (Note! Ongoing project, this is only a first draft of the readme)

This project implements a sound monitoring system using an ESP32 microcontroller. The system measures sound levels using a microphone sensor, categorizes the intensity into quiet, moderate, or loud, and publishes the averaged decibel levels to an AWS IoT (Internet of Things) endpoint.

## Features

- Real-time sound level monitoring.
- Categorization of sound levels into quiet, moderate, or loud.
- Publishing of averaged decibel levels to AWS IoT.

## Getting Started

### Prerequisites

- Arduino IDE
- ESP32 board support installed
- AWS IoT account

### Installation

1. Clone the repository.
2. Set up AWS IoT credentials in the `credentials.h` file.
3. Upload the code to your ESP32 board using the Arduino IDE.

## Usage

- Connect the ESP32 to a power source.
- Monitor sound levels via the AWS IoT console.

## Code Overview

- Libraries for WiFi, MQTT, and JSON handling.
- Constants for WiFi connection, AWS IoT topics, sensor pins, LED pins, and sampling windows.
- Functions for measuring decibel levels, controlling LEDs, updating averaged decibel levels, connecting to WiFi, connecting to AWS IoT, publishing messages, and handling incoming messages.

For detailed instructions and customization, blablabla...
