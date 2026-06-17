#include <Arduino.h>
#include "kcomp_oled_paged.h"
#include "kcomp_button_select.h"
#include "kcomp_button_start.h"
#include "kcomp_buzzer.h"
#include "kcomp_distance.h"
#include "kcomp_led.h"
#include "kcomp_potentiometer.h"
#include "kcomp_joystick.h"
#include "kcomp_AHT10.h"
#include "kcomp_GYBMI160.h"
#include "kcomp_MAX30102.h"

// ==============================================================================
// 🎨 BITMAP ASSETS
// ==============================================================================
// A 32x32 Pixel stylized "K" inside a Diamond.
const uint8_t PROGMEM logo_bitmap_32x32[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 
  0x00, 0x0f, 0xf0, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0xff, 0xff, 0x00, 0x03, 0xc3, 0xc3, 0xc0, 
  0x0f, 0x03, 0xc0, 0xf0, 0x3c, 0x03, 0xc0, 0x3c, 0x70, 0x03, 0xc0, 0x0e, 0xe0, 0x03, 0xe0, 0x07, 
  0xc0, 0x03, 0xf0, 0x03, 0xc0, 0x03, 0xf8, 0x03, 0xc0, 0x03, 0x7c, 0x03, 0xc0, 0x03, 0x3e, 0x03, 
  0xc0, 0x03, 0x1f, 0x03, 0xc0, 0x03, 0x1f, 0x03, 0xc0, 0x03, 0x3e, 0x03, 0xc0, 0x03, 0x7c, 0x03, 
  0xc0, 0x03, 0xf8, 0x03, 0xc0, 0x03, 0xf0, 0x03, 0xe0, 0x03, 0xe0, 0x07, 0x70, 0x03, 0xc0, 0x0e, 
  0x3c, 0x03, 0xc0, 0x3c, 0x0f, 0x03, 0xc0, 0xf0, 0x03, 0xc3, 0xc3, 0xc0, 0x00, 0xff, 0xff, 0x00, 
  0x00, 0x3f, 0xfc, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00
};

// ==============================================================================
// 🧠 SYSTEM STATE & HARDWARE FLAGS
// ==============================================================================
int currentDashboard = -1; // -1 = Boot/Instructions
int previousDashboard = -1;
unsigned long lastSlowUpdate = 0;
unsigned long lastFastUpdate = 0;

// Hardware Presence Flags
bool hasAHT10 = false;
bool hasBMI160 = false;
bool hasMAX30102 = false;

// Sensor Data Containers
BMI160Data imuData;
int distanceMM = 0;
int joyX = 0, joyY = 0;
float tempC = 0.0, hum = 0.0;
int bpm = 0, spo2 = 0;

// Button States for UI (String Pointers)
const char* selectEvent = "WAITING...";
const char* startEvent = "WAITING...";

// Melody for the Buzzer Dashboard
#define REST 0 
int dashMelody[] = {      NOTE_A4, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_E5, NOTE_F5, NOTE_D5, NOTE_E5, REST,
                          NOTE_D5, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_C5, NOTE_A4, REST, NOTE_A4,
                          NOTE_D5, NOTE_D5, NOTE_C5, NOTE_B4,
                          NOTE_D5, NOTE_D5, NOTE_C5, NOTE_B4,
                          NOTE_C5, NOTE_A4, REST, NOTE_G4, NOTE_A4, };

int dashDurations[] = {   4, 4, 4, 3, 3, 4, 4, 2, 4,
                          4, 6, 6, 6, 2, 3, 8, 4,
                          4, 4, 4, 4,
                          4, 6, 6, 6,
                          2, 3, 8, 2, 1};

// ==============================================================================
// 🔘 INTERRUPT-LIKE CALLBACKS
// ==============================================================================
void onButtonSelectClick() { selectEvent = "SINGLE CLICK"; buzzerBeepNonBlocking(NOTE_C5, 20); }
void onButtonSelectDoubleClick() { selectEvent = "DOUBLE CLICK"; buzzerBeepNonBlocking(NOTE_E5, 20); }
void onButtonSelectLongPress() { selectEvent = "LONG PRESS"; }
void onButtonSelectLongRelease() { selectEvent = "RELEASED"; }

void onButtonStartClick() { startEvent = "SINGLE CLICK"; buzzerBeepNonBlocking(NOTE_G5, 20); }
void onButtonStartDoubleClick() { startEvent = "DOUBLE CLICK"; buzzerBeepNonBlocking(NOTE_C6, 20); }
void onButtonStartLongPress() { startEvent = "LONG PRESS"; }
void onButtonStartLongRelease() { startEvent = "RELEASED"; }

