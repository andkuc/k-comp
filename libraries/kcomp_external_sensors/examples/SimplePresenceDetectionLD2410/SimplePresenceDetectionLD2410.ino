#include "kcomp_LD2410.h"

// WIRING: Sensor TX -> Pin 6, Sensor RX -> Pin 7
#define RX_PIN 6
#define TX_PIN 7

void setup() {
  Serial.begin(115200);
  Serial.println("LD2410C Radar Test");

  // Init at 9600 (Assuming Setup Tool was run)
  if (!initLD2410(RX_PIN, TX_PIN, 9600)) {
    Serial.println("❌ Sensor not found.");
    Serial.println("1. Check Wiring (TX->2, RX->3)");
    Serial.println("2. Did you run the 'Setup Tool'?");
    while(1);
  }
  Serial.println("✅ Sensor Connected!");
}

void loop() {
  if (updateLD2410()) {
    
    if (isPresenceDetected()) {
      Serial.print("Target Distance: ");
      Serial.print(getDetectionDistance());
      Serial.print(" cm");

      if (isMoving()) Serial.print(" [Moving]");
      if (isStationary()) Serial.print(" [Static]");
      
      Serial.println();
    }
  }
}