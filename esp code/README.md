# ESP Microcontroller Firmware

This directory contains Arduino / C++ firmware for the ESP32 / ESP8266 microcontroller on the boat.

## Responsibilities
- Motor speed & direction control (PWM)
- Rudder servo angle positioning
- Sensor telemetry acquisition (GPS, IMU, battery voltage, water sensor)
- Communication bridge with Raspberry Pi over Serial / WebSocket / MQTT

## Files
- `esp_boat_controller.ino`: Main ESP firmware sketch
