/*
  K-COMP Diagnostic Suite: Paged Architecture
  Purpose: Verifies the 8-slice rendering loop, proving that complex 
           geometry can be drawn seamlessly across memory band boundaries.
*/

#include "kcomp_oled_paged.h"

// State machine variables MUST be evaluated outside the page loop
uint8_t currentTestPhase = 0;
unsigned long lastPhaseChange = 0;
const int PHASE_DURATION = 3000; 

// Dynamic animation variable
int movingX = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting Paged Display Diagnostic...");

  if (!initDisplay()) {
    Serial.println("FATAL: OLED hardware not responding.");
    while(1);
  }
}

void loop() {
  // --- 1. STATE MANAGEMENT (LOGIC LAYER) ---
  // We evaluate time and update variables exactly ONCE per frame.
  if (millis() - lastPhaseChange > PHASE_DURATION) {
    currentTestPhase++;
    if (currentTestPhase > 3) currentTestPhase = 0;
    
    invertDisplay(false);
    dimDisplay(false);
    lastPhaseChange = millis();
  }

  movingX++;
  if (movingX > 127) movingX = 0;

  // --- 2. HARDWARE RENDERING LAYER (THE PAGED LOOP) ---
  // The system will run this block 8 times to construct the physical screen.
  // Variables like 'movingX' are read, but NEVER modified inside this loop.
  
  firstPage();
  do {
    
    switch (currentTestPhase) {
      case 0:
        setCursor(OLED_ALIGN_CENTER_X(12, 1), 10);
        printText("PAGED SYSTEM", OLED_COLOR_WHITE, 1);
        
        setCursor(OLED_ALIGN_CENTER_X(10, 2), 30);
        printText("DIAGNOSTIC", OLED_COLOR_WHITE, 2);
        
        // A moving pixel proves the loop is executing cleanly across frames
        drawPixel(movingX, 55, OLED_COLOR_WHITE); 
        break;

      case 1:
        // Drawing a circle across the middle proves that geometry 
        // renders correctly even when sliced across multiple memory pages.
        setCursor(0, 0); printText("PAGE BOUNDARY", OLED_COLOR_WHITE, 1);
        
        drawLine(0, 15, 127, 15, OLED_COLOR_WHITE);        
        drawRect(5, 25, 20, 20, OLED_COLOR_WHITE);         
        fillRect(35, 25, 20, 20, OLED_COLOR_WHITE);        
        drawCircle(80, 35, 10, OLED_COLOR_WHITE);          
        fillTriangle(105, 45, 115, 25, 125, 45, OLED_COLOR_WHITE); 
        break;

      case 2:
        setCursor(0, 0); printText("FAST LINES", OLED_COLOR_WHITE, 1);
        for (int i = 0; i < 128; i += 8) {
          drawFastVLine(i, 15, 49, OLED_COLOR_WHITE); 
        }
        for (int i = 15; i < 64; i += 8) {
          drawFastHLine(0, i, 128, OLED_COLOR_WHITE); 
        }
        break;

      case 3:
        setCursor(OLED_ALIGN_CENTER_X(13, 1), OLED_ALIGN_CENTER_Y(1));
        
        if (millis() % 1000 < 500) {
          invertDisplay(true);
          printText("INVERT TEST", OLED_COLOR_WHITE, 1);
        } else {
          invertDisplay(false);
          dimDisplay(true); 
          printText("DIM TEST", OLED_COLOR_WHITE, 1);
        }
        break;
    }
    
  } while (nextPage());
  
  // No delay needed here; the 8-page loop inherently throttles the framerate
}