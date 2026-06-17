#include "kcomp_LD2450C.h"

// --- Wiring ---
// Connect Arduino Pin 6 to the Radar's TX pin
// Connect Arduino Pin 7 to the Radar's RX pin
const uint8_t RADAR_RX_PIN = 6;
const uint8_t RADAR_TX_PIN = 7;

// IMPORTANT: An Arduino Uno using SoftwareSerial cannot handle the default 256000 baud!
// You MUST use the HLKRadarTool app on your phone to change the sensor's 
// baud rate to 38400 before running this code.
const int RADAR_BAUD_RATE = 9600;

// Let's define a "Danger Zone" in front of the sensor.
// X: -500mm (Left) to 500mm (Right)
// Y: 0mm (Sensor face) to 1500mm (1.5 meters away)
const int ZONE_MIN_X = -500;
const int ZONE_MAX_X = 500;
const int ZONE_MIN_Y = 0;
const int ZONE_MAX_Y = 1500;

void setup() {
  // Start the serial monitor so we can see the results on the computer
  Serial.begin(115200);
  Serial.println("--- K-Comp LD2450 Radar System Starting ---");

  // Initialize the radar
  if (initLD2450(RADAR_RX_PIN, RADAR_TX_PIN, RADAR_BAUD_RATE)) {
    Serial.println("SUCCESS: Radar detected and communicating!");
  } else {
    Serial.println("ERROR: Could not communicate with Radar.");
  }
}

void loop() {
  // updateLD2450() must be called as fast as possible in the loop.
  // It returns 'true' only when a completely new frame of data is ready.
  if (updateLD2450()) {
    
    // The LD2450 can track 3 targets. Let's just focus on Target 0 (the primary target).
    if (isTargetActive(0)) {
      
      // 1. Get the raw coordinates
      int x = getTargetX(0);
      int y = getTargetY(0);
      
      // 2. Get the student-friendly advanced data
      int distance = getTargetTrueDistance(0);
      int angle = getTargetAngle(0);
      
      // 3. Print the tracking info to the Serial Monitor
      Serial.print("Target 1 is (");
      Serial.print(x);
      Serial.print(",");
      Serial.print(y);
      Serial.print(") ");
      Serial.print(distance);
      Serial.print("mm away at an angle of ");
      Serial.print(angle);
      Serial.print(" degrees. ");

      // 4. Use the movement helpers
      if (isTargetApproaching(0)) {
        Serial.print("[Walking Towards Us] ");
      } else if (isTargetDeparting(0)) {
        Serial.print("[Walking Away] ");
      } else {
        Serial.print("[Standing Still] ");
      }

      // 5. Use the Zone helper to trigger logic
      if (isTargetInZone(0, ZONE_MIN_X, ZONE_MAX_X, ZONE_MIN_Y, ZONE_MAX_Y)) {
        Serial.println("--> ALERT! IN DANGER ZONE! <--");
        // Students could add code here to turn on an LED, sound a buzzer, 
        // or close a servo-motor door!
      } else {
        Serial.println("--> Safe. <--");
      }
      
    }
  }
}