#include "kcomp_PN532.h"
#include <Wire.h>

#define PN532_I2C_ADDRESS 0x24

// --- Internal Command Codes ---
#define CMD_SAMCONFIGURATION    0x14
#define CMD_INLISTPASSIVETARGET 0x4A
#define CMD_INDATAEXCHANGE      0x40

// --- Hidden Internal State ---
typedef struct {
    uint8_t uid[7];        
    uint8_t uidLength;     
    uint8_t type;          
} nfc_tag_t;


static nfc_tag_t activeTag; 
static uint8_t mifareKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

// ==============================================================================
// ⚙️ RAW I2C FRAME ENGINE (WITH SAFE POLLING)
// ==============================================================================

static void writeCommand(const uint8_t* cmd, uint8_t cmdLen) {
    uint8_t checksum = 0;
    
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    
    Wire.write(0x00); // Preamble
    Wire.write(0x00); // Start code 1
    Wire.write(0xFF); // Start code 2
    
    uint8_t len = cmdLen + 1; 
    Wire.write(len);
    Wire.write((uint8_t)(~len + 1)); // Length Checksum
    
    Wire.write(0xD4); // TFI (Host to PN532)
    checksum += 0xD4;
    
    for (uint8_t i = 0; i < cmdLen; i++) {
        Wire.write(cmd[i]);
        checksum += cmd[i];
    }
    
    Wire.write((uint8_t)(~checksum + 1)); // Data Checksum 
    Wire.write(0x00); // Postamble
    
    Wire.endTransmission();
}

static bool waitUntilReady(uint16_t timeout) {
    long start = millis();
    while (millis() - start < timeout) {
        
        // SAFE POLLING: Request exactly 1 byte.
        Wire.requestFrom(PN532_I2C_ADDRESS, 1); 
        
        if (Wire.read() == 0x01) {
            return true; // 0x01 means the chip is ready!
        }
        delay(5); // Give the PN532 internal processor time to breathe
    }
    return false; 
}

