# IoT Based Accident Detection and Vehicle Monitoring System

## Overview
## 📌 Project Overview

Road accidents are one of the major causes of injuries and deaths worldwide. In many cases, medical assistance is delayed because nobody reports the accident immediately.

This project automatically detects vehicle accidents using an ADXL345 accelerometer connected to an ESP32. Once an accident is detected, the system retrieves the vehicle's GPS location and uploads the data to ThingSpeak Cloud.

The system enables faster emergency response and continuous vehicle monitoring.

## Features
- Accident detection
- GPS location tracking
- ThingSpeak cloud monitoring
- Web server dashboard
- LCD and buzzer alerts

## 🔧 Hardware Components

| Component | Quantity |
|-----------|----------|
| ESP32 Dev Board | 1 |
| ADXL345 Accelerometer | 1 |
| GPS Module (Neo-6M) | 1 |
| Breadboard | 1 |
| Jumper Wires | Several |
| USB Cable | 1 |
|vibration sensor | 1 |
## 💻 Software Used

Arduino IDE

ThingSpeak

ESP32 Board Package

TinyGPS++ Library

Adafruit ADXL345 Library

WiFi Library
HTML

## ⚙ Working Principle

Step 1:
ESP32 continuously reads vibration sensor value and acceleration values from the ADXL345 sensor.

Step 2:
If the acceleration and vibration exceeds the accident threshold, an accident is detected.

Step 3:
ESP32 reads the current latitude and longitude from the GPS module.

Step 4:
Vehicle location is uploaded to ThingSpeak.

Step 5:
The web server displays the accident status and live sensor readings.

Step 6:
The monitoring dashboard updates automatically.
