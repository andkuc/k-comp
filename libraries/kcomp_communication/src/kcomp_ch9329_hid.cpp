#include "kcomp_CH9329_hid.h"
#include <SoftwareSerial.h>

static SoftwareSerial *chSerial = NULL;

// Internal Keyboard Report State
static uint8_t _keyReport[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// --- PROTOCOL CONSTANTS ---
const uint8_t HEAD_1 = 0x57;
const uint8_t HEAD_2 = 0xAB;
const uint8_t ADDR   = 0x00; 

// Commands
const uint8_t CMD_KB         = 0x02;
const uint8_t CMD_MOUSE_ABS  = 0x04; // Matches CMD_SEND_MS_ABS_DATA
const uint8_t CMD_MOUSE_REL  = 0x05;
const uint8_t CMD_CONFIG     = 0x09; 

// --- LOW LEVEL SEND ---
static void _sendPacket(uint8_t cmd, uint8_t* data, uint8_t len) {
    if (!chSerial) return;
    uint16_t sum = 0;
    
    chSerial->write(HEAD_1); sum += HEAD_1;
    chSerial->write(HEAD_2); sum += HEAD_2;
    chSerial->write(ADDR);   sum += ADDR;
    chSerial->write(cmd);    sum += cmd;
    chSerial->write(len);    sum += len;

    for (int i = 0; i < len; i++) {
        chSerial->write(data[i]);
        sum += data[i];
    }
    chSerial->write((uint8_t)(sum & 0xFF));
    delay(10); 
}

// --- MAPPING UTILS ---
static uint8_t _mapToHID(uint8_t k) {
    if (k >= 'a' && k <= 'z') return 0x04 + (k - 'a');
    if (k >= 'A' && k <= 'Z') return 0x04 + (k - 'A');
    if (k >= '1' && k <= '9') return 0x1E + (k - '1');
    if (k == '0') return 0x27;
    if (k == ' ') return 0x2C;
    if (k == '\n') return 0x28; 
    if (k == '\t') return 0x2B; 

    switch (k) {
        case KEY_RETURN: return 0x28;
        case KEY_ESC:    return 0x29;
        case KEY_BACKSPACE: return 0x2A;
        case KEY_TAB:    return 0x2B;
        case KEY_HOME:   return 0x4A;
        case KEY_PAGE_UP: return 0x4B;
        case KEY_DELETE: return 0x4C;
        case KEY_END:    return 0x4D;
        case KEY_PAGE_DOWN: return 0x4E;
        case KEY_RIGHT_ARROW: return 0x4F;
        case KEY_LEFT_ARROW:  return 0x50;
        case KEY_DOWN_ARROW:  return 0x51;
        case KEY_UP_ARROW:    return 0x52;
        // Function Keys
        case KEY_F1: return 0x3A; case KEY_F2: return 0x3B; case KEY_F3: return 0x3C;
        case KEY_F4: return 0x3D; case KEY_F5: return 0x3E; case KEY_F6: return 0x3F;
        case KEY_F7: return 0x40; case KEY_F8: return 0x41; case KEY_F9: return 0x42;
        case KEY_F10: return 0x43; case KEY_F11: return 0x44; case KEY_F12: return 0x45;
        default: return 0x00; 
    }
}

static void _sendKeyReport() {
    _sendPacket(CMD_KB, _keyReport, 8);
}

// --- SETUP ---
void beginCH9329(int rxPin, int txPin) {
    if (chSerial) delete chSerial;
    chSerial = new SoftwareSerial(rxPin, txPin);
    chSerial->begin(CH9329_BAUD);
    delay(500); 
}

// --- CONFIGURATION ---
void configureMode0() {
    uint8_t cfg[50];
    memset(cfg, 0, 50);
    cfg[0] = 0x00; // Mode 0
    cfg[5] = 0x25; cfg[6] = 0x80; // 9600 Baud
    cfg[11] = 0x1A; cfg[12] = 0x86; // VID
    cfg[13] = 0xE0; cfg[14] = 0x10; // PID E010
    cfg[16] = 0x0A; cfg[18] = 0x64; 
    _sendPacket(CMD_CONFIG, cfg, 50);
    delay(100); 
}

// --- ABSOLUTE MOUSE FIX ---
// Range: 0 (Top/Left) to 4096 (Bottom/Right)
void moveMouseAbsolute(uint16_t x, uint16_t y) {
    // FIX: The CH9329 chip ignores the packet if X and Y are both exactly 0.
    // We force them to 1,1 which is still the top-left pixel on screen.
    if (x == 0 && y == 0) {
        x = 1;
        y = 1;
    }
    
    // Clamp to valid range 0-4096
    if (x > 4096) x = 4096;
    if (y > 4096) y = 4096;

    // Structure: [ReportID 0x02, Buttons, X_Low, X_High, Y_Low, Y_High, Wheel]
    // Little Endian: Low byte first
    uint8_t data[7];
    data[0] = 0x02;         // Report ID
    data[1] = 0x00;         // Buttons (0 = None)
    data[2] = (uint8_t)(x & 0xFF);   // X Low
    data[3] = (uint8_t)((x >> 8) & 0xFF); // X High
    data[4] = (uint8_t)(y & 0xFF);   // Y Low
    data[5] = (uint8_t)((y >> 8) & 0xFF); // Y High
    data[6] = 0x00;         // Wheel

    _sendPacket(CMD_MOUSE_ABS, data, 7);
}

// --- RELATIVE MOUSE ---
void moveMouse(int8_t x, int8_t y) {
    uint8_t data[5] = {0x01, 0x00, (uint8_t)x, (uint8_t)y, 0x00};
    _sendPacket(CMD_MOUSE_REL, data, 5);
}

void clickMouse(uint8_t buttons) {
    uint8_t data[5] = {0x01, buttons, 0x00, 0x00, 0x00};
    _sendPacket(CMD_MOUSE_REL, data, 5);
}

void scrollMouse(int8_t wheel) {
    uint8_t data[5] = {0x01, 0x00, 0x00, 0x00, (uint8_t)wheel};
    _sendPacket(CMD_MOUSE_REL, data, 5);
}

// --- KEYBOARD ---
size_t pressKey(uint8_t k) {
    uint8_t i;
    if (k >= 0x80 && k <= 0x87) {
        _keyReport[0] |= (1 << (k - 0x80));
    } else {
        uint8_t hidCode = _mapToHID(k);
        if (!hidCode) return 0;
        for (i = 2; i < 8; i++) if (_keyReport[i] == hidCode) return 1;
        for (i = 2; i < 8; i++) if (_keyReport[i] == 0) { _keyReport[i] = hidCode; break; }
    }
    _sendKeyReport();
    return 1;
}

size_t releaseKey(uint8_t k) {
    uint8_t i;
    if (k >= 0x80 && k <= 0x87) {
        _keyReport[0] &= ~(1 << (k - 0x80));
    } else {
        uint8_t hidCode = _mapToHID(k);
        for (i = 2; i < 8; i++) if (_keyReport[i] == hidCode) _keyReport[i] = 0;
    }
    _sendKeyReport();
    return 1;
}

void releaseAll() {
    for (int i = 0; i < 8; i++) _keyReport[i] = 0;
    _sendKeyReport();
    uint8_t mData[5] = {0x01, 0x00, 0x00, 0x00, 0x00};
    _sendPacket(CMD_MOUSE_REL, mData, 5);
}

size_t writeKey(uint8_t k) {
    if (k >= 'A' && k <= 'Z') pressKey(KEY_LEFT_SHIFT);
    pressKey(k);
    delay(10);
    releaseKey(k);
    if (k >= 'A' && k <= 'Z') releaseKey(KEY_LEFT_SHIFT);
    return 1;
}

void typeText(const char* text) {
    while (*text) {
        writeKey(*text);
        text++;
        delay(20);
    }
}