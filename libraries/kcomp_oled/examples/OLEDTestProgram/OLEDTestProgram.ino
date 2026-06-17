/*
  K-COMP Diagnostic Suite: Standard Buffer (Non-Paged)
  Purpose: Verifies OLED hardware initialization, geometry rendering, 
           zero-cost layout macros, and hardware control signals.
*/

#include "kcomp_oled.h"

// State machine variables for autonomous testing
uint8_t currentTestPhase = 0;
unsigned long lastPhaseChange = 0;
const int PHASE_DURATION = 3000; // 3 seconds per test screen

void setup() {
  Serial.begin(115200);
  Serial.println("Booting Standard Display Diagnostic...");

  if (!initDisplay()) {
    Serial.println("FATAL: OLED hardware not responding.");
    while(1); // Halt the system if silicon is disconnected
  }
}

void loop() {
  // --- 1. STATE MANAGEMENT (LOGIC) ---
  if (millis() - lastPhaseChange > PHASE_DURATION) {
    currentTestPhase++;
    if (currentTestPhase > 3) currentTestPhase = 0;
    
    // Reset any hardware states before the next test
    invertDisplay(false);
    dimDisplay(false);
    
    lastPhaseChange = millis();
  }

  // --- 2. RENDERING LAYER ---
  clearDisplay(); // Wipe the 1024-byte shadow buffer

  switch (currentTestPhase) {
    case 0:
      // TEST: Zero-Cost Macros & Text Engine
      setCursor(OLED_ALIGN_CENTER_X(12, 1), 10);
      printText("K-COMP BOARD", OLED_COLOR_WHITE, 1);
      
      setCursor(OLED_ALIGN_CENTER_X(10, 2), 30);
      printText("DIAGNOSTIC", OLED_COLOR_WHITE, 2); // Size 2 text
      break;

    case 1:
      // TEST: Geometric Primitives & Absolute Coordinates
      setCursor(0, 0); printText("GEOMETRY", OLED_COLOR_WHITE, 1);
      
      drawLine(0, 15, 127, 15, OLED_COLOR_WHITE);        // Boundary line
      drawRect(5, 25, 20, 20, OLED_COLOR_WHITE);         // Hollow Box
      fillRect(35, 25, 20, 20, OLED_COLOR_WHITE);        // Solid Box
      drawCircle(80, 35, 10, OLED_COLOR_WHITE);          // Hollow Circle
      fillTriangle(105, 45, 115, 25, 125, 45, OLED_COLOR_WHITE); // Solid Triangle
      break;

    case 2:
      // TEST: High-Speed Line Engines
      setCursor(0, 0); printText("FAST LINES", OLED_COLOR_WHITE, 1);
      
      for (int i = 0; i < 128; i += 8) {
        drawFastVLine(i, 15, 49, OLED_COLOR_WHITE); // Picket fence
      }
      for (int i = 15; i < 64; i += 8) {
        drawFastHLine(0, i, 128, OLED_COLOR_WHITE); // Ladder
      }
      break;

    case 3:
      // TEST: Silicon-Level Hardware Commands (Invert & Dim)
      setCursor(OLED_ALIGN_CENTER_X(13, 1), OLED_ALIGN_CENTER_Y(1));
      
      // Blink the physical display properties using the system clock
      if (millis() % 1000 < 500) {
        invertDisplay(true);
        printText("INVERT TEST", OLED_COLOR_WHITE, 1);
      } else {
        invertDisplay(false);
        dimDisplay(true); // Drop voltage
        printText("DIM TEST", OLED_COLOR_WHITE, 1);
      }
      break;
  }

  updateDisplay(); // Blast the buffer to the physical screen
  delay(20);       // Brief rest to prevent I2C bus flooding
}