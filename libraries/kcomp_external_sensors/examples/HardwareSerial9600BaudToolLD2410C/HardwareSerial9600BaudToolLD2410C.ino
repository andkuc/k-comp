/*
 * LD2410_Hardware_Setup_Tool.ino
 * * FORCES the sensor to 9600 Baud using Hardware Serial.
 * * INSTRUCTIONS:
 * 1. Upload with NO SENSOR connected.
 * 2. Connect Sensor: TX -> Pin 0, RX -> Pin 1.
 * 3. Power on (Plug in USB).
 * 4. Wait for LED 5 to turn ON SOLID.
 */

#define LED_PIN 5

// Commands
const uint8_t ENABLE_CONFIG[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0x01, 0x00, 0x00, 0x04, 0x03, 0x02, 0x01};
const uint8_t SET_BAUD_9600[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xA1, 0x00, 0x01, 0x00, 0x04, 0x03, 0x02, 0x01};
const uint8_t RESTART[]       = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xFE, 0x00, 0x04, 0x03, 0x02, 0x01};

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Start with LED OFF

  // 1. Begin High Speed (Factory Default)
  Serial.begin(256000);
  delay(2000); // Wait for sensor boot

  // 2. Enable Configuration
  Serial.write(ENABLE_CONFIG, sizeof(ENABLE_CONFIG));
  delay(100);

  // 3. Set to 9600
  Serial.write(SET_BAUD_9600, sizeof(SET_BAUD_9600));
  delay(100);

  // 4. Restart Sensor
  Serial.write(RESTART, sizeof(RESTART));
  delay(1000);

  // 5. Signal Completion
  digitalWrite(LED_PIN, HIGH); // LED ON = DONE
}

void loop() {
  // Stay ON to indicate success
}