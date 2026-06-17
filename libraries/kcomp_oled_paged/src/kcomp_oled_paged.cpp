#include "kcomp_oled_paged.h"
#include <Wire.h>
#include <avr/pgmspace.h>
#include "glcdfont.c"

// ==============================================================================
// 🧠 THE PAGED SHADOW BUFFER (128 Bytes Total)
// ==============================================================================
static uint8_t _oled_buffer[128]; // Exactly 1 horizontal band of the screen.
static uint8_t _current_page = 0; // Tracks which of the 8 bands we are drawing.

static uint8_t _rotation = OLED_ROTATION_0;
static int16_t _cursor_x = 0;
static int16_t _cursor_y = 0;

// OPTIMIZED: Zero-RAM dynamic dimension evaluation
#define CURRENT_WIDTH  ((_rotation % 2 == 0) ? OLED_WIDTH : OLED_HEIGHT)
#define CURRENT_HEIGHT ((_rotation % 2 == 0) ? OLED_HEIGHT : OLED_WIDTH)

static void sendCommand(uint8_t cmd) {
    Wire.beginTransmission(OLED_I2C_ADDRESS);
    Wire.write(0x00); 
    Wire.write(cmd);
    Wire.endTransmission();
}

// ==============================================================================
// 🛠️ SYSTEM & THE PICTURE LOOP
// ==============================================================================

bool initDisplay() {
    delay(100); // Analog stabilization for Cold Boot
    
    Wire.begin();
    Wire.beginTransmission(OLED_I2C_ADDRESS);
    if (Wire.endTransmission() != 0) return false; 

    // CHUNK 1
    Wire.beginTransmission(OLED_I2C_ADDRESS);
    Wire.write(0x00); Wire.write(0xAE); 
    Wire.write(0xD5); Wire.write(0x80); 
    Wire.write(0xA8); Wire.write(0x3F); 
    Wire.write(0xD3); Wire.write(0x00); 
    Wire.endTransmission();

    // CHUNK 2 (Page Addressing Mode Enforced)
    Wire.beginTransmission(OLED_I2C_ADDRESS);
    Wire.write(0x00); Wire.write(0x40); 
    Wire.write(0x8D); Wire.write(0x14); 
    Wire.write(0x20); Wire.write(0x02); // CRITICAL: Page Addressing Mode
    Wire.write(0xA1); Wire.write(0xC8); 
    Wire.write(0xDA); Wire.write(0x12); 
    Wire.endTransmission();

    // CHUNK 3
    Wire.beginTransmission(OLED_I2C_ADDRESS);
    Wire.write(0x00); Wire.write(0x81); Wire.write(0xCF); 
    Wire.write(0xD9); Wire.write(0xF1); 
    Wire.write(0xDB); Wire.write(0x40); 
    Wire.write(0xA4); Wire.write(0xA6); 
    Wire.write(0x2E); Wire.write(0xAF); 
    Wire.endTransmission();

    delay(100); // Charge Pump Spin-Up

    // Instantly wipe physical memory in safe 16-byte chunks
    for (uint8_t p = 0; p < OLED_PAGES; p++) {
        Wire.beginTransmission(OLED_I2C_ADDRESS);
        Wire.write(0x00); Wire.write(0xB0 + p); Wire.write(0x00); Wire.write(0x10);
        Wire.endTransmission();

        for (uint8_t col = 0; col < 128; col += 16) {
            Wire.beginTransmission(OLED_I2C_ADDRESS);
            Wire.write(0x40);
            for (uint8_t i = 0; i < 16; i++) Wire.write(0x00);
            Wire.endTransmission();
        }
    }
    return true;
}

void firstPage() {
    _current_page = 0;
    memset(_oled_buffer, 0, 128);
}

bool nextPage() {
    // 1. Move hardware pointer to the current row safely
    Wire.beginTransmission(OLED_I2C_ADDRESS);
    Wire.write(0x00); 
    Wire.write(0xB0 + _current_page); 
    Wire.write(0x00); 
    Wire.write(0x10); 
    Wire.endTransmission();

    // 2. Blast the 128-byte slice in safe 16-byte chunks
    for (uint8_t col = 0; col < 128; col += 16) {
        Wire.beginTransmission(OLED_I2C_ADDRESS);
        Wire.write(0x40); 
        for (uint8_t i = 0; i < 16; i++) {
            Wire.write(_oled_buffer[col + i]);
        }
        Wire.endTransmission();
    }

    // 3. Step down and clear
    _current_page++;
    if (_current_page >= OLED_PAGES) return false; 
    
    memset(_oled_buffer, 0, 128);
    return true;
}

void setDisplayRotation(uint8_t rotation) {
    _rotation = rotation & 3; 
}

void invertDisplay(bool invert) { sendCommand(invert ? 0xA7 : 0xA6); }
void dimDisplay(bool dim) { sendCommand(0x81); sendCommand(dim ? 0x00 : 0xCF); }

// ==============================================================================
// 🎨 CORE GEOMETRY PRIMITIVES
// ==============================================================================

