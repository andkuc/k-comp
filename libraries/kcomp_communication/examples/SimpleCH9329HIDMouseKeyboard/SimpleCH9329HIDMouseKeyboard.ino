#include "kcomp_CH9329_hid.h"

// --- PINS ---
// Connect CH9329 TX -> Arduino Pin 10
// Connect CH9329 RX -> Arduino Pin 11
#define RX_PIN 6
#define TX_PIN 7

void setup() {
  Serial.begin(115200);
  
  // 1. Initialize the library
  beginCH9329(RX_PIN, TX_PIN);
  
  Serial.println(F("\n=============================================="));
  Serial.println(F("       CH9329 EXTENSIVE TEST SUITE"));
  Serial.println(F("=============================================="));
  Serial.println(F("1. Force Configuration (Unbrick Mode 0)"));
  Serial.println(F("2. Mouse Move Test (Square Pattern)"));
  Serial.println(F("3. Mouse Scroll & Click Test"));
  Serial.println(F("4. Absolute Mouse Test ( (0,0), (200,0), (0,200), (200,200) )"));
  Serial.println(F("5. Keyboard Typing Test (Hello World)"));
  Serial.println(F("6. Macro Test (Open Notepad via Win+R)"));
  Serial.println(F("=============================================="));
  Serial.println(F("Enter a number (1-6) to run a test:"));
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    // Clear buffer
    while(Serial.available()) Serial.read();

    switch (cmd) {
      case '1': testConfig(); break;
      case '2': testMouseMove(); break;
      case '3': testMouseClickScroll(); break;
      case '4': testAbsoluteMouse(); break;
      case '5': testTyping(); break;
      case '6': testMacro(); break;
      default: Serial.println(F("Unknown command.")); break;
    }
    
    Serial.println(F("\nDone. Select another test (1-6):"));
  }
}

// --- TEST 1: CONFIGURATION ---
void testConfig() {
  Serial.println(F("Configuring Chip to Mode 0 (PID E010)..."));
  Serial.println(F("Please wait 3 seconds for chip restart..."));
  configureMode0();
  delay(3000);
  Serial.println(F("Configuration Packet Sent."));
}

// --- TEST 2: RELATIVE MOUSE ---
void testMouseMove() {
  Serial.println(F("Drawing a square with the mouse..."));
  
  // Move in a square: Right -> Down -> Left -> Up
  int speed = 10;
  int dist = 100;
  
  // Right
  for(int i=0; i<dist/speed; i++) { moveMouse(speed, 0); delay(20); }
  delay(200);
  
  // Down
  for(int i=0; i<dist/speed; i++) { moveMouse(0, speed); delay(20); }
  delay(200);
  
  // Left
  for(int i=0; i<dist/speed; i++) { moveMouse(-speed, 0); delay(20); }
  delay(200);
  
  // Up
  for(int i=0; i<dist/speed; i++) { moveMouse(0, -speed); delay(20); }
}

// --- TEST 3: CLICKS & SCROLL ---
void testMouseClickScroll() {
  Serial.println(F("Testing Right Click (Context Menu)..."));
  clickMouse(0x02); // 0x01=Left, 0x02=Right
  delay(100);
  clickMouse(0x00); // Release
  delay(1000);
  
  Serial.println(F("Closing Context Menu (Left Click)..."));
  clickMouse(0x01);
  delay(100);
  clickMouse(0x00);
  delay(1000);
  
  Serial.println(F("Scrolling Down..."));
  for(int i=0; i<5; i++) { scrollMouse(-1); delay(100); }
  
  Serial.println(F("Scrolling Up..."));
  for(int i=0; i<5; i++) { scrollMouse(1); delay(100); }
}

// --- TEST 4: ABSOLUTE MOUSE ---
void testAbsoluteMouse() {
  Serial.println(F("Moving to (0,0)..."));
  moveMouseAbsolute(0, 0);
  delay(1000);
  
  Serial.println(F("Moving to (200, 0)..."));
  moveMouseAbsolute(200, 0);
  delay(1000);
  
  Serial.println(F("Moving to (0, 200)..."));
  moveMouseAbsolute(0, 200);
  delay(1000);

    Serial.println(F("Moving to (200, 200)..."));
  moveMouseAbsolute(200, 200);
  delay(1000);
}

// --- TEST 5: TYPING ---
void testTyping() {
  Serial.println(F("Typing text. Please ensure a text box is focused!"));
  Serial.println(F("Waiting 2 seconds..."));
  delay(2000);
  
  typeText("Hello! This is the CH9329 Library.");
  writeKey(KEY_RETURN);
  typeText("It supports Symbols: !@#$%^&*()");
  writeKey(KEY_RETURN);
}

// --- TEST 6: MACRO (Win+R -> Notepad) ---
void testMacro() {
  Serial.println(F("Executing Macro: Open Notepad..."));
  
  // 1. Press Win + R
  pressKey(KEY_LEFT_GUI); // Windows Key
  writeKey('r');
  releaseAll();
  delay(500); // Wait for Run dialog
  
  // 2. Type "notepad"
  typeText("notepad");
  delay(100);
  
  // 3. Press Enter
  writeKey(KEY_RETURN);
  
  delay(1000); // Wait for Notepad to open
  
  // 4. Type inside Notepad
  typeText("Automation successful!");
}