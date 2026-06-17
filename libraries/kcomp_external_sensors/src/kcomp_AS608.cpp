#include "kcomp_AS608.h"
#include <SoftwareSerial.h>

static SoftwareSerial* as608_serial = NULL;

// --- AS608 Hexadecimal Instructions ---
#define CMD_VFY_PWD    0x13
#define CMD_GEN_IMG    0x01
#define CMD_IMG_2_TZ   0x02
#define CMD_SEARCH     0x04
#define CMD_REG_MODEL  0x05
#define CMD_STORE      0x06
#define CMD_EMPTY      0x0C

#define AS608_TIMEOUT  1000 // Milliseconds before giving up on a response


#define CMD_DELETE     0x0C
#define CMD_EMPTY      0x0D // Fixed from previous version!
#define CMD_TEMPLATE   0x1D
#define CMD_LOAD_CHAR  0x07
#define CMD_UP_CHAR    0x08
#define CMD_LED        0x35

// ==============================================================================
// RAW PROTOCOL ENGINE
// ==============================================================================

// Builds a data packet and sends it over the serial line
static void writePacket(uint8_t instruction, uint8_t* args, uint8_t argLen) {
    // BUGFIX: Length must include the Instruction (1), Args (argLen), and Checksum (2)
    uint16_t wireLen = argLen + 3; 
    
    uint16_t checksum = 0x01 + (wireLen >> 8) + (wireLen & 0xFF) + instruction;
    
    // 1. Header & Default Address
    uint8_t header[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF};
    as608_serial->write(header, 6);
    
    // 2. Packet Identifier (0x01 for Command) & Length
    as608_serial->write(0x01);
    as608_serial->write(wireLen >> 8);
    as608_serial->write(wireLen & 0xFF);
    
    // 3. Instruction
    as608_serial->write(instruction);
    
    // 4. Arguments & Checksum Calculation
    for (uint8_t i = 0; i < argLen; i++) {
        as608_serial->write(args[i]);
        checksum += args[i];
    }
    
    // 5. Checksum
    as608_serial->write(checksum >> 8);
    as608_serial->write(checksum & 0xFF);
}

// State-machine parser to read the incoming hexadecimal response
// Returns the Confirmation Code (0 = Success). Data payload is written into rxData.
static int16_t readAck(uint8_t* rxData, uint8_t maxLen) {
    long start = millis();
    uint8_t state = 0;
    uint16_t len = 0;
    uint16_t checksum = 0;
    uint16_t calcChecksum = 0;
    uint8_t dataIdx = 0;

    while (millis() - start < AS608_TIMEOUT) {
        if (as608_serial->available()) {
            uint8_t b = as608_serial->read();
            switch(state) {
                case 0: if (b == 0xEF) state = 1; break;
                case 1: if (b == 0x01) state = 2; else state = 0; break;
                case 2: case 3: case 4: 
                case 5: if (b == 0xFF) state++; else state = 0; break;
                case 6: 
                    if (b == 0x07) { state = 7; calcChecksum = b; } // 0x07 = Acknowledge Packet
                    else state = 0; 
                    break;
                case 7: len = b << 8; calcChecksum += b; state = 8; break;
                case 8: len |= b; calcChecksum += b; state = 9; len -= 2; break; 
                case 9:
                    rxData[dataIdx++] = b;
                    calcChecksum += b;
                    if (dataIdx >= len || dataIdx >= maxLen) state = 10;
                    break;
                case 10: checksum = b << 8; state = 11; break;
                case 11:
                    checksum |= b;
                    if (checksum == calcChecksum) return rxData[0]; // rxData[0] is the confirmation code
                    return AS608_COMM_ERROR; // Checksum mismatch
            }
        }
    }
    return AS608_COMM_ERROR; // Timeout
}

// ==============================================================================
//  FLAT API FUNCTIONS
// ==============================================================================


