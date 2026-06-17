#ifndef KCOMP_OLED_GRID_H
#define KCOMP_OLED_GRID_H

#include <Arduino.h>

// ==============================================================================
// 🧠 MEMORY CALCULATOR MACRO
// Evaluated at compile-time. Calculates the exact number of bytes needed 
// to store the bit-crunched grid (2 cells per byte).
// ==============================================================================
#define GRID_MEM_SIZE(rows, cols) (((rows) * (cols) + 1) / 2)

// ==============================================================================
// CALLBACK SIGNATURES
// ==============================================================================
typedef void (*GridRenderCallback)(int16_t anchorX, int16_t anchorY, int16_t cellWidth, int16_t cellHeight, uint8_t cellState);
typedef void (*GridCursorCallback)(int16_t anchorX, int16_t anchorY, int16_t cellWidth, int16_t cellHeight);

// ==============================================================================
// CORE SYSTEM FUNCTIONS
// ==============================================================================
// Initializes the grid system and links the student's memory array to the engine.
void initGrid(uint8_t rows, uint8_t cols, int16_t cellW, int16_t cellH, int16_t offsetX, int16_t offsetY, uint8_t* memoryPointer);

void attachGridRender(GridRenderCallback callback);
void attachCursorRender(GridCursorCallback callback);

// ==============================================================================
// STATE MANAGEMENT
// ==============================================================================
void clearGrid();
void setGridCell(uint8_t row, uint8_t col, uint8_t state);
uint8_t getGridCell(uint8_t row, uint8_t col);

// ==============================================================================
// CURSOR LOGIC
// ==============================================================================
void enableCursor(bool enable);
void setCursorPos(uint8_t row, uint8_t col);
void moveCursor(int8_t deltaRow, int8_t deltaCol);
uint8_t getCursorRow();
uint8_t getCursorCol();

// ==============================================================================
// HARDWARE RENDERING
// ==============================================================================
void drawGridFramework();
void renderGrid();

#endif