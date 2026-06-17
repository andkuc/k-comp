/*
 * kcomp_Servo_Example.ino
 * Demonstrates the difference between controlling Angle vs Speed.
 */

#include "kcomp_servoSG90.h"

// Hardware Configuration
#define PIN_SERVO0     6
// #define PIN_SERVO1  9

// IDs for our library
#define ID_SERVO0      0
//#define ID_SERVO1    1

void setup() {
  Serial.begin(115200);
  Serial.println("--- SG90 Servo Test ---");

  // 1. Initialize the Robot Arm (Standard 180 Servo)
  initServo(ID_SERVO0, PIN_SERVO0, SERVO_TYPE_180);

  // 2. Initialize the Robot Arm (360 Servo)
  // initServo(ID_SERVO1, PIN_SERVO1, SERVO_TYPE_360);

  Serial.println("System Ready.");
}

void loop() {
  // --- PART 1: The Robot Arm (Using Angles) ---
  // Notice we use 'setServoAngle' because this is a 180-degree motor.
  
  Serial.println("Servo0 -> 0 degrees");
  setServoAngle(ID_SERVO0, 0); 
  delay(1000);

  Serial.println("Servo0 -> 90 degrees");
  setServoAngle(ID_SERVO0, 90); 
  delay(1000);

  // --- PART 2: The Servo1 (Using Speed) ---
  // Notice we use 'setServoSpeed' because this is a 360-degree motor.
  
  /*Serial.println("Servo1 -> Forward 50% Speed");
  setServoSpeed(ID_SERVO1, 50);
  delay(2000);

  Serial.println("Servo1 -> STOP");
  stopServo(ID_SERVO1);
  delay(1000);

  Serial.println("Servo1 -> Backward 100% Speed");
  setServoSpeed(ID_SERVO1, -100);
  delay(2000);
  
  stopServo(ID_SERVO1);
  delay(1000);
  */
}