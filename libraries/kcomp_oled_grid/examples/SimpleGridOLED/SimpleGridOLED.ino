#include "kcomp_oled_paged.h"
#include "kcomp_oled_grid.h"
#include "kcomp_joystick.h"

// 1. Claim the system memory for a 3x3 grid
uint8_t myGameMemory[GRID_MEM_SIZE(3, 3)] = {0};

// 2. Define how game pieces look (Triggered automatically by the engine)
void drawPieces(int16_t anchorX, int16_t anchorY, int16_t w, int16_t h, uint8_t state) {
  if (state == 1) { 
    // Draw an 'X' using the anchor points
    drawLine(anchorX, anchorY, anchorX + w, anchorY + h, OLED_COLOR_WHITE);
    drawLine(anchorX + w, anchorY, anchorX, anchorY + h, OLED_COLOR_WHITE);
  }
  else if (state == 2) { 
    // Draw an 'X' using the anchor points
    drawLine(anchorX, anchorY, anchorX + w, anchorY + h, OLED_COLOR_WHITE);
    drawLine(anchorX + w, anchorY, anchorX, anchorY + h, OLED_COLOR_WHITE);
  }
}

// 3. Define how the selection cursor looks
void drawMyCursor(int16_t anchorX, int16_t anchorY, int16_t w, int16_t h) {
  drawRect(anchorX, anchorY, w, h, OLED_COLOR_INVERT); // Blinking inverted box
}

void setup() {
  initDisplay();
  initJoystick();

  // Bind the settings and memory to the engine
  initGrid(3, 3, 20, 20, 34, 2, myGameMemory);
  
  // Attach rendering functions
  attachGridRender(drawPieces);
  attachCursorRender(drawMyCursor);
  
  enableCursor(true);
  
  // Place a test piece in Row 0, Col 0
  setGridCell(0, 0, 1); 
  // Place a test piece in Row 0, Col 0
  setGridCell(0, 0, 2); 
}

void loop() {
  // --- INPUT LAYER ---
  int joyX = getJoystickXValue(); 
  int joyY = getJoystickYValue();
  
  // Basic un-debounced movement
  if (joyX > 250)  moveCursor(0, 1);  // Right
  if (joyX < -250) moveCursor(0, -1); // Left
  if (joyY > 250)  moveCursor(-1, 0); // Up
  if (joyY < -250) moveCursor(1, 0);  // Down

  // --- RENDERING LAYER ---
  firstPage();
  do {
    // Let the engine draw the borders, pieces, and cursor automatically
    renderGrid(); 
  } while (nextPage());
}