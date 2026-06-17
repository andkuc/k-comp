#include "kcomp_LD2410.h"
#include <SoftwareSerial.h>

static SoftwareSerial* _radarSerial = NULL;

// --- Parsing State Machine ---
enum ParseState {
  WAIT_HEADER_1, WAIT_HEADER_2, WAIT_HEADER_3, WAIT_HEADER_4,
  READ_PAYLOAD,
  CHECK_FOOTER_1, CHECK_FOOTER_2, CHECK_FOOTER_3
};

static ParseState _state = WAIT_HEADER_1;
static uint8_t    _frameBuf[64];
static int        _frameIdx = 0;

// --- Data Storage ---
static bool _hasTarget = false;
static int  _detDist = 0;
static int  _movEnergy = 0;
static int  _statEnergy = 0;
static uint8_t _targetType = 0; // 0=None, 1=Mov, 2=Stat, 3=Both

// --- Internal Helper: Parse Valid Packet ---
static void _parsePacket() {
    // _frameBuf contains payload starting AFTER the F4 F3 F2 F1 header.
    // Index 0 = Length LSB
    // Index 4 = Target State (0=No, 1=Mov, 2=Stat, 3=Both)
    // Index 7 = Moving Energy
    // Index 10 = Static Energy
    // Index 11/12 = Detection Distance (Basic Mode)
    
    _targetType = _frameBuf[4];
    _hasTarget  = (_targetType != 0x00);
    _movEnergy  = _frameBuf[7];
    _statEnergy = _frameBuf[10];

    // Combine LSB/MSB for Distance (Indices 11 and 12)
    _detDist = _frameBuf[11] | (_frameBuf[12] << 8);
}

// --- Public Functions ---

bool initLD2410(uint8_t rxPin, uint8_t txPin, long baud) {
    if (_radarSerial) delete _radarSerial;
    _radarSerial = new SoftwareSerial(rxPin, txPin);
    _radarSerial->begin(baud);
    
    // Check connection: Listen for 1.5 seconds for valid data
    unsigned long start = millis();
    while (millis() - start < 1500) {
        if (updateLD2410()) return true;
    }
    return false;
}

bool updateLD2410() {
    if (!_radarSerial) return false;
    bool newData = false;

    while (_radarSerial->available()) {
        uint8_t b = _radarSerial->read();

        switch (_state) {
            // 1. Hunt for Header F4 F3 F2 F1
            case WAIT_HEADER_1: if (b == 0xF4) _state = WAIT_HEADER_2; break;
            case WAIT_HEADER_2: if (b == 0xF3) _state = WAIT_HEADER_3; else _state = WAIT_HEADER_1; break;
            case WAIT_HEADER_3: if (b == 0xF2) _state = WAIT_HEADER_4; else _state = WAIT_HEADER_1; break;
            case WAIT_HEADER_4: 
                if (b == 0xF1) {
                    _state = READ_PAYLOAD;
                    _frameIdx = 0;
                } else _state = WAIT_HEADER_1;
                break;

            // 2. Read until Footer Start F8
            case READ_PAYLOAD:
                _frameBuf[_frameIdx++] = b;
                if (_frameIdx >= 60) _state = WAIT_HEADER_1; // Overflow protection
                if (b == 0xF8) _state = CHECK_FOOTER_1;
                break;

            // 3. Verify Footer F7 F6 F5
            case CHECK_FOOTER_1:
                _frameBuf[_frameIdx++] = b;
                if (b == 0xF7) _state = CHECK_FOOTER_2;
                else _state = READ_PAYLOAD; 
                break;
            case CHECK_FOOTER_2:
                _frameBuf[_frameIdx++] = b;
                if (b == 0xF6) _state = CHECK_FOOTER_3;
                else _state = READ_PAYLOAD;
                break;
            case CHECK_FOOTER_3:
                _frameBuf[_frameIdx++] = b;
                if (b == 0xF5) {
                    // Packet Complete!
                    _parsePacket();
                    newData = true;
                    _state = WAIT_HEADER_1;
                } else _state = READ_PAYLOAD;
                break;
        }
    }
    return newData;
}

void setLD2410BaudRate(long newBaud) {
    if (!_radarSerial) return;
    uint8_t idx = (newBaud==9600)?0x01 : (newBaud==256000)?0x07 : 0x04;
    
    // Commands
    uint8_t cmdEnable[] = {0xFD,0xFC,0xFB,0xFA,0x04,0x00,0x01,0x00,0x00,0x04,0x03,0x02,0x01};
    uint8_t cmdBaud[]   = {0xFD,0xFC,0xFB,0xFA,0x04,0x00,0xA1,0x00,idx, 0x00,0x04,0x03,0x02,0x01};
    uint8_t cmdRest[]   = {0xFD,0xFC,0xFB,0xFA,0x02,0x00,0xFE,0x00,0x04,0x03,0x02,0x01};

    _radarSerial->write(cmdEnable, sizeof(cmdEnable)); delay(50);
    _radarSerial->write(cmdBaud, sizeof(cmdBaud));     delay(50);
    _radarSerial->write(cmdRest, sizeof(cmdRest));     delay(200);
}

// Getters
bool isPresenceDetected() { return _hasTarget; }
bool isMoving() { return (_targetType == 1 || _targetType == 3); }
bool isStationary() { return (_targetType == 2 || _targetType == 3); }
int getDetectionDistance() { return _detDist; }
int getMovingEnergy() { return _movEnergy; }
int getStationaryEnergy() { return _statEnergy; }