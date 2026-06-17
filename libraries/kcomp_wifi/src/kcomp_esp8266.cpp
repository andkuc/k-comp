/*
  kcomp_ESP8266.cpp - Implementation
*/

#include "kcomp_ESP8266.h"
#include <SoftwareSerial.h>

// --- Internal State ---
static SoftwareSerial *espSerial = NULL;
static bool _debugMode = true; 

// --- Internal Helper: Send Command & Wait for Response ---
static bool _sendCmd(const char* cmd, const char* expected, int timeout) {
    if (!espSerial) return false;
    
    // Clear buffer
    while (espSerial->available() > 0) espSerial->read();

    // Send Command
    if (cmd && cmd[0] != '\0') {
        espSerial->println(cmd);
        if (_debugMode) {
            Serial.print(F("[ESP Send] "));
            Serial.println(cmd);
        }
    }

    // Wait for Response
    unsigned long start = millis();
    String response = "";
    
    while (millis() - start < timeout) {
        while (espSerial->available() > 0) {
            char c = espSerial->read();
            response += c;
        }
        if (response.indexOf(expected) != -1) {
            if (_debugMode) Serial.println(F("[ESP OK]"));
            return true;
        }
    }
    
    if (_debugMode) {
        Serial.print(F("[ESP Timeout] Got: "));
        Serial.println(response);
    }
    return false;
}

// --- Independent Baud Rate Setup ---
void setupESP8266BaudRate(int rxPin, int txPin) {
    if (espSerial) delete espSerial;
    
    Serial.println(F("Configuring ESP8266 Baud Rate..."));
    
    // 1. Try 9600
    espSerial = new SoftwareSerial(rxPin, txPin);
    espSerial->begin(9600);
    delay(100);
    espSerial->println("AT");
    delay(200);
    
    if (espSerial->find("OK")) {
        Serial.println(F("ESP is already at 9600 baud."));
        return; 
    }
    delete espSerial;

    // 2. Try 115200 (Factory Default)
    Serial.println(F("Attempting 115200 negotiation..."));
    espSerial = new SoftwareSerial(rxPin, txPin);
    espSerial->begin(115200);
    delay(100);
    
    for(int i=0; i<5; i++) {
        espSerial->println("AT+UART_DEF=9600,8,1,0,0");
        delay(200);
    }
    delete espSerial;

    // 3. Verify
    espSerial = new SoftwareSerial(rxPin, txPin);
    espSerial->begin(9600);
    delay(100);
    espSerial->println("AT");
    delay(500);
    
    if (espSerial->find("OK")) {
        Serial.println(F("Success! ESP8266 set to 9600 baud."));
    } else {
        Serial.println(F("Failed. Check wiring (RX/TX swapped?)."));
    }
    delete espSerial;
    espSerial = NULL;
}

// --- Public Functions ---

bool beginESP8266(int rxPin, int txPin, const char* ssid, const char* pass) {
    if (espSerial != NULL) delete espSerial;
    espSerial = new SoftwareSerial(rxPin, txPin);
    espSerial->begin(9600); 
    
    if (!_sendCmd("AT", "OK", 1000)) return false;
    _sendCmd("AT+CWMODE=1", "OK", 2000);

    String cmd = "AT+CWJAP=\"";
    cmd += ssid;
    cmd += "\",\"";
    cmd += pass;
    cmd += "\"";
    
    return _sendCmd(cmd.c_str(), "OK", 15000); 
}

bool syncESP8266Time(int timezone) {
    if (!espSerial) return false;

    if (_debugMode) Serial.println(F("--- Syncing Time (NTP) ---"));

    // 1. Enable SNTP (Timezone in hours)
    // AT+CIPSNTPCFG=1,timezone
    String cmd = "AT+CIPSNTPCFG=1,";
    cmd += timezone;
    _sendCmd(cmd.c_str(), "OK", 5000);

    // 2. Wait for time to update from 1970 -> 202X
    if (_debugMode) Serial.println(F("Waiting for NTP..."));
    
    for (int i = 0; i < 15; i++) { // Wait up to 15 seconds
        espSerial->println("AT+CIPSNTPTIME?");
        
        // Manual read to check year
        unsigned long start = millis();
        String resp = "";
        while (millis() - start < 1000) {
            while (espSerial->available()) resp += (char)espSerial->read();
        }

        // Check for current decade (2020-2029)
        if (resp.indexOf("202") != -1) {
            if (_debugMode) {
                Serial.print(F("Time Synced: "));
                Serial.println(resp);
            }
            return true;
        }
    }
    
    if (_debugMode) Serial.println(F("Time Sync Failed (Proceeding anyway)"));
    return false;
}

bool setSSLBufferSize(uint16_t size) {
    if (!espSerial) return false;
    String cmd = "AT+CIPSSLSIZE=";
    cmd += size;
    return _sendCmd(cmd.c_str(), "OK", 2000);
}

void printESP8266WifiStatus(void) {
    if (!espSerial) return;
    Serial.println(F("--- WiFi Status ---"));
    espSerial->println("AT+CIFSR");
    unsigned long start = millis();
    while (millis() - start < 2000) {
        while (espSerial->available()) Serial.write(espSerial->read());
    }
    Serial.println(F("\n-------------------"));
}

bool connectESP8266Server(const char* domain, int port) {
    if (!espSerial) return false;

    _sendCmd("AT+CIPCLOSE", "ERROR", 500); 

    String type = "TCP";
    if (port == 443) type = "SSL";

    String cmd = "AT+CIPSTART=\"";
    cmd += type;
    cmd += "\",\"";
    cmd += domain;
    cmd += "\",";
    cmd += port;

    // Increased timeout to 15s for slow SSL handshakes
    return _sendCmd(cmd.c_str(), "OK", 15000);
}

void printlnESP8266(const char* data) {
    if (!espSerial) return;

    int len = strlen(data) + 2; 
    espSerial->print("AT+CIPSEND=");
    espSerial->println(len);

    if (espSerial->find(">")) {
        espSerial->print(data);
        espSerial->print("\r\n");
        
        unsigned long start = millis();
        while(millis() - start < 2000) {
            if(espSerial->find("SEND OK")) break;
        }
    } else {
        if (_debugMode) Serial.println(F("[ESP Error] Send Timeout"));
    }
}

int readESP8266(void) {
    if (espSerial && espSerial->available()) return espSerial->read();
    return -1;
}

int availableESP8266(void) {
    if (espSerial) return espSerial->available();
    return 0;
}

void stopESP8266(void) {
    if (espSerial) espSerial->println("AT+CIPCLOSE");
}