bool initAS608(uint8_t rxPin, uint8_t txPin, uint32_t baudRate) {
    if (as608_serial != NULL) { delete as608_serial; }
    
    as608_serial = new SoftwareSerial(rxPin, txPin);
    as608_serial->begin(baudRate);
    
    // BUGFIX: Give the AS608 hardware 250 milliseconds to wake up 
    // and stabilize its serial bus before we throw the password at it.
    delay(250); 
    
    return checkAS608();
}

bool checkAS608() {
    if (as608_serial == NULL) return false;
    
    // Clear incoming buffer
    while (as608_serial->available()) as608_serial->read();
    
    // Send Verify Password Command (Default Password is 0)
    uint8_t pwd[] = {0x00, 0x00, 0x00, 0x00};
    writePacket(CMD_VFY_PWD, pwd, 4);
    
    uint8_t response[4];
    return (readAck(response, 4) == 0x00);
}

int16_t scanFingerprint() {
    uint8_t response[8];
    
    // 1. Take a picture
    writePacket(CMD_GEN_IMG, NULL, 0);
    int16_t status = readAck(response, 4);
    if (status == 0x02) return AS608_NO_FINGER;
    if (status != 0x00) return AS608_IMAGE_FAIL;
    
    // 2. Convert picture to mathematical model in Buffer 1
    uint8_t buf1[] = {0x01};
    writePacket(CMD_IMG_2_TZ, buf1, 1);
    if (readAck(response, 4) != 0x00) return AS608_IMAGE_FAIL;
    
    // 3. Search database for the model in Buffer 1
    // Args: BufferID (1), StartPage (0), NumPages (200)
    uint8_t searchArgs[] = {0x01, 0x00, 0x00, 0x00, 0xC8}; 
    writePacket(CMD_SEARCH, searchArgs, 5);
    status = readAck(response, 8);
    
    if (status == 0x00) {
        // response[1] and response[2] contain the matched ID
        return (response[1] << 8) | response[2];
    }
    
    return AS608_MATCH_FAIL;
}

int8_t enrollFingerprint(uint8_t target_id) {
    uint8_t response[4];
    int16_t status = -1;
    
    // --- Step 1: First Scan ---
    Serial.println("Waiting for valid finger to enroll...");
    while (status != 0x00) {
        writePacket(CMD_GEN_IMG, NULL, 0);
        status = readAck(response, 4);
    }
    
    uint8_t buf1[] = {0x01};
    writePacket(CMD_IMG_2_TZ, buf1, 1);
    if (readAck(response, 4) != 0x00) return AS608_IMAGE_FAIL;

    // --- Wait for removal ---
    Serial.println("Remove finger...");
    status = 0x00;
    while (status != 0x02) { // 0x02 = No Finger Detected
        writePacket(CMD_GEN_IMG, NULL, 0);
        status = readAck(response, 4);
    }
    delay(500);

    // --- Step 2: Second Scan ---
    Serial.println("Place same finger again...");
    status = -1;
    while (status != 0x00) {
        writePacket(CMD_GEN_IMG, NULL, 0);
        status = readAck(response, 4);
    }
    
    uint8_t buf2[] = {0x02};
    writePacket(CMD_IMG_2_TZ, buf2, 1); // Save to Buffer 2
    if (readAck(response, 4) != 0x00) return AS608_IMAGE_FAIL;

    // --- Step 3: Combine Buffers into a Template ---
    Serial.println("Creating Template...");
    writePacket(CMD_REG_MODEL, NULL, 0);
    if (readAck(response, 4) != 0x00) return AS608_MATCH_FAIL;

    // --- Step 4: Store Template to Flash ---
    uint8_t storeArgs[] = {0x01, (uint8_t)(target_id >> 8), (uint8_t)(target_id & 0xFF)};
    writePacket(CMD_STORE, storeArgs, 3);
    if (readAck(response, 4) == 0x00) {
        Serial.println("Stored Successfully!");
        return AS608_OK;
    }

    return AS608_ENROLL_FAIL;
}

bool clearAllFingerprints() {
    uint8_t response[4];
    writePacket(CMD_EMPTY, NULL, 0);
    return (readAck(response, 4) == 0x00);
}