void drawPixel(int16_t x, int16_t y, uint8_t color) {
    if ((x < 0) || (x >= CURRENT_WIDTH) || (y < 0) || (y >= CURRENT_HEIGHT)) return;

    int16_t t;
    switch (_rotation) {
        case OLED_ROTATION_90:  t = x; x = OLED_WIDTH - 1 - y; y = t; break;
        case OLED_ROTATION_180: x = OLED_WIDTH - 1 - x; y = OLED_HEIGHT - 1 - y; break;
        case OLED_ROTATION_270: t = x; x = y; y = OLED_HEIGHT - 1 - t; break;
    }

    // THE PHYSICAL GATEKEEPER
    uint8_t pixel_page = y / 8;
    if (pixel_page != _current_page) return;

    switch (color) {
        case OLED_COLOR_WHITE:  _oled_buffer[x] |=  (1 << (y & 7)); break;
        case OLED_COLOR_BLACK:  _oled_buffer[x] &= ~(1 << (y & 7)); break;
        case OLED_COLOR_INVERT: _oled_buffer[x] ^=  (1 << (y & 7)); break;
    }
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) { int16_t t = x0; x0 = y0; y0 = t; t = x1; x1 = y1; y1 = t; }
    if (x0 > x1) { int16_t t = x0; x0 = x1; x1 = t; t = y0; y0 = y1; y1 = t; }
    int16_t dx = x1 - x0, dy = abs(y1 - y0);
    int16_t err = dx / 2, ystep = (y0 < y1) ? 1 : -1;
    for (; x0 <= x1; x0++) {
        if (steep) drawPixel(y0, x0, color); else drawPixel(x0, y0, color);
        err -= dy; if (err < 0) { y0 += ystep; err += dx; }
    }
}

void drawFastVLine(int16_t x, int16_t y, int16_t h, uint8_t color) {
    if (x < 0 || x >= CURRENT_WIDTH || y >= CURRENT_HEIGHT) return;
    
    int16_t y0 = y;
    int16_t y1 = y + h - 1;
    
    int16_t page_top = _current_page * 8;
    int16_t page_bottom = page_top + 7;

    // CLIPPING: Skip entirely if off-page
    if (y1 < page_top || y0 > page_bottom) return;

    // CLIPPING: Loop only through visible pixels
    int16_t start_y = (y0 > page_top) ? y0 : page_top;
    int16_t end_y = (y1 < page_bottom) ? y1 : page_bottom;

    for (int16_t i = start_y; i <= end_y; i++) {
        drawPixel(x, i, color);
    }
}

void drawFastHLine(int16_t x, int16_t y, int16_t w, uint8_t color) {
    if (y < 0 || y >= CURRENT_HEIGHT || x >= CURRENT_WIDTH) return;

    // CLIPPING: Horizontal lines occupy exactly one page
    uint8_t line_page = y / 8;
    if (line_page != _current_page) return;

    int16_t x0 = x;
    int16_t x1 = x + w - 1;
    
    // Clip against screen edges
    if (x0 < 0) x0 = 0;
    if (x1 >= CURRENT_WIDTH) x1 = CURRENT_WIDTH - 1;

    for (int16_t i = x0; i <= x1; i++) {
        drawPixel(i, y, color);
    }
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) {
    // MATHEMATICAL Y-AXIS CLIPPING (Bounding Box)
    int16_t rect_bottom = y + h - 1;
    int16_t page_top = _current_page * 8;
    int16_t page_bottom = page_top + 7;

    // Skip entirely if off-page
    if (rect_bottom < page_top || y > page_bottom) return;

    drawFastHLine(x, y, w, color); 
    drawFastHLine(x, rect_bottom, w, color);
    drawFastVLine(x, y, h, color); 
    drawFastVLine(x + w - 1, y, h, color);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) {
    // MATHEMATICAL Y-AXIS CLIPPING
    int16_t rect_bottom = y + h - 1;
    int16_t page_top = _current_page * 8;
    int16_t page_bottom = page_top + 7;

    // Skip entirely if off-page
    if (rect_bottom < page_top || y > page_bottom) return;

    // Direct, exact-bounds pixel loop
    int16_t start_y = (y > page_top) ? y : page_top;
    int16_t end_y = (rect_bottom < page_bottom) ? rect_bottom : page_bottom;
    
    int16_t start_x = (x > 0) ? x : 0;
    int16_t end_x = x + w - 1;
    if (end_x >= CURRENT_WIDTH) end_x = CURRENT_WIDTH - 1;

    for (int16_t i = start_x; i <= end_x; i++) {
        for (int16_t j = start_y; j <= end_y; j++) {
            drawPixel(i, j, color);
        }
    }
}

void drawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color) {
    int16_t f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
    drawPixel(x0, y0 + r, color); drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color); drawPixel(x0 - r, y0, color);
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        drawPixel(x0 + x, y0 + y, color); drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color); drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color); drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color); drawPixel(x0 - y, y0 - x, color);
    }
}

void fillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color) {
    drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    int16_t f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        drawFastVLine(x0 + x, y0 - y, 2 * y + 1, color); drawFastVLine(x0 - x, y0 - y, 2 * y + 1, color);
        drawFastVLine(x0 + y, y0 - x, 2 * x + 1, color); drawFastVLine(x0 - y, y0 - x, 2 * x + 1, color);
    }
}

