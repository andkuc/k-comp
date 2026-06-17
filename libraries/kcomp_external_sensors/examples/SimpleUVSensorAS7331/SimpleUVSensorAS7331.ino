#include "kcomp_AS7331.h"

void setup() {
  // Start the Serial Monitor at 9600 baud
  Serial.begin(9600);
  
  // Wait a moment for the Serial Monitor to connect
  delay(1000); 
  
  Serial.println("K-Comp AS7331 Advanced UV Sensor Test");
  Serial.println("-------------------------------------");

  // Attempt to initialize the sensor
  if (initAS7331() == true) {
    Serial.println("AS7331 found and initialized successfully!");
  } else {
    Serial.println("Error: Could not find AS7331. Check your I2C wiring!");
    // Stop the program completely if the sensor isn't found
    while (1) {
      delay(10);
    }
  }

  // --- NEW: Set the Gain ---
  // The default gain is 64x (AS7331_GAIN_64X).
  // If you are measuring dim room light, you can turn the gain UP (e.g., 256x).
  // If you are measuring bright sunlight or a strong UV flashlight, 
  // turn the gain DOWN (e.g., 16x) so the sensor doesn't max out.
  Serial.println("Setting Gain to 256x for better indoor sensitivity...");
  setAS7331Gain(AS7331_GAIN_256X);
  
  Serial.println("Starting measurements...\n");
}

void loop() {
  // Read the UV values. The library automatically adjusts the math 
  // based on the gain we set in setup()!
  float uva = getAS7331_UVA();
  float uvb = getAS7331_UVB();
  float uvc = getAS7331_UVC();
  

  // Print the values to the Serial Monitor
  Serial.print("UVA: ");
  Serial.print(uva);
  Serial.print(" uW/cm2  |  ");

  Serial.print("UVB: ");
  Serial.print(uvb);
  Serial.print(" uW/cm2  |  ");

  Serial.print("UVC: ");
  Serial.print(uvc);
  Serial.println(" uW/cm2");
  Serial.println(getRawUVA());

  // Wait 1 second before taking the next reading
  delay(1000);
}