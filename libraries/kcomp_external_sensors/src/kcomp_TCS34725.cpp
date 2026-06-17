#include "kcomp_TCS34725.h"


#include <Wire.h>
#include <math.h> 
#include <avr/pgmspace.h> // Required to store data in Flash Memory instead of RAM

#define TCS34725_ADDRESS 0x29
#define COMMAND_BIT 0x80
#define REG_ENABLE  0x00
#define REG_ATIME   0x01
#define REG_CONTROL 0x0F
#define REG_ID      0x12
#define REG_STATUS  0x13
#define REG_CDATAL  0x14

// Hidden internal state variables
static uint16_t _red = 0, _green = 0, _blue = 0, _clear = 0;


// Calibration Thresholds (Default safe values)
static uint16_t _threshBlack = 250;
static uint16_t _threshWhite = 15000;

// ==============================================================================
// 🧠 PROGMEM COLOR DICTIONARY (Stored in Flash Memory to save RAM)
// ==============================================================================
#define NUM_DICT_COLORS 13
const uint8_t dictR[] PROGMEM = {65, 55, 25, 25, 35, 15, 15, 25, 50, 42, 60, 55, 55};
const uint8_t dictG[] PROGMEM = {18, 20, 55, 45, 55, 30, 35, 35, 40, 42, 30, 20, 30};
const uint8_t dictB[] PROGMEM = {17, 25, 20, 30, 10, 55, 50, 40, 10, 16, 10, 25, 15};
const uint8_t dictID[] PROGMEM = {COLOR_RED, COLOR_DARK_RED, COLOR_GREEN, COLOR_FOREST_GREEN, 
                                  COLOR_LIME_GREEN, COLOR_BLUE, COLOR_NAVY_BLUE, COLOR_LIGHT_BLUE, 
                                  COLOR_YELLOW, COLOR_LIGHT_YELLOW, COLOR_ORANGE, COLOR_PINK, COLOR_BROWN};

// ==============================================================================
// LOW LEVEL HARDWARE I2C
// ==============================================================================
static void write8(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(TCS34725_ADDRESS);
    Wire.write(COMMAND_BIT | reg);
    Wire.write(value);
    Wire.endTransmission();
}

static uint8_t read8(uint8_t reg) {
    Wire.beginTransmission(TCS34725_ADDRESS);
    Wire.write(COMMAND_BIT | reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)TCS34725_ADDRESS, (uint8_t)1);
    return Wire.read();
}

// ==============================================================================
// CORE FUNCTIONS
// ==============================================================================
bool initColorSensor() {
    Wire.begin();
    if (read8(REG_ID) != 0x44 && read8(REG_ID) != 0x4D) return false; 
    write8(REG_ATIME, COLOR_TIME_50MS); 
    write8(REG_CONTROL, 0x01); 
    write8(REG_ENABLE, 0x01); 
    delay(3);                 
    write8(REG_ENABLE, 0x03); 
    return true;
}


void setColorIntegrationTime(uint8_t timeSetting) { write8(REG_ATIME, timeSetting); }

void calibrateColorEngine(uint16_t blackThreshold, uint16_t whiteThreshold) {
    _threshBlack = blackThreshold;
    _threshWhite = whiteThreshold;
}

void pollColorSensor() {
    if (read8(REG_STATUS) & 0x01) {
        Wire.beginTransmission(TCS34725_ADDRESS);
        Wire.write(COMMAND_BIT | REG_CDATAL);
        Wire.endTransmission();
        Wire.requestFrom((uint8_t)TCS34725_ADDRESS, (uint8_t)8);
        if (Wire.available() == 8) {
            _clear = Wire.read() | (Wire.read() << 8);
            _red   = Wire.read() | (Wire.read() << 8);
            _green = Wire.read() | (Wire.read() << 8);
            _blue  = Wire.read() | (Wire.read() << 8);
        }
    }
}

// GETTERS
uint16_t getRed()   { return _red; }
uint16_t getGreen() { return _green; }
uint16_t getBlue()  { return _blue; }
uint16_t getClear() { return _clear; }

uint16_t getLux() {
    float lux = (-0.32466F * _red) + (1.57837F * _green) + (-0.73191F * _blue);
    return (lux > 0) ? (uint16_t)lux : 0;
}

uint16_t getColorTemperature() {
    float X = -0.14282F * _red + 1.54924F * _green - 0.95641F * _blue;
    float Y = -0.32466F * _red + 1.57837F * _green - 0.73191F * _blue;
    float Z = -0.68202F * _red + 0.77073F * _green + 0.56332F * _blue;
    if ((X + Y + Z) == 0) return 0;
    float xc = (X) / (X + Y + Z);
    float yc = (Y) / (X + Y + Z);
    if ((0.1858F - yc) == 0) return 0;
    float n = (xc - 0.3320F) / (0.1858F - yc);
    float cct = 449.0F * pow(n, 3) + 3525.0F * pow(n, 2) + 6823.3F * n + 5520.33F;
    return (cct > 0) ? (uint16_t)cct : 0;
}

uint8_t estimateColor() {
    if (_clear < _threshBlack) return COLOR_BLACK;
    if (_clear > _threshWhite) return COLOR_WHITE;

    uint32_t totalRGB = _red + _green + _blue;
    if (totalRGB == 0) totalRGB = 1; 

    long rPct = ((long)_red * 100) / totalRGB;
    long gPct = ((long)_green * 100) / totalRGB;
    long bPct = ((long)_blue * 100) / totalRGB;

    if (rPct > 25 && rPct < 40 && gPct > 25 && gPct < 45 && bPct > 20 && bPct < 40) {
        return COLOR_GREY;
    }

    long shortestDistance = 2147483647; 
    uint8_t bestMatchID = COLOR_UNKNOWN;

    // Nearest Neighbor against Flash Memory Dictionary
    for (int i = 0; i < NUM_DICT_COLORS; i++) {
        long dR = rPct - pgm_read_byte(&dictR[i]);
        long dG = gPct - pgm_read_byte(&dictG[i]);
        long dB = bPct - pgm_read_byte(&dictB[i]);

        long distSq = (dR * dR) + (dG * dG) + (dB * dB);
        if (distSq < shortestDistance) {
            shortestDistance = distSq;
            bestMatchID = pgm_read_byte(&dictID[i]);
        }
    }
    return bestMatchID;
}

const char* getColorName(uint8_t colorCode) {
    switch(colorCode) {
        case COLOR_BLACK:        return "Black";
        case COLOR_WHITE:        return "White";
        case COLOR_GREY:         return "Grey";
        case COLOR_RED:          return "Red";
        case COLOR_DARK_RED:     return "Dark Red";
        case COLOR_GREEN:        return "Green";
        case COLOR_FOREST_GREEN: return "Forest Green";
        case COLOR_LIME_GREEN:   return "Lime Green";
        case COLOR_BLUE:         return "Blue";
        case COLOR_NAVY_BLUE:    return "Navy Blue";
        case COLOR_LIGHT_BLUE:   return "Light Blue";
        case COLOR_YELLOW:       return "Yellow";
        case COLOR_LIGHT_YELLOW: return "Light Yellow";
        case COLOR_ORANGE:       return "Orange";
        case COLOR_PINK:         return "Pink";
        case COLOR_BROWN:        return "Brown";
        default:                 return "Unknown";
    }
}