void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color) {
    drawLine(x0, y0, x1, y1, color); drawLine(x1, y1, x2, y2, color); drawLine(x2, y2, x0, y0, color);
}

void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color) {
    int16_t a, b, y, last;
    if (y0 > y1) { int16_t t = y0; y0 = y1; y1 = t; t = x0; x0 = x1; x1 = t; }
    if (y1 > y2) { int16_t t = y2; y2 = y1; y1 = t; t = x2; x2 = x1; x1 = t; }
    if (y0 > y1) { int16_t t = y0; y0 = y1; y1 = t; t = x0; x0 = x1; x1 = t; }
    if (y0 == y2) { 
        a = b = x0;
        if (x1 < a) a = x1; else if (x1 > b) b = x1;
        if (x2 < a) a = x2; else if (x2 > b) b = x2;
        drawFastHLine(a, y0, b - a + 1, color); return;
    }
    int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;
    last = (y1 == y2) ? y1 : y1 - 1;
    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01; b = x0 + sb / dy02; sa += dx01; sb += dx02;
        if (a > b) { int16_t t = a; a = b; b = t; }
        drawFastHLine(a, y, b - a + 1, color);
    }
    sa = (int32_t)dx12 * (y - y1); sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12; b = x0 + sb / dy02; sa += dx12; sb += dx02;
        if (a > b) { int16_t t = a; a = b; b = t; }
        drawFastHLine(a, y, b - a + 1, color);
    }
}

// ==============================================================================
// 📝 TEXT ENGINE
// ==============================================================================

void setCursor(int16_t x, int16_t y) { _cursor_x = x; _cursor_y = y; }

void printChar(char c, uint8_t color, uint8_t size) {
    if ((_cursor_x >= CURRENT_WIDTH) || (_cursor_y >= CURRENT_HEIGHT) || 
        ((_cursor_x + 5 * size - 1) < 0) || ((_cursor_y + 8 * size - 1) < 0)) {
        return; 
    }

    // MATHEMATICAL Y-AXIS CLIPPING
    int16_t char_top = _cursor_y;
    int16_t char_bottom = _cursor_y + (8 * size) - 1;
    int16_t page_top = _current_page * 8;
    int16_t page_bottom = page_top + 7;

    // If completely off-page, silently advance the cursor
    if (char_bottom < page_top || char_top > page_bottom) {
        _cursor_x += 6 * size; 
        return;               
    }

    for (int8_t i = 0; i < 5; i++) {
        uint8_t line = pgm_read_byte(&font[c * 5 + i]);
        for (int8_t j = 0; j < 8; j++, line >>= 1) {
            if (line & 1) {
                if (size == 1) drawPixel(_cursor_x + i, _cursor_y + j, color);
                else fillRect(_cursor_x + (i * size), _cursor_y + (j * size), size, size, color);
            }
        }
    }
    
    _cursor_x += 6 * size; 
}

void printText(const char* text, uint8_t color, uint8_t size) {
    int16_t startX = _cursor_x;
    while (*text) {
        if (*text == '\n') {
            _cursor_x = startX; _cursor_y += 8 * size;
        } else {
            if (_cursor_x + (5 * size) > CURRENT_WIDTH) {
                _cursor_x = startX; _cursor_y += 8 * size;
            }
            printChar(*text, color, size);
        }
        text++;
    }
}

void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint8_t color) {
    // MATHEMATICAL Y-AXIS CLIPPING
    int16_t bmp_top = y;
    int16_t bmp_bottom = y + h - 1;
    int16_t page_top = _current_page * 8;
    int16_t page_bottom = page_top + 7;

    if (bmp_bottom < page_top || bmp_top > page_bottom) return;

    int16_t byteWidth = (w + 7) / 8; 
    uint8_t b = 0;

    for (int16_t j = 0; j < h; j++, y++) {
        // PER-ROW CLIPPING
        if (y < page_top || y > page_bottom) continue;

        for (int16_t i = 0; i < w; i++) {
            if (i & 7) b <<= 1;
            else b = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            if (b & 0x80) drawPixel(x + i, y, color);
        }
    }
}

void startScrollRight(uint8_t startPage, uint8_t endPage) {
    Wire.beginTransmission(OLED_I2C_ADDRESS);
    Wire.write(0x00);
    Wire.write(0x26); Wire.write(0x00); Wire.write(startPage);
    Wire.write(0x00); Wire.write(endPage); Wire.write(0x00);
    Wire.write(0xFF); Wire.write(0x2F); 
    Wire.endTransmission();
}

void startScrollLeft(uint8_t startPage, uint8_t endPage) {
    Wire.beginTransmission(OLED_I2C_ADDRESS);
    Wire.write(0x00);
    Wire.write(0x27); Wire.write(0x00); Wire.write(startPage);
    Wire.write(0x00); Wire.write(endPage); Wire.write(0x00);
    Wire.write(0xFF); Wire.write(0x2F); 
    Wire.endTransmission();
}

void stopScroll() { sendCommand(0x2E); }