// ==============================================================================
// 🖥️ SERIAL MENU SYSTEM
// ==============================================================================
void printMenu() {
  Serial.println(F("\n================================="));
  Serial.println(F("    K-COMP V2 FACTORY TEST       "));
  Serial.println(F("================================="));
  Serial.println(F(" [0] LED Control"));
  Serial.println(F(" [1] Potentiometer"));
  Serial.println(F(" [2] Joystick"));
  Serial.println(F(" [3] Distance Sensor"));
  Serial.println(F(" [4] START Button"));
  Serial.println(F(" [5] SELECT Button"));
  Serial.println(F(" [6] Buzzer Test"));
  Serial.println(F(" [7] AHT10 (Temp/Hum)"));
  Serial.println(F(" [8] BMI160 (IMU)"));
  Serial.println(F(" [9] MAX30102 (Vitals)"));
  Serial.println(F("================================="));
  Serial.print(F("Select Dashboard (0-9): "));
}

// ==============================================================================
// 🛠️ SYSTEM SETUP
// ==============================================================================
void setup() {
  Serial.begin(115200);
  
  if (!initDisplay()) {
    Serial.println(F("HALT: OLED missing. Check I2C bus."));
    while(1);
  }

  // --- BOOT SPLASH: BITMAP LOGO ---
  firstPage();
  do {
    // Render the new 32x32 PROGMEM Bitmap
    drawBitmap(16, 16, logo_bitmap_32x32, 32, 32, OLED_COLOR_WHITE);
    
    // Render the K-COMP text next to it
    setCursor(56, 28);
    printText("K-COMP", OLED_COLOR_WHITE, OLED_TEXT_LARGE);
  } while (nextPage());
  delay(2000);

  // --- BOOT SPLASH: INSTRUCTIONS ---
  firstPage();
  do {
    setCursor(OLED_ALIGN_CENTER_X(13, OLED_TEXT_NORMAL), 20);
    printText("OPEN SERIAL", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
    setCursor(OLED_ALIGN_CENTER_X(17, OLED_TEXT_NORMAL), 35);
    printText("MONITOR TO SELECT", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
  } while (nextPage());
  
  // --- INITIALIZE CORE PERIPHERALS ---
  initLed();
  initBuzzer();
  initDistanceSensor();
  initPotentiometer();
  initJoystick();

  initButtonSelect();
  attachButtonSelectClick(onButtonSelectClick);
  attachButtonSelectDoubleClick(onButtonSelectDoubleClick);
  attachButtonSelectLongPressed(onButtonSelectLongPress);
  attachButtonSelectLongReleased(onButtonSelectLongRelease);

  initButtonStart();
  attachButtonStartClick(onButtonStartClick);
  attachButtonStartDoubleClick(onButtonStartDoubleClick);
  attachButtonStartLongPressed(onButtonStartLongPress);
  attachButtonStartLongReleased(onButtonStartLongRelease);

  // --- INITIALIZE I2C SENSORS ---
  Serial.println(F("\nProbing I2C Bus..."));
  
  initAHT10(); 
  hasAHT10 = true; // Assuming standard library hook
  
  hasBMI160 = initBMI160();
  hasMAX30102 = initMAX30102();

  Serial.print(F("AHT10:    ")); Serial.println(hasAHT10 ? F("OK") : F("MISSING"));
  Serial.print(F("BMI160:   ")); Serial.println(hasBMI160 ? F("OK") : F("MISSING"));
  Serial.print(F("MAX30102: ")); Serial.println(hasMAX30102 ? F("OK") : F("MISSING"));

  delay(1000);
  printMenu();
}

// ==============================================================================
// 🔄 THE BREATHING LOOP (Non-Blocking)
// ==============================================================================
void loop() {
  unsigned long currentMillis = millis();

  // --- THE HYPER-FAST LAYER ---
  // Must run continuously to prevent MAX30102 hardware FIFO from overflowing
  if (hasMAX30102) updateVitalSigns();

  // --- SERIAL MENU ROUTING ---
  if (Serial.available() > 0) {
    char inChar = Serial.read();
    if (inChar >= '0' && inChar <= '9') {
      previousDashboard = currentDashboard;
      currentDashboard = inChar - '0';
      
      if (previousDashboard == 0) ledOff(); // Cleanup LED
      if (currentDashboard == 6) {
        // Trigger the melody once when switching to the Buzzer dashboard
        buzzerMelodyNonBlocking(dashMelody, dashDurations, 30); 
      }
      
      Serial.println(inChar); 
      printMenu();
    }
  }
  

  // --- THE FAST LOOP (20ms) ---
  if (currentMillis - lastFastUpdate >= 20) {
    lastFastUpdate = currentMillis;
    
    pollSelectButton();
    pollStartButton();
    updateMelody();
    
    if (hasBMI160) imuData = readBMI160(); 
  }

  // --- THE SLOW LOOP (200ms) ---
  if (currentMillis - lastSlowUpdate >= 200) {
    lastSlowUpdate = currentMillis;
    char valBuffer[10];

    // Gather Heavy Sensor Data 
    if (currentDashboard == 2) {
      joyX = getJoystickXValue();
      joyY = getJoystickYValue();
    }
    if (currentDashboard == 3) {
      distanceMM = getDistanceMM();
    }
    if (currentDashboard == 7 && hasAHT10) {
      tempC = getAHT10Temperature();
      hum = getAHT10Humidity();
    }
    if (currentDashboard == 9 && hasMAX30102) {
      bpm = getHeartRate();
      spo2 = getSPO2();
    }

    // --- RENDER UI ---
    if (currentDashboard >= 0) {
      firstPage();
      do {
        // CRITICAL: Service the MAX30102 *inside* the rendering loop to prevent buffer overflow!
        if (hasMAX30102) updateVitalSigns();

        // --- HEADER BAR ---
        fillRect(0, 0, OLED_WIDTH, 12, OLED_COLOR_WHITE);
        setCursor(2, 2);
        
        switch (currentDashboard) {
          case 0: printText("TEST: LED", OLED_COLOR_BLACK, OLED_TEXT_NORMAL); break;
          case 1: printText("TEST: POTENTIOMETER", OLED_COLOR_BLACK, OLED_TEXT_NORMAL); break;
          case 2: printText("TEST: JOYSTICK", OLED_COLOR_BLACK, OLED_TEXT_NORMAL); break;
          case 3: printText("TEST: DISTANCE", OLED_COLOR_BLACK, OLED_TEXT_NORMAL); break;
          case 4: printText("TEST: START BTN", OLED_COLOR_BLACK, OLED_TEXT_NORMAL); break;
          case 5: printText("TEST: SELECT BTN", OLED_COLOR_BLACK, OLED_TEXT_NORMAL); break;
          case 6: printText("TEST: BUZZER", OLED_COLOR_BLACK, OLED_TEXT_NORMAL); break;
          case 7: printText("TEST: AHT10 (T/H)", OLED_COLOR_BLACK, OLED_TEXT_NORMAL); break;
          case 8: printText("TEST: BMI160 (IMU)", OLED_COLOR_BLACK, OLED_TEXT_NORMAL); break;
          case 9: printText("TEST: MAX30102", OLED_COLOR_BLACK, OLED_TEXT_NORMAL); break;
        }

        // --- DYNAMIC CONTENT ---
        if (currentDashboard == 0) {
          setCursor(10, 30); printText("LED is Fading.", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
        }
        else if (currentDashboard == 1) {
          int potVal = getPotentiometerValue(); // 0 to 100
          setCursor(5, 25); printText("Value:", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
          setCursor(50, 25); itoa(potVal, valBuffer, 10); printText(valBuffer, OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
          
          // Eye-Catching Slider UI
          drawRect(10, 42, 108, 14, OLED_COLOR_WHITE); // Outer track
          int fillWidth = map(potVal, 0, 100, 0, 104);
          fillRect(12, 44, fillWidth, 10, OLED_COLOR_WHITE); // Dynamic fill bar
          
          // Draw the physical "Thumbstick" marker
          drawFastVLine(12 + fillWidth, 40, 18, OLED_COLOR_WHITE);
          drawFastVLine(13 + fillWidth, 40, 18, OLED_COLOR_WHITE);
        }
        else if (currentDashboard == 2) {
          setCursor(5, 25); printText("X:", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
          setCursor(20, 25); itoa(joyX, valBuffer, 10); printText(valBuffer, OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
          setCursor(5, 45); printText("Y:", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
          setCursor(20, 45); itoa(joyY, valBuffer, 10); printText(valBuffer, OLED_COLOR_WHITE, OLED_TEXT_NORMAL);

          drawFastHLine(64, 38, 60, OLED_COLOR_WHITE);
          drawFastVLine(94, 14, 48, OLED_COLOR_WHITE);
          int dotX = map(joyX, -512, 511, 64, 124);
          int dotY = map(joyY, -512, 511, 62, 14); 
          if (dotX < 64) dotX = 64; if (dotX > 124) dotX = 124;
          if (dotY < 14) dotY = 14; if (dotY > 62) dotY = 62;
          fillCircle(dotX, dotY, 4, OLED_COLOR_WHITE);
        }
        else if (currentDashboard == 3) {
          setCursor(OLED_ALIGN_CENTER_X(8, OLED_TEXT_LARGE), 25);
          itoa(distanceMM, valBuffer, 10); printText(valBuffer, OLED_COLOR_WHITE, OLED_TEXT_LARGE);
          setCursor(OLED_ALIGN_CENTER_X(2, OLED_TEXT_NORMAL), 45); printText("MM", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
          
          // Dynamic shrinking/growing circle visualization
          if (distanceMM > 0 && distanceMM < 400) {
            int radius = map(distanceMM, 0, 400, 2, 20);
            drawCircle(105, 35, radius, OLED_COLOR_WHITE);
          }
        }
        else if (currentDashboard == 4) {
          setCursor(10, 25); printText("Action:", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
          setCursor(10, 40); printText(startEvent, OLED_COLOR_WHITE, OLED_TEXT_NORMAL); // Adjusted Text Size
        }
        else if (currentDashboard == 5) {
          setCursor(10, 25); printText("Action:", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
          setCursor(10, 40); printText(selectEvent, OLED_COLOR_WHITE, OLED_TEXT_NORMAL); // Adjusted Text Size
        }
        else if (currentDashboard == 6) {
          setCursor(OLED_ALIGN_CENTER_X(12, OLED_TEXT_NORMAL), 30); 
          printText("PLAYING MELODY", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
        }
        else if (currentDashboard == 7) {
          if (!hasAHT10) { setCursor(10, 30); printText("NOT CONNECTED", OLED_COLOR_WHITE, OLED_TEXT_NORMAL); }
          else {
            setCursor(10, 25); printText("Temp: ", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
            itoa((int)tempC, valBuffer, 10); printText(valBuffer, OLED_COLOR_WHITE, OLED_TEXT_NORMAL); printText(" C", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
            setCursor(10, 45); printText("Hum:  ", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
            itoa((int)hum, valBuffer, 10); printText(valBuffer, OLED_COLOR_WHITE, OLED_TEXT_NORMAL); printText(" %", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
          }
        }
        else if (currentDashboard == 8) {
          if (!hasBMI160) { setCursor(10, 30); printText("NOT CONNECTED", OLED_COLOR_WHITE, OLED_TEXT_NORMAL); }
          else {
            setCursor(5, 20); printText("P:", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
            setCursor(20, 20); itoa((int)imuData.angleX, valBuffer, 10); printText(valBuffer, OLED_COLOR_WHITE, OLED_TEXT_NORMAL); 
            setCursor(5, 35); printText("R:", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
            setCursor(20, 35); itoa((int)imuData.angleY, valBuffer, 10); printText(valBuffer, OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
            setCursor(5, 50); printText("Y:", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
            setCursor(20, 50); itoa((int)imuData.angleZ, valBuffer, 10); printText(valBuffer, OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
          }
        }
        else if (currentDashboard == 9) {
          if (!hasMAX30102) { setCursor(10, 30); printText("NOT CONNECTED", OLED_COLOR_WHITE, OLED_TEXT_NORMAL); }
          else {
            setCursor(10, 25); printText("BPM:  ", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
            if (bpm == 0) printText("--", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
            else { itoa(bpm, valBuffer, 10); printText(valBuffer, OLED_COLOR_WHITE, OLED_TEXT_NORMAL); }
            
            setCursor(10, 45); printText("SpO2: ", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
            if (spo2 == 0) printText("--", OLED_COLOR_WHITE, OLED_TEXT_NORMAL);
            else { itoa(spo2, valBuffer, 10); printText(valBuffer, OLED_COLOR_WHITE, OLED_TEXT_NORMAL); printText("%", OLED_COLOR_WHITE, OLED_TEXT_NORMAL); }
          }
        }
      } while (nextPage()); // End of Paged Render Loop
    }
  }

  // --- LED FADE LOGIC (BLOCKING) ---
  if (currentDashboard == 0) {
    // These library functions contain internal delays.
    // The processor will pause here for 2 seconds to complete the visual fade effect.
    ledFadeIn(100, 1000);
    ledFadeOut(100, 1000);
  }

}