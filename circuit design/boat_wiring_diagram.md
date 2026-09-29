# microclog_boat | Hardware Circuit Schematic & Wiring Guide

This document defines the physical hardware connections between the **ESP32 Microcontroller**, **Raspberry Pi Main System**, **L298N Motor Driver**, **Rudder Servo**, and **Sensors**.

---

## 1. Pin Mapping Table

| Component | Pin Function | ESP32 Pin | Raspberry Pi Pin | Notes |
| :--- | :--- | :--- | :--- | :--- |
| **L298N Motor Driver** | PWM (ENA) | GPIO 18 | - | Speed Control |
| | IN1 | GPIO 19 | - | Forward Direction |
| | IN2 | GPIO 21 | - | Reverse Direction |
| **Rudder Servo Motor** | Signal (PWM) | GPIO 22 | - | Steering Servo (30° to 150°) |
| **Ultrasonic (HC-SR04)**| Trig | GPIO 5 | - | Obstacle Detection |
| | Echo | GPIO 17 | - | Distance Echo Input |
| **Battery Divider** | ADC Sense | GPIO 34 | - | 12V LiPo Battery Monitoring |
| **Pi-to-ESP Communication**| RX / TX | GPIO 16 (RX) / 17 (TX) | GPIO 14 (TX) / 15 (RX) | UART Serial Bridge @ 115200 Baud |

---

## 2. Block Diagram

```text
       +--------------------+           +------------------------+
       |   LiPo Battery     |           |     Raspberry Pi       |
       |  (11.1V / 12.6V)   |           |    (Web Server/Log)    |
       +---------+----------+           +-----------+------------+
                 |                                  |
                 | (Power)                          | (Serial UART / USB)
                 v                                  v
       +---------+----------+           +-----------+------------+
       | L298N Motor Driver +----------->   ESP32 Microcontroller|
       +---------+----------+ (Control) +-----------+------------+
                 |                                  |
                 v                                  +---> Servo Rudder
           DC Thruster Motor                        +---> HC-SR04 Ultrasonic Sensor
                                                    +---> Voltage Sensor
```

---

## 3. Power Supply Notes
- **ESP32 & Raspberry Pi**: Powered via step-down 5V/3A UBEC from the main LiPo battery.
- **Common Ground**: Ensure a common ground wire connects the battery (-), ESP32 GND, L298N GND, and Raspberry Pi GND to prevent signal noise.