static bool readAck() {
    if (!waitUntilReady(150)) return false; 
    
    Wire.requestFrom(PN532_I2C_ADDRESS, 7); 
    
    if (Wire.read() != 0x01) return false; 
    
    const uint8_t ack_frame[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
    for (int i = 0; i < 6; i++) {
        if (Wire.read() != ack_frame[i]) return false;
    }
    return true;
}

static int8_t readResponse(uint8_t* buffer, uint8_t maxLen) {
    if (!waitUntilReady(1000)) return PN532_TIMEOUT;
    
    Wire.requestFrom(PN532_I2C_ADDRESS, maxLen + 9); 
    
    if (Wire.read() != 0x01) return PN532_ERROR; 
    
    if (Wire.read() != 0x00 || Wire.read() != 0x00 || Wire.read() != 0xFF) return PN532_ERROR;
    
    uint8_t len = Wire.read();
    uint8_t lcs = Wire.read();
    if ((uint8_t)(len + lcs) != 0x00) return PN532_ERROR; 
    
    uint8_t tfi = Wire.read(); 
    if (tfi != 0xD5) return PN532_ERROR; 
    
    len--; 
    
    for (uint8_t i = 0; i < len; i++) {
        if (i < maxLen) buffer[i] = Wire.read();
        else Wire.read(); // Dump buffer overflow safely
    }
    
    Wire.read(); // DCS 
    Wire.read(); // Postamble
    
    return len; 
}

// ==============================================================================
// 🚀 INITIALIZATION
// ==============================================================================

bool initPN532() {
    Wire.begin();
    
    
    delay(10); 
    
    // THE WAKEUP PING: Essential for budget clones
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.endTransmission();
    delay(10);
    
    uint8_t samCmd[] = {CMD_SAMCONFIGURATION, 0x01, 0x14, 0x01};
    writeCommand(samCmd, 4);
    
    if (!readAck()) return false;
    
    uint8_t response[8];
    readResponse(response, 8); // Clear response from buffer
    
    return true;
}

// ==============================================================================
// 📡 THE DUAL-ARCHITECTURE SCANNER
// ==============================================================================

int8_t scanForTag() {
    // 1. Send the command to look for 1 passive target at 106 kbps (Type A)
    uint8_t scanCmd[] = {CMD_INLISTPASSIVETARGET, 0x01, 0x00};
    writeCommand(scanCmd, 3);
    
    if (!readAck()) return PN532_ERROR;

    // Ask the bus if it's ready, but don't block for more than 50ms.
    if (!waitUntilReady(50)) return PN532_NO_CARD;
    

    // 3. Extract the Payload
    uint8_t response[20];
    int8_t len = readResponse(response, 20);
    
    // Validate the response header and ensure at least 1 tag was found
    if (len < 0 || response[0] != CMD_INLISTPASSIVETARGET + 1) return PN532_ERROR;
    if (response[1] == 0) return PN532_NO_CARD; 
    
    // 4. Parse the Data (Corrected Indexing)
    // response[0] = 0x4B
    // response[1] = Number of Tags (0x01)
    // response[2] = Target Number (0x01)
    // response[3] = ATQA MSB
    // response[4] = ATQA LSB
    
    uint8_t sak = response[5]; 
    activeTag.uidLength = response[6];
    
    // Safety bounds check to prevent memory overflow if the clone sends garbage
    if (activeTag.uidLength > 7) activeTag.uidLength = 7; 
    
    // The actual Serial Number starts at index 7
    for (uint8_t i = 0; i < activeTag.uidLength; i++) {
        activeTag.uid[i] = response[7 + i];
    }
    
    // 5. Decode the Architecture
    if (sak == 0x08 || sak == 0x18) {
        activeTag.type = CARD_MIFARE_CLASSIC;
    } else if (sak == 0x00) {
        activeTag.type = CARD_NTAG;
    } else {
        activeTag.type = CARD_UNKNOWN;
    }

    return PN532_OK;
}
// ==============================================================================
// 🔍 GETTERS
// ==============================================================================

uint8_t getTagType() { return activeTag.type; }
uint8_t getUidLength() { return activeTag.uidLength; }

void getUid(uint8_t uidArray[]) {
    for (int i = 0; i < activeTag.uidLength; i++) {
        uidArray[i] = activeTag.uid[i];
    }
}

void setMifareKey(uint8_t keyArray[]) {
    for (int i = 0; i < 6; i++) mifareKey[i] = keyArray[i];
}

// ==============================================================================
// 🔐 MIFARE AUTHENTICATION ENGINE
// ==============================================================================

static bool authenticateMifare(uint8_t blockNumber) {
    uint8_t uid[4];
    for(int i=0; i<4; i++) uid[i] = activeTag.uid[i];

    uint8_t authCmd[15] = {
        CMD_INDATAEXCHANGE, 0x01, 0x60, blockNumber,
        mifareKey[0], mifareKey[1], mifareKey[2], 
        mifareKey[3], mifareKey[4], mifareKey[5],
        uid[0], uid[1], uid[2], uid[3]
    };

    writeCommand(authCmd, 15);
    if (!readAck()) return false;

    uint8_t response[8];
    int8_t len = readResponse(response, 8);
    
    if (len < 0 || response[0] != CMD_INDATAEXCHANGE + 1 || response[1] != 0x00) {
        return false; 
    }
    return true;
}

// ==============================================================================
// 🧠 UNIFIED MEMORY ROUTER (READ)
// ==============================================================================

int8_t readTagMemory(uint8_t slot, uint8_t bufferArray[]) {
    if (activeTag.type == CARD_UNKNOWN) return PN532_NO_CARD;

    if (activeTag.type == CARD_MIFARE_CLASSIC) {
        if (!authenticateMifare(slot)) return PN532_AUTH_FAIL;

        uint8_t readCmd[] = {CMD_INDATAEXCHANGE, 0x01, 0x30, slot};
        writeCommand(readCmd, 4);
        if (!readAck()) return PN532_ERROR;

        uint8_t response[24];
        int8_t len = readResponse(response, 24);
        
        if (len < 17 || response[1] != 0x00) return PN532_ERROR;

        for (int i = 0; i < 16; i++) {
            bufferArray[i] = response[2 + i]; 
        }
        return PN532_OK;
    }
    
    else if (activeTag.type == CARD_NTAG) {
        uint8_t readCmd[] = {CMD_INDATAEXCHANGE, 0x01, 0x30, slot};
        writeCommand(readCmd, 4);
        if (!readAck()) return PN532_ERROR;

        uint8_t response[24];
        int8_t len = readResponse(response, 24);

        if (len < 17 || response[1] != 0x00) return PN532_ERROR;

        for (int i = 0; i < 16; i++) {
            bufferArray[i] = response[2 + i];
        }
        return PN532_OK;
    }

    return PN532_ERROR;
}

// ==============================================================================
// ✍️ UNIFIED MEMORY ROUTER (WRITE)
// ==============================================================================

int8_t writeTagMemory(uint8_t slot, uint8_t dataArray[]) {
    if (activeTag.type == CARD_UNKNOWN) return PN532_NO_CARD;

    if (activeTag.type == CARD_MIFARE_CLASSIC) {
        if (!authenticateMifare(slot)) return PN532_AUTH_FAIL;

        uint8_t writeCmd[20] = {CMD_INDATAEXCHANGE, 0x01, 0xA0, slot};
        for (int i = 0; i < 16; i++) {
            writeCmd[4 + i] = dataArray[i];
        }

        writeCommand(writeCmd, 20);
        if (!readAck()) return PN532_ERROR;

        uint8_t response[8];
        int8_t len = readResponse(response, 8);
        if (len < 0 || response[1] != 0x00) return PN532_ERROR;

        return PN532_OK;
    }
    
    else if (activeTag.type == CARD_NTAG) {
        for (uint8_t pageOffset = 0; pageOffset < 4; pageOffset++) {
            uint8_t writeCmd[8] = {
                CMD_INDATAEXCHANGE, 0x01, 0xA2, 
                (uint8_t)(slot + pageOffset), 
                dataArray[pageOffset * 4 + 0],
                dataArray[pageOffset * 4 + 1],
                dataArray[pageOffset * 4 + 2],
                dataArray[pageOffset * 4 + 3]
            };

            writeCommand(writeCmd, 8);
            if (!readAck()) return PN532_ERROR;

            uint8_t response[8];
            int8_t len = readResponse(response, 8);
            if (len < 0 || response[1] != 0x00) return PN532_ERROR;
        }
        return PN532_OK;
    }

    return PN532_ERROR;
}