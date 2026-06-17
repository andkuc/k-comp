#include "kcomp_VibrationMotor.h"

#define VIB_PIN 6 // Must be a PWM capable pin

void setup() {
  Serial.begin(115200);
  initVibration(VIB_PIN);
  
  Serial.println("--- Haptic Engine Ready ---");
  Serial.println("Send '1' for a short click.");
  Serial.println("Send '2' for a long, heavy rumble.");
  Serial.println("Send '3' to turn ON permanently at 50% power.");
  Serial.println("Send '0' to stop all vibration.");
}

void loop() {
  // CRITICAL: Keep the state machine running in the background
  pollVibration();

  if (Serial.available() > 0) {
    char cmd = Serial.read();
    while(Serial.available()) Serial.read(); // Clear the serial buffer

    if (cmd == '1') {
      Serial.println("Short Click (Max Power, 200ms)");
      vibratePulseNonBlocking(255, 200); 
    }
    
    else if (cmd == '2') {
      Serial.println("Heavy Rumble (Power 150, 800ms)");
      vibratePulseNonBlocking(150, 800);
    }

    else if (cmd == '3') {
      Serial.println("Permanent ON (Power 127)");
      setVibration(127); // 127 is exactly half of 255
    }

    else if (cmd == '0') {
      Serial.println("Stop vibration.");
      stopVibration();
    }
  }
}