// ==============================================================================
//  ADVANCED DATABASE MANAGEMENT
// ==============================================================================

int8_t deleteFingerprint(uint16_t id) {
    uint8_t response[4];
    // Args: PageID (2 bytes), Number of templates to delete (2 bytes, we use 1)
    uint8_t args[] = {(uint8_t)(id >> 8), (uint8_t)(id & 0xFF), 0x00, 0x01};
    
    writePacket(CMD_DELETE, args, 4);
    if (readAck(response, 4) == 0x00) return AS608_OK;
    return AS608_COMM_ERROR;
}

int16_t getFingerprintCount() {
    uint8_t response[4];
    writePacket(CMD_TEMPLATE, NULL, 0);
    
    if (readAck(response, 4) == 0x00) {
        // The sensor replies with Status (0x00) followed by 2 bytes representing the count
        return (response[1] << 8) | response[2];
    }
    return -1; // Error
}

bool isSlotOccupied(uint16_t id) {
    uint8_t response[4];
    // We test the slot by attempting to load the template from flash into Buffer 1
    // Args: BufferID (1), PageID (2 bytes)
    uint8_t args[] = {0x01, (uint8_t)(id >> 8), (uint8_t)(id & 0xFF)};
    
    writePacket(CMD_LOAD_CHAR, args, 3);
    
    // If it returns 0x00, the load was successful, meaning the slot is occupied.
    return (readAck(response, 4) == 0x00);
}

// ==============================================================================
// 💡 HARDWARE CONTROL
// ==============================================================================

void controlLED(uint8_t mode, uint8_t color, uint8_t speed, uint8_t cycles) {
    uint8_t response[4];
    uint8_t args[] = {mode, speed, color, cycles};
    writePacket(CMD_LED, args, 4);
    readAck(response, 4); // We don't strictly need to check the return here
}

// ==============================================================================
// 🧬 TEMPLATE EXTRACTION (512 Bytes)
// ==============================================================================

bool downloadTemplate(uint16_t id, uint8_t* buffer) {
    uint8_t response[4];
    
    // 1. Load the template from the database into Buffer 1
    uint8_t loadArgs[] = {0x01, (uint8_t)(id >> 8), (uint8_t)(id & 0xFF)};
    writePacket(CMD_LOAD_CHAR, loadArgs, 3);
    if (readAck(response, 4) != 0x00) return false; // Slot is likely empty
    
    // 2. Command the sensor to transmit Buffer 1 to the Arduino
    uint8_t upArgs[] = {0x01};
    writePacket(CMD_UP_CHAR, upArgs, 1);
    if (readAck(response, 4) != 0x00) return false;
    
    // 3. Receive Data Packets
    // The sensor sends the 512 bytes in chunks (usually four 128-byte data packets).
    // This is a brutal blocking loop that extracts the raw payload from the hex wrappers.
    uint16_t bytesRead = 0;
    long start = millis();
    
    while (bytesRead < 512 && (millis() - start < 3000)) {
        if (as608_serial->available() >= 9) { // Wait for packet header
            if (as608_serial->read() == 0xEF && as608_serial->read() == 0x01) {
                // Skip address (4 bytes)
                for(int i=0; i<4; i++) as608_serial->read(); 
                
                uint8_t packetType = as608_serial->read(); // 0x02 = Data, 0x08 = End of Data
                
                // Read Length
                uint16_t len = (as608_serial->read() << 8) | as608_serial->read();
                len -= 2; // Subtract the 2 checksum bytes at the end
                
                // Read the actual mathematical payload
                for (int i = 0; i < len; i++) {
                    while(!as608_serial->available()); // Wait for byte
                    if (bytesRead < 512) {
                        buffer[bytesRead++] = as608_serial->read();
                    } else {
                        as608_serial->read(); // Dump overflow
                    }
                }
                
                // Dump the 2 checksum bytes
                while(as608_serial->available() < 2);
                as608_serial->read(); as608_serial->read();
                
                if (packetType == 0x08) break; // Reached the end packet
            }
        }
    }
    return (bytesRead == 512);
}