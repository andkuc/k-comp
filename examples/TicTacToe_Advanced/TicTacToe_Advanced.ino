#include "kcomp_oled_paged.h"
#include "kcomp_joystick.h"
#include "kcomp_button_select.h"
#include "kcomp_button_start.h"
#include "kcomp_led.h"
#include "kcomp_buzzer.h"
#include <SoftwareSerial.h>
#include <avr/pgmspace.h> // Required for storing bitmaps in Flash memory instead of SRAM

// ==============================================================================
// 1. MEMORY, VARIABLES & NETWORK
// ==============================================================================
SoftwareSerial linkSerial(6, 7); // RX, TX

// 32x32 Epic Pixel Art Logo (Stored in PROGMEM to save RAM)
const unsigned char splashBmp[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xe0,
  0x0e, 0x00, 0x00, 0x70, 0x1c, 0x00, 0x00, 0x38, 0x38, 0x01, 0x80, 0x1c,
  0x38, 0x01, 0x80, 0x1c, 0x70, 0x01, 0x80, 0x0e, 0x70, 0x01, 0x80, 0x0e,
  0xe0, 0x7f, 0xfe, 0x07, 0xe0, 0x7f, 0xfe, 0x07, 0xe0, 0x01, 0x80, 0x07,
  0xe0, 0x01, 0x80, 0x07, 0x70, 0x01, 0x80, 0x0e, 0x70, 0x01, 0x80, 0x0e,
  0x38, 0x01, 0x80, 0x1c, 0x38, 0x01, 0x80, 0x1c, 0x1c, 0x00, 0x00, 0x38,
  0x0e, 0x00, 0x00, 0x70, 0x07, 0xff, 0xff, 0xe0, 0x03, 0xff, 0xff, 0xc0,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00,
  0x02, 0x28, 0x00, 0x00, 0x04, 0x44, 0x00, 0x00, 0x08, 0x82, 0x00, 0x00,
  0x11, 0x01, 0x00, 0x00, 0x22, 0x00, 0x80, 0x00, 0x44, 0x00, 0x40, 0x00,
  0x88, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t gameMemory[3][3] = {{0}}; 
uint8_t gameState = 0; // 0=Menu, 1=WaitLink, 2=Playing, 3=GameOver
uint8_t networkMode = 0; // 0=Local, 1=Host (X), 2=Client (O)
uint8_t myPlayerRole = 1; // 1 = X, 2 = O (Used to lock controls in network mode)
uint8_t currentPlayer = 1; 

int8_t cursorR = 1;
int8_t cursorC = 1;
unsigned long lastJoyMove = 0;

// Juice / Game Feel Variables
int8_t shakeX = 0;
int8_t shakeY = 0;
uint8_t shakeTime = 0;
uint8_t menuSelection = 0; // 0: Local, 1: Host, 2: Join

// ==============================================================================
// 2. GAME LOGIC & JUICE (Runs exactly once per frame)
// ==============================================================================
void triggerShake(uint8_t intensity) {
  shakeTime = intensity;
}

void checkWin() {
  uint8_t winner = 0;
  // Check Rows & Cols
  for (uint8_t i = 0; i < 3; i++) {
    if (gameMemory[i][0] != 0 && gameMemory[i][0] == gameMemory[i][1] && gameMemory[i][1] == gameMemory[i][2]) winner = gameMemory[i][0];
    if (gameMemory[0][i] != 0 && gameMemory[0][i] == gameMemory[1][i] && gameMemory[1][i] == gameMemory[2][i]) winner = gameMemory[0][i];
  }
  // Check Diagonals
  if (gameMemory[0][0] != 0 && gameMemory[0][0] == gameMemory[1][1] && gameMemory[1][1] == gameMemory[2][2]) winner = gameMemory[0][0];
  if (gameMemory[0][2] != 0 && gameMemory[0][2] == gameMemory[1][1] && gameMemory[1][1] == gameMemory[2][0]) winner = gameMemory[0][2];

  if (winner != 0) {
    gameState = 3; 
    triggerShake(20); // Massive screen shake on win
    
    // Victory Arpeggio
    buzzerBeepNonBlocking(523, 100); delay(100);
    buzzerBeepNonBlocking(659, 100); delay(100);
    buzzerBeepNonBlocking(784, 200); 
  } else {
    // Check Tie
    bool full = true;
    for(int r=0; r<3; r++) for(int c=0; c<3; c++) if(gameMemory[r][c] == 0) full = false;
    if(full) {
      gameState = 3;
      buzzerBeepNonBlocking(150, 400); // Sad tone
    }
  }
}

void applyMove(uint8_t r, uint8_t c, uint8_t player) {
  if (gameMemory[r][c] == 0) {
    gameMemory[r][c] = player;
    
    ledOn(100); 
    if(player == 1) buzzerBeepNonBlocking(800, 50); // X makes a high blip
    else buzzerBeepNonBlocking(400, 50);            // O makes a low blop
    ledOn(0);                
    triggerShake(5); // Small bump on placement

    checkWin();
    if(gameState == 2) currentPlayer = (player == 1) ? 2 : 1;
  }
}

// ==============================================================================
// 3. HARDWARE & NETWORK CALLBACKS
// ==============================================================================
void updateNetwork() {
  if (networkMode == 0) return; // Local mode ignores serial

  // State 1: Handshake
  if (gameState == 1) {
    if (networkMode == 1) { // Host
      linkSerial.write('H');
      if (linkSerial.read() == 'A') gameState = 2; // Client acknowledged, start game!
    } else if (networkMode == 2) { // Client
      if (linkSerial.read() == 'H') {
        linkSerial.write('A');
        gameState = 2; // Acknowledged host, start game!
      }
    }
    delay(50);
  }
  
  // State 2: Gameplay Sync
  if (gameState == 2 && linkSerial.available() >= 2) {
    uint8_t r = linkSerial.read();
    uint8_t c = linkSerial.read();
    if (r < 3 && c < 3) {
      applyMove(r, c, currentPlayer); // Apply the opponent's move
    }
  }
}

void onSelectClicked() {
  if (gameState == 0) {
    // Menu Selection
    networkMode = menuSelection;
    if (networkMode == 0) { myPlayerRole = 0; gameState = 2; } // Local (controls both)
    if (networkMode == 1) { myPlayerRole = 1; gameState = 1; } // Host (Plays X)
    if (networkMode == 2) { myPlayerRole = 2; gameState = 1; } // Client (Plays O)
    
    buzzerBeepNonBlocking(600, 100);
    for(int r=0; r<3; r++) for(int c=0; c<3; c++) gameMemory[r][c] = 0;
    currentPlayer = 1;
    return;
  }

  if (gameState == 2) {
    // Only allow move if it's local mode, OR if it's your network turn
    if (networkMode == 0 || currentPlayer == myPlayerRole) {
      if (gameMemory[cursorR][cursorC] == 0) {
        if(networkMode != 0) {
          // Send move to opponent
          linkSerial.write(cursorR);
          linkSerial.write(cursorC);
        }
        applyMove(cursorR, cursorC, currentPlayer);
      } else {
        buzzerBeepNonBlocking(200, 100); // Error
        triggerShake(2);
      }
    }
  }
}

void onStartClicked() {
  if (gameState == 3) {
    gameState = 0; // Return to menu
    buzzerBeepNonBlocking(440, 200); 
  }
}

// ==============================================================================
// 4. SYSTEM SETUP
// ==============================================================================
void setup() {
  initDisplay(); // CRITICAL: Initializes the memory-efficient display engine
  initJoystick();
  initButtonSelect();
  initButtonStart();
  initLed();
  initBuzzer(); 

  linkSerial.begin(9600); // Initialize Link Cable

  attachButtonSelectClick(onSelectClicked);
  attachButtonStartClick(onStartClicked);
}

// ==============================================================================
// 5. MAIN LOOP (Logic separated from Paged Rendering)
// ==============================================================================
void loop() {
  // --- 1. LOGIC & HARDWARE POLL ---
  pollSelectButton();
  pollStartButton();
  updateMelody(); 
  updateNetwork();

  // Screen Shake Decay Math
  if (shakeTime > 0) {
    shakeX = (millis() % 5) - 2; // Random-ish jitter between -2 and 2
    shakeY = (millis() % 3) - 1;
    shakeTime--;
  } else {
    shakeX = 0; shakeY = 0;
  }

  // Joystick Polling (with cooldown)
  if (millis() - lastJoyMove > 150) {
    int joyX = getJoystickXValue();
    int joyY = getJoystickYValue();
    bool moved = false;

    if (gameState == 0) { // Menu Navigation
      if (joyY > 250 && menuSelection > 0) { menuSelection--; moved = true; }
      if (joyY < -250 && menuSelection < 2) { menuSelection++; moved = true; }
    } 
    else if (gameState == 2) { 
      // Corrected Grid Navigation (X controls Columns, Y controls Rows)
      if (joyX > 250 && cursorC < 2)  { cursorC++; moved = true; }
      if (joyX < -250 && cursorC > 0) { cursorC--; moved = true; }
      if (joyY > 250 && cursorR < 2)  { cursorR++; moved = true; }
      if (joyY < -250 && cursorR > 0) { cursorR--; moved = true; }
    }
    
    if (moved) {
      lastJoyMove = millis();
      buzzerBeepNonBlocking(1000, 10); // Tiny haptic click
    }
  }

  // --- 2. PAGED RENDER ENGINE ---
  firstPage();
  do {
    if (gameState == 0) {
      // TITLE SCREEN WITH BITMAP
      setCursor(10 + shakeX, 10 + shakeY);
      printText("TIC-TAC-TOE", OLED_COLOR_WHITE, 1);
      
      // Draw the stylized 32x32 PROGMEM Bitmap on the right side of the screen
      drawBitmap(85 + shakeX, 15 + shakeY, splashBmp, 32, 32, OLED_COLOR_WHITE);
      
      setCursor(5 + shakeX, 30 + shakeY); printText(menuSelection == 0 ? "> 1P LOCAL" : "  1P LOCAL", OLED_COLOR_WHITE, 1);
      setCursor(5 + shakeX, 40 + shakeY); printText(menuSelection == 1 ? "> 2P HOST(X)" : "  2P HOST(X)", OLED_COLOR_WHITE, 1);
      setCursor(5 + shakeX, 50 + shakeY); printText(menuSelection == 2 ? "> 2P JOIN(O)" : "  2P JOIN(O)", OLED_COLOR_WHITE, 1);
    } 
    else if (gameState == 1) {
      // LINK WAIT
      setCursor(20 + shakeX, 30 + shakeY);
      // Visual Heartbeat on text
      if ((millis() / 500) % 2 == 0) printText("WAITING LINK...", OLED_COLOR_WHITE, 1);
    }
    else {
      // GAMEPLAY & GAME OVER
      int gridOffX = 5 + shakeX;
      int gridOffY = 2 + shakeY;
      int cellSize = 20;

      // Draw Grid Lines
      drawLine(gridOffX + cellSize, gridOffY, gridOffX + cellSize, gridOffY + 60, OLED_COLOR_WHITE);
      drawLine(gridOffX + cellSize*2, gridOffY, gridOffX + cellSize*2, gridOffY + 60, OLED_COLOR_WHITE);
      drawLine(gridOffX, gridOffY + cellSize, gridOffX + 60, gridOffY + cellSize, OLED_COLOR_WHITE);
      drawLine(gridOffX, gridOffY + cellSize*2, gridOffX + 60, gridOffY + cellSize*2, OLED_COLOR_WHITE);

      // Draw Pieces & Cursor
      for(int r=0; r<3; r++) {
        for(int c=0; c<3; c++) {
          int cx = gridOffX + (c * cellSize);
          int cy = gridOffY + (r * cellSize);
          
          if (gameMemory[r][c] == 1) { // X
            drawLine(cx+4, cy+4, cx+16, cy+16, OLED_COLOR_WHITE);
            drawLine(cx+16, cy+4, cx+4, cy+16, OLED_COLOR_WHITE);
          } else if (gameMemory[r][c] == 2) { // O
            drawCircle(cx+10, cy+10, 6, OLED_COLOR_WHITE);
          }

          // Draw Cursor (Blinking if it's your turn in network mode)
          if (gameState == 2 && cursorR == r && cursorC == c) {
            if (networkMode == 0 || currentPlayer == myPlayerRole || (millis() / 200) % 2 == 0) {
              drawRect(cx+2, cy+2, 16, 16, OLED_COLOR_INVERT);
            }
          }
        }
      }

      // UI Panel
      setCursor(75 + shakeX, 10 + shakeY);
      if (gameState == 3) {
        printText("GAME OVER", OLED_COLOR_WHITE, 1);
        
        // Split "PRESS START" onto two lines so it fits gracefully within the remaining screen width
        if ((millis() / 400) % 2 == 0) {
            setCursor(75 + shakeX, 35 + shakeY);
            printText("PRESS", OLED_COLOR_WHITE, 1);
            setCursor(75 + shakeX, 45 + shakeY);
            printText("START", OLED_COLOR_WHITE, 1);
        }
      } else {
        if (currentPlayer == 1) printText("X TURN", OLED_COLOR_WHITE, 1);
        else printText("O TURN", OLED_COLOR_WHITE, 1);
      }
    }
  } while (nextPage());
}