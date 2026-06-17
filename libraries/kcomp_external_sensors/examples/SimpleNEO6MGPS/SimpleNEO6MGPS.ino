/*
 * KComp NEO-6M GPS Full Example
 * Demonstrates:
 * 1. Initializing GPS
 * 2. Configuring Timezone (CET example)
 * 3. Reading all Position/Signal data
 * 4. Calculating Distance/Course to a destination
 */

#include "kcomp_NEO6MGPS.h"

// --- Hardware Pins ---
#define GPS_RX_PIN 6
#define GPS_TX_PIN 7
#define GPS_BAUD   9600

// --- Destination (Example: London, UK) ---
// Change these to your own target coordinates!
#define DEST_LAT 51.5074
#define DEST_LON -0.1278

// --- Helper: Convert Cardinal Enum to Text ---
void printCardinalText(GPSCardinal c) {
  switch(c) {
    case CARDINAL_N:  Serial.print("North"); break;
    case CARDINAL_NE: Serial.print("North-East"); break;
    case CARDINAL_E:  Serial.print("East"); break;
    case CARDINAL_SE: Serial.print("South-East"); break;
    case CARDINAL_S:  Serial.print("South"); break;
    case CARDINAL_SW: Serial.print("South-West"); break;
    case CARDINAL_W:  Serial.print("West"); break;
    case CARDINAL_NW: Serial.print("North-West"); break;
    default:          Serial.print("Unknown"); break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("--- KComp GPS Library Demo ---");

  // 1. Initialize the GPS
  if (!initNEO6MGPS(GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD)) {
    Serial.println("CRITICAL ERROR: GPS Init Failed!");
    while(1);
  }
  
  // 2. Configure Timezone
  // Example: Austria/Germany (CET)
  // Standard Offset = +1 Hour
  setGPSOffset(1); 
  
  // Is it Summer? (Daylight Saving Time)
  // Set true for Summer (+1h), false for Winter.
  setGPSDST(false); 

  Serial.println("GPS Initialized. Waiting for Satellite Fix...");
}

void loop() {
  // -------------------------------------------------------------
  // 1. Smart Delay Loop
  // -------------------------------------------------------------
  // We must constantly read from the GPS for 1 second.
  // Do NOT use delay(1000) or you will lose data!
  unsigned long start = millis();
  while (millis() - start < 1000) {
    updateGPS(); 
  }

  // -------------------------------------------------------------
  // 2. Get and Print Data
  // -------------------------------------------------------------
  GPSData gps = getGPSData();

  Serial.println("\n-------------------------------------------");

  if (gps.valid) {
    Serial.println("[ STATUS: FIX ACTIVE ]");

    // --- Time & Date (Local) ---
    Serial.print("Local Time:   ");
    if(gps.hour < 10) Serial.print("0"); Serial.print(gps.hour); Serial.print(":");
    if(gps.minute < 10) Serial.print("0"); Serial.print(gps.minute); Serial.print(":");
    if(gps.second < 10) Serial.print("0"); Serial.println(gps.second);
    
    Serial.print("Date:         ");
    if(gps.day < 10) Serial.print("0"); Serial.print(gps.day); Serial.print(".");
    if(gps.month < 10) Serial.print("0"); Serial.print(gps.month); Serial.print(".");
    Serial.println(gps.year);
    
    // --- Position ---
    Serial.print("Latitude:     "); Serial.print(gps.latitude, 6); Serial.println(" deg");
    Serial.print("Longitude:    "); Serial.print(gps.longitude, 6); Serial.println(" deg");
    Serial.print("Altitude:     "); Serial.print(gps.altitude); Serial.println(" m");
    
    // --- Motion ---
    Serial.print("Speed:        "); Serial.print(gps.speed); Serial.println(" km/h");
    Serial.print("Heading:      "); Serial.print(gps.course); Serial.print(" deg (");
    printCardinalText(gps_cardinal(gps.course));
    Serial.println(")");

    // --- Signal ---
    Serial.print("Satellites:   "); Serial.println(gps.satellites);
    Serial.print("Accuracy:     "); Serial.print(gps.hdop); Serial.println(" (HDOP)");

    // --- Navigation Demo ---
    double dist = gps_distanceBetween(gps.latitude, gps.longitude, DEST_LAT, DEST_LON);
    double bear = gps_courseTo(gps.latitude, gps.longitude, DEST_LAT, DEST_LON);
    
    Serial.println("- - - Target: London - - -");
    Serial.print("Distance:     "); Serial.print(dist / 1000.0); Serial.println(" km");
    Serial.print("Bearing:      "); Serial.print(bear); Serial.println(" deg");

  } else {
    // No valid data yet
    Serial.println("[ STATUS: SEARCHING... ]");
    Serial.print("Satellites:   "); Serial.println(gps.satellites);
    Serial.println("Please go outside for a clear sky view.");
  }
}