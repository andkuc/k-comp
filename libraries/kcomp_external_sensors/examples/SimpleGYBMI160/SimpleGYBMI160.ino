#include "kcomp_GYBMI160.h"

/*
 * IMPORTANT: SAMPLING FREQUENCY & ACCURACY
 * * To get accurate Z-Angles (Yaw), readBMI160() must be called frequently.
 * * Why?
 * The library calculates the angle by adding up tiny slices of rotation over time 
 * (Integration). 
 * * - High Frequency (e.g., delay(10)):  Many small, accurate slices = Precise Angle.
 * - Low Frequency  (e.g., delay(500)): Few large, average slices = Chunky/Inaccurate Angle.
 * * Recommendation: Keep your loop delay between 10ms and 50ms for best results.
 */

void setup() {
  Serial.begin(115200); // Use fast serial for smooth data

  Serial.println("Initializing GY-BMI160...");

  if (!initBMI160()) {
    Serial.println("Error: BMI160 not found! Check wiring (SDA/SCL).");
    while(1);
  }

  Serial.println("BMI160 Ready!");
  Serial.println("Pitch/Roll: Tilt the board.");
  Serial.println("Yaw (Z): Rotate the board flat on the table.");
  delay(1000);


  /* * INFO: THE "GYRO DRIFT" PROBLEM
  * * You will notice that 'Yaw(Z)' changes slowly even if the board is still.
  * Why? 
  * The accelerometer cannot see rotation around the Z-axis (gravity doesn't change).
  * So we calculate Z by adding up speed over time (Integration).
  * * Example: "I turned 10 deg/s for 1 second" = 10 degrees total.
  * * However, tiny errors add up. If the sensor thinks it's turning at 0.1 deg/s
  * when it is actually stopped, after 1 minute (60 seconds) it will be wrong by 6 degrees!
  * This is called "Drift." Professional robots fix this with a Compass (Magnetometer).
  */
  resetGyroZ();
  Serial.println("--- YAW RESET TO 0 ---");
  delay(200); // Debounce
}

void loop() {
  // 1. Get Data
  BMI160Data imu = readBMI160();

  // 2. Print Acceleration
  Serial.print("ACC [m/s^2]: ");
  Serial.print("X="); Serial.print(imu.accelX);
  Serial.print(" | Y="); Serial.print(imu.accelY);
  Serial.print(" | Z="); Serial.print(imu.accelZ);
  
  // 3. Print Gyroscope (Rotation Speed)
  Serial.print("   ||   GYRO [deg/s]: ");
  Serial.print("X="); Serial.print(imu.gyroX);
  Serial.print(" | Y="); Serial.print(imu.gyroY);
  Serial.print(" | Z="); Serial.print(imu.gyroZ);

  // 4. Print Angles
    Serial.print("   ||   Angles [deg]: ");
  Serial.print("Pitch(X): "); 
  Serial.print(imu.angleX, 1);
  
  Serial.print(" | Roll(Y): "); 
  Serial.print(imu.angleY, 1);
  
  // The Z-Angle (Yaw)
  Serial.print(" | Yaw(Z): "); 
  Serial.println(imu.angleZ, 1);

  delay(50); // Keep loop fast for accurate integration!
}