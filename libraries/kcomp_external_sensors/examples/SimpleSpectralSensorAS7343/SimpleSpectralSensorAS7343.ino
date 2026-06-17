#include "kcomp_AS7343.h"

void setup() {
  Serial.begin(115200);
  delay(1000); 

  Serial.println("=========================================");
  Serial.println("   🔬 KComp AS7343 Spectral Sensor 🔬   ");
  Serial.println("=========================================");

  if (initAS7343()) {
    Serial.println("✅ Sensor found and ready to go!");
  } else {
    Serial.println("❌ ERROR: Sensor not found. Are the wires plugged in?");
    while (1) { delay(100); } 
  }

  setAS7343Gain(6); 
  setAS7343LED(true, 12); 
  
  Serial.println("💡 Flashlight is ON! Hold a colored object 2-3cm above the sensor.");
  Serial.println("-----------------------------------------");
}

void loop() {
  if (updateAS7343()) {
    
    AS7343Data lightData = getAS7343Data();

    // The Brightness Reading
    Serial.print("☀️ Total Brightness: ");
    Serial.println(lightData.clear);

    // The Scientist Reading: All the raw hardware channels
    Serial.println("\n🔬 Raw Spectral Data:");
    
    // Violet & Blue
    Serial.print("405nm_F1: "); Serial.print(lightData.f1_405nm); Serial.print("\t");
    Serial.print("425nm_F2: "); Serial.print(lightData.f2_425nm); Serial.print("\t");
    Serial.print("450nm_FZ: "); Serial.print(lightData.fz_450nm); Serial.println();

    // Cyan & Green
    Serial.print("475nm_F3: "); Serial.print(lightData.f3_475nm); Serial.print("\t");
    Serial.print("515nm_F4: "); Serial.print(lightData.f4_515nm); Serial.print("\t");
    Serial.print("550nm_F5: "); Serial.print(lightData.f5_550nm); Serial.println();

    // Yellow, Orange, & Red
    Serial.print("555nm_FY: "); Serial.print(lightData.fy_555nm); Serial.print("\t");
    Serial.print("600nm_FXL:"); Serial.print(lightData.fxl_600nm); Serial.print("\t");
    Serial.print("640nm_F6: "); Serial.print(lightData.f6_640nm); Serial.println();

    // Deep Red & Invisible Infrared
    Serial.print("690nm_F7: "); Serial.print(lightData.f7_690nm); Serial.print("\t");
    Serial.print("745nm_F8: "); Serial.print(lightData.f8_745nm); Serial.print("\t");
    Serial.print("855nm_NIR:"); Serial.print(lightData.nir);    Serial.println();

    Serial.println("-----------------------------------------");
    
  } else {
    Serial.println("⚠️ Oops! Couldn't read the sensor. Is a wire loose?");
  }

  delay(1000); 
}