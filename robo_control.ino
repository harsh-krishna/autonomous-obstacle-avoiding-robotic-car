/*
 * Project:  Autonomous Obstacle Avoiding  & Bluetooth-Controlled Robotic Car
 * Description: 4WD Obstacle Avoiding and Bluetooth Controlled Robot using Arduino Uno,
 *              L293D Motor Driver Shield, HC-SR04 Ultrasonic Sensor, and HC-05 BT Module.
 */

#include <AFMotor.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// DC Motors Initialization (L293D Shield Channels)
AF_DCMotor motor1(1); 
AF_DCMotor motor2(2); 
AF_DCMotor motor3(3); 
AF_DCMotor motor4(4); 

// Ultrasonic Sensor Pins
#define trigPin A0
#define echoPin A1

// Bluetooth Serial Communication (TX -> A2, RX -> A3)
SoftwareSerial BT(A2, A3); 

// Servo Motor Configuration
Servo myServo;
#define servoPin 9 

// Global Variables
char command = 'S';
bool isAutoMode = false; 

void setup() {
  Serial.begin(9600);
  BT.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  myServo.attach(servoPin);
  myServo.write(90); // Center position
  
  setMotorSpeed(160);
  Serial.println("System Initialized: Ready.");
}

void loop() {
  // Read distance from ultrasonic sensor
  int distance = getDistance();

  // Read incoming Bluetooth commands
  if (BT.available()) {
    command = BT.read();
    
    // Mode Switching Logic
    if (command == 'A' || command == 'a') {
      isAutoMode = true;
      Serial.println("Active Mode: AUTONOMOUS");
    } 
    else if (command == 'M' || command == 'm' || command == 'S' || command == 's') {
      isAutoMode = false;
      stopMotors();
      Serial.println("Active Mode: MANUAL");
    }
  }

  // Execute selected mode
  if (isAutoMode) {
    runFullAutoMode(distance);
  } else {
    runSafeManualMode(distance);
  }
}

// ==========================================
// 1. SAFE MANUAL MODE (Anti-Collision System)
// ==========================================
void runSafeManualMode(int currentDistance) {
  if (command == 'F' || command == 'f') {
    // Only proceed forward if clear path detected
    if (currentDistance > 20 || currentDistance == 0) {
      moveForward();
    } else {
      stopMotors(); // Emergency stop on obstacle detection
      Serial.println("Collision Alert: Auto Brake Engaged!");
    }
  } 
  else if (command == 'B' || command == 'b') moveBackward();
  else if (command == 'L' || command == 'l') turnLeft();
  else if (command == 'R' || command == 'r') turnRight();
  else if (command == 'S' || command == 's') stopMotors();
}

// ==========================================
// 2. FULLY AUTONOMOUS MODE (Self-Driving)
// ==========================================
void runFullAutoMode(int currentDistance) {
  if (currentDistance > 0 && currentDistance < 25) {
    stopMotors();
    delay(200);
    moveBackward();
    delay(400);
    stopMotors();
    
    // Scan Left Path
    myServo.write(10); 
    delay(500);
    int distLeft = getDistance();

    // Scan Right Path
    myServo.write(170); 
    delay(500);
    int distRight = getDistance();

    // Recenter Servo
    myServo.write(90); 
    delay(300);

    // Decision Logic: Move toward the clearer side
    if (distLeft > distRight) {
      turnLeft();
    } else {
      turnRight();
    }
    
    delay(700);
    stopMotors();
  } else {
    moveForward();
  }
}

// ==========================================
// HARDWARE DRIVER & SENSOR FUNCTIONS
// ==========================================

// Calculates distance in centimeters via HC-SR04
int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 25000); // 25ms timeout
  if (duration == 0) return 0; 
  return duration * 0.034 / 2;
}

// Sets uniform speed for all 4 DC Motors
void setMotorSpeed(int speed) {
  motor1.setSpeed(speed);
  motor2.setSpeed(speed);
  motor3.setSpeed(speed);
  motor4.setSpeed(speed);
}

void moveForward() {
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void moveBackward() {
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void turnLeft() {
  setMotorSpeed(200);
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
  setMotorSpeed(160);
}

void turnRight() {
  setMotorSpeed(200);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
  setMotorSpeed(160);
}

void stopMotors() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}
