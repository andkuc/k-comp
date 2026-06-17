#include "kcomp_TCS34725.h"

// ========================================================
// HELPER FUNCTION: Wait for user input
// ========================================================
// This function traps the K-Comp in a loop until the user types 'c'.
// We keep calling pollColorSensor() inside it so the sensor stays alive.
void waitForCalibrationInput() {
  while (true) {
    pollColorSensor(); 
    if (Serial.available() > 0) {
      char cmd = Serial.read();
      while(Serial.available()) Serial.read(); // Clear buffer
      if (cmd == 'c') return; // Exit the loop when 'c' is received
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  if (!initColorSensor()) {
    Serial.println("Error: Sensor offline.");
    while(1); 
  }
  
  setColorIntegrationTime(COLOR_TIME_101MS);
  
  // ========================================================
  // AUTOMATIC INTERACTIVE CALIBRATION
  // ========================================================
  Serial.println("--- Boot Calibration Sequence ---");
  
  // Step 1: Calibrate Black (The Noise Floor)
  Serial.println("1. Place the sensor on a BLACK object and send 'c'.");
  waitForCalibrationInput();
  
  uint16_t rawBlack = getClear();
  uint16_t safeBlackThreshold = rawBlack + 50; // Add safety margin
  Serial.print("-> Black threshold set to: "); 
  Serial.println(safeBlackThreshold);

  // Step 2: Calibrate White (The Glare Limit)
  Serial.println("2. Place the sensor on a pure WHITE object and send 'c'.");
  waitForCalibrationInput();

  uint16_t rawWhite = getClear();
  uint16_t safeWhiteThreshold = rawWhite - 500; // Subtract safety margin
  Serial.print("-> White threshold set to: "); Serial.println(safeWhiteThreshold);

  // Apply the dynamically calculated thresholds to the library's AI Engine
  calibrateColorEngine(safeBlackThreshold, safeWhiteThreshold); 
  
  Serial.println("=================================");
  Serial.println("Type 's' to scan any object.");
}

void loop() {
  pollColorSensor();

  if (Serial.available() > 0) {
    char cmd = Serial.read();
    while(Serial.available()) Serial.read(); 
    
    if (cmd == 's') {
      // 1. Fetch raw data
      uint16_t r = getRed();
      uint16_t g = getGreen();
      uint16_t b = getBlue();
      uint16_t lux = getLux();

      Serial.println("\n--- Scan Results ---");
      Serial.print("Raw RGB -> R: "); Serial.print(r);
      Serial.print(" | G: "); Serial.print(g);
      Serial.print(" | B: "); Serial.println(b);
      
      Serial.print("Brightness: "); 
      Serial.print(lux); Serial.println(" Lux");

      // 2. Run the AI Engine to get the 1-Byte Color ID
      uint8_t myColor = estimateColor();
      
      Serial.print(">> Estimation: ");
      Serial.println(getColorName(myColor));
   }
  }
}