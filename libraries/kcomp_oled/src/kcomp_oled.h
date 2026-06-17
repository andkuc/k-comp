#ifndef _KCOMP_OLED_H
#define _KCOMP_OLED_H

#include <Arduino.h>

// ==============================================================================
// 📐 HARDWARE CONSTANTS & MACROS
// ==============================================================================
#define OLED_I2C_ADDRESS  0x3C
#define OLED_WIDTH        128
#define OLED_HEIGHT       64

#define OLED_COLOR_BLACK  0
#define OLED_COLOR_WHITE  1
#define OLED_COLOR_INVERT 2

#define OLED_ROTATION_0   0  
#define OLED_ROTATION_90  1  
#define OLED_ROTATION_180 2  
#define OLED_ROTATION_270 3  

#define OLED_TEXT_NORMAL 1  
#define OLED_TEXT_LARGE  2  
#define OLED_TEXT_HUGE   3  

// Zero-Cost Layout Macros
#define OLED_PCT_X(percent)                   ((OLED_WIDTH * (percent)) / 100)
#define OLED_PCT_Y(percent)                   ((OLED_HEIGHT * (percent)) / 100)
#define OLED_TEXT_WIDTH(charCount, size)      ((charCount) * 6 * (size))
#define OLED_TEXT_HEIGHT(size)                (8 * (size))
#define OLED_ALIGN_CENTER_X(charCount, size)  ((OLED_WIDTH - OLED_TEXT_WIDTH(charCount, size)) / 2)
#define OLED_ALIGN_RIGHT_X(charCount, size)   (OLED_WIDTH - OLED_TEXT_WIDTH(charCount, size))
#define OLED_ALIGN_CENTER_Y(size)             ((OLED_HEIGHT - OLED_TEXT_HEIGHT(size)) / 2)
#define OLED_ALIGN_BOTTOM_Y(size)             (OLED_HEIGHT - OLED_TEXT_HEIGHT(size))

#define CURRENT_WIDTH  ((_rotation % 2 == 0) ? OLED_WIDTH : OLED_HEIGHT)
#define CURRENT_HEIGHT ((_rotation % 2 == 0) ? OLED_HEIGHT : OLED_WIDTH)

// ==============================================================================
// 🛠️ SYSTEM & STATE MANAGEMENT
// ==============================================================================

bool initDisplay();
void clearDisplay();
void updateDisplay(); // Pushes the 1024-byte RAM buffer to the physical screen

void setDisplayRotation(uint8_t rotation);
void invertDisplay(bool invert);
void dimDisplay(bool dim);

// ==============================================================================
// 🎨 CORE GEOMETRY PRIMITIVES
// ==============================================================================

void drawPixel(int16_t x, int16_t y, uint8_t color);
bool getPixel(int16_t x, int16_t y);

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color);
void drawFastVLine(int16_t x, int16_t y, int16_t h, uint8_t color);
void drawFastHLine(int16_t x, int16_t y, int16_t w, uint8_t color);

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);

void drawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color);
void fillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color);

void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color);
void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color);

// ==============================================================================
// 📝 TEXT ENGINE & BITMAPS
// ==============================================================================

void setCursor(int16_t x, int16_t y);
void printText(const char* text, uint8_t color, uint8_t size);
void printChar(char c, uint8_t color, uint8_t size);

void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint8_t color);

// ==============================================================================
// 🔄 HARDWARE SCROLLING
// ==============================================================================

void startScrollRight(uint8_t startPage, uint8_t endPage);
void startScrollLeft(uint8_t startPage, uint8_t endPage);
void stopScroll();

#endif