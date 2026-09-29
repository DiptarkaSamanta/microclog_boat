/*
 * microclog_boat - ESP32 / ESP8266 Firmware Controller
 * ----------------------------------------------------
 * Handles motor control (thrust & rudder), telemetry sensors (IMU, Ultrasonic, Battery),
 * and Serial / WebSockets communication bridge with Raspberry Pi.
 */

#include <Arduino.h>

#ifdef ESP32
  #include <WiFi.h>
  #include <ESP32Servo.h>
#else
  #include <ESP8266WiFi.h>
  #include <Servo.h>
#endif

// --- Pin Definitions ---
#define MOTOR_PWM_PIN    18   // Main propeller motor PWM speed control
#define MOTOR_DIR_PIN1   19   // Motor Direction A
#define MOTOR_DIR_PIN2   21   // Motor Direction B
#define RUDDER_SERVO_PIN 22   // Steering rudder servo motor
#define TRIG_PIN         5    // Ultrasonic distance sensor Trigger
#define ECHO_PIN         17   // Ultrasonic distance sensor Echo
#define BATTERY_ADC_PIN  34   // Battery voltage sense pin (ADC)

// --- Global Objects & Variables ---
Servo rudderServo;
int motorSpeed = 0;           // Speed: 0 to 255
int rudderAngle = 90;         // Steering: 0 (Left), 90 (Center), 180 (Right)
unsigned long lastTelemetryTime = 0;
const unsigned long TELEMETRY_INTERVAL = 500; // 500ms telemetry interval

// --- Function Declarations ---
void setupHardware();
void setMotorThrust(int speed, bool forward);
void setRudderAngle(int angle);
float getObstacleDistance();
float readBatteryVoltage();
void sendTelemetry();
void processIncomingSerialCommand();

void setup() {
  Serial.begin(115200);
  setupHardware();
  Serial.println(F("{\"status\":\"INITIALIZED\",\"device\":\"ESP32_BOAT_CONTROLLER\"}"));
}

void loop() {
  // Read and process commands sent from Raspberry Pi over Serial
  if (Serial.available() > 0) {
    processIncomingSerialCommand();
  }

  // Periodic Telemetry Transmission to Raspberry Pi
  if (millis() - lastTelemetryTime >= TELEMETRY_INTERVAL) {
    lastTelemetryTime = millis();
    sendTelemetry();
  }
}

void setupHardware() {
  pinMode(MOTOR_PWM_PIN, OUTPUT);
  pinMode(MOTOR_DIR_PIN1, OUTPUT);
  pinMode(MOTOR_DIR_PIN2, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  rudderServo.attach(RUDDER_SERVO_PIN);
  rudderServo.write(rudderAngle); // Center rudder
  setMotorThrust(0, true);        // Stop motors initially
}

void setMotorThrust(int speed, bool forward) {
  motorSpeed = constrain(speed, 0, 255);
  digitalWrite(MOTOR_DIR_PIN1, forward ? HIGH : LOW);
  digitalWrite(MOTOR_DIR_PIN2, forward ? LOW : HIGH);
  analogWrite(MOTOR_PWM_PIN, motorSpeed);
}

void setRudderAngle(int angle) {
  rudderAngle = constrain(angle, 30, 150); // Safe rudder limit
  rudderServo.write(rudderAngle);
}

float getObstacleDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (duration == 0) return 999.0;
  return (duration * 0.0343) / 2.0; // Distance in cm
}

float readBatteryVoltage() {
  int raw = analogRead(BATTERY_ADC_PIN);
  return (raw / 4095.0) * 3.3 * 4.0; // Voltage divider calibration multiplier
}

void sendTelemetry() {
  float distance = getObstacleDistance();
  float battery = readBatteryVoltage();

  // Transmit JSON formatted telemetry to Raspberry Pi
  Serial.print(F("{\"type\":\"telemetry\",\"thrust\":"));
  Serial.print(motorSpeed);
  Serial.print(F(",\"rudder\":"));
  Serial.print(rudderAngle);
  Serial.print(F(",\"distance_cm\":"));
  Serial.print(distance, 1);
  Serial.print(F(",\"battery_v\":"));
  Serial.print(battery, 2);
  Serial.println(F("}"));
}

void processIncomingSerialCommand() {
  String input = Serial.readStringUntil('\n');
  input.trim();
  
  if (input.startsWith("THRUST:")) {
    int val = input.substring(7).toInt();
    setMotorThrust(abs(val), val >= 0);
  } else if (input.startsWith("STEER:")) {
    int angle = input.substring(6).toInt();
    setRudderAngle(angle);
  } else if (input == "STOP") {
    setMotorThrust(0, true);
    setRudderAngle(90);
  }
}
