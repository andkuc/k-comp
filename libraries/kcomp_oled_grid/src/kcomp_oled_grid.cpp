#include "kcomp_oled_grid.h"

// Hardware geometry hooks
extern void drawFastVLine(int16_t x, int16_t y, int16_t h, uint8_t color);
extern void drawFastHLine(int16_t x, int16_t y, int16_t w, uint8_t color);
#define OLED_COLOR_WHITE 1

// ==============================================================================
// INTERNAL STATE (Protected)
// ==============================================================================
static uint8_t _rows = 0;
static uint8_t _cols = 0;
static int16_t _cellW = 0;
static int16_t _cellH = 0;
static int16_t _offsetX = 0;
static int16_t _offsetY = 0;

static uint8_t* _gridData = nullptr;

static uint8_t _cursorRow = 0;
static uint8_t _cursorCol = 0;
static bool _cursorActive = false;

static GridRenderCallback _grid_callback = nullptr;
static GridCursorCallback _cursor_callback = nullptr;

// ==============================================================================
// INITIALIZATION
// ==============================================================================
void initGrid(uint8_t rows, uint8_t cols, int16_t cellW, int16_t cellH, int16_t offsetX, int16_t offsetY, uint8_t* memoryPointer) {
    _rows = rows;
    _cols = cols;
    _cellW = cellW;
    _cellH = cellH;
    _offsetX = offsetX;
    _offsetY = offsetY;
    _gridData = memoryPointer;
}

void attachGridRender(GridRenderCallback callback) { _grid_callback = callback; }
void attachCursorRender(GridCursorCallback callback) { _cursor_callback = callback; }

// ==============================================================================
// STATE MANAGEMENT (Bit-Crunching)
// ==============================================================================
void clearGrid() {
    if (_gridData == nullptr) return;
    int totalBytes = ((_rows * _cols) + 1) / 2;
    for (int i = 0; i < totalBytes; i++) {
        _gridData[i] = 0;
    }
}

void setGridCell(uint8_t row, uint8_t col, uint8_t state) {
    if (_gridData == nullptr || row >= _rows || col >= _cols || state > 15) return;

    int linearIndex = (row * _cols) + col;
    uint8_t byteIndex = linearIndex / 2;
    bool isUpperNibble = (linearIndex % 2 != 0);

    if (isUpperNibble) {
        _gridData[byteIndex] &= 0x0F;
        _gridData[byteIndex] |= (state << 4);
    } else {
        _gridData[byteIndex] &= 0xF0;
        _gridData[byteIndex] |= state;
    }
}

uint8_t getGridCell(uint8_t row, uint8_t col) {
    if (_gridData == nullptr || row >= _rows || col >= _cols) return 0;

    int linearIndex = (row * _cols) + col;
    uint8_t byteIndex = linearIndex / 2;
    bool isUpperNibble = (linearIndex % 2 != 0);

    if (isUpperNibble) {
        return (_gridData[byteIndex] >> 4) & 0x0F;
    } else {
        return _gridData[byteIndex] & 0x0F;
    }
}

// ==============================================================================
// CURSOR LOGIC
// ==============================================================================
void enableCursor(bool enable) { _cursorActive = enable; }
void setCursorPos(uint8_t row, uint8_t col) {
    if (row < _rows) _cursorRow = row;
    if (col < _cols) _cursorCol = col;
}

void moveCursor(int8_t deltaRow, int8_t deltaCol) {
    int16_t newRow = (int16_t)_cursorRow + deltaRow;
    int16_t newCol = (int16_t)_cursorCol + deltaCol;

    if (newRow < 0) newRow = 0;
    if (newRow >= _rows) newRow = _rows - 1;
    
    if (newCol < 0) newCol = 0;
    if (newCol >= _cols) newCol = _cols - 1;

    _cursorRow = (uint8_t)newRow;
    _cursorCol = (uint8_t)newCol;
}

uint8_t getCursorRow() { return _cursorRow; }
uint8_t getCursorCol() { return _cursorCol; }

// ==============================================================================
// HARDWARE RENDERING
// ==============================================================================
void drawGridFramework() {
    int16_t totalWidth = _cols * _cellW;
    int16_t totalHeight = _rows * _cellH;

    for (uint8_t c = 0; c <= _cols; c++) {
        int16_t lineX = _offsetX + (c * _cellW);
        drawFastVLine(lineX, _offsetY, totalHeight + 1, OLED_COLOR_WHITE);
    }

    for (uint8_t r = 0; r <= _rows; r++) {
        int16_t lineY = _offsetY + (r * _cellH);
        drawFastHLine(_offsetX, lineY, totalWidth + 1, OLED_COLOR_WHITE);
    }
}

void renderGrid() {
    if (_gridData == nullptr) return;

    drawGridFramework();

    if (_grid_callback != nullptr) {
        for (uint8_t r = 0; r < _rows; r++) {
            for (uint8_t c = 0; c < _cols; c++) {
                uint8_t state = getGridCell(r, c);
                if (state > 0) {
                    int16_t anchorX = _offsetX + (c * _cellW);
                    int16_t anchorY = _offsetY + (r * _cellH);
                    _grid_callback(anchorX, anchorY, _cellW, _cellH, state);
                }
            }
        }
    }

    if (_cursorActive && _cursor_callback != nullptr) {
        int16_t cursorAnchorX = _offsetX + (_cursorCol * _cellW);
        int16_t cursorAnchorY = _offsetY + (_cursorRow * _cellH);
        _cursor_callback(cursorAnchorX, cursorAnchorY, _cellW, _cellH);
    }
}