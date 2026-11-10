#include <Wire.h>

#define MPU_ADDR 0x68
#define SDA_PIN 21
#define SCL_PIN 22

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("Starting MPU-6050...");

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  // Check connection
  Wire.beginTransmission(MPU_ADDR);

  if (Wire.endTransmission() != 0) {
    Serial.println("ERROR: MPU-6050 not found!");
    Serial.println("Check wiring and sensor address.");

    while (true) {
      delay(1000);
    }
  }

  // Wake up sensor
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  delay(100);

  // Accelerometer range: +/- 2g
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C);
  Wire.write(0x00);
  Wire.endTransmission();

  // Gyroscope range: +/- 250 deg/s
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission();

  Serial.println("MPU-6050 initialized successfully!");
  Serial.println();
}

void loop() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);

  if (Wire.endTransmission(false) != 0) {
    Serial.println("ERROR: Sensor communication failed");
    delay(500);
    return;
  }

  if (Wire.requestFrom(MPU_ADDR, 14) != 14) {
    Serial.println("ERROR: Incomplete sensor data");
    delay(500);
    return;
  }

  int16_t axRaw = (Wire.read() << 8) | Wire.read();
  int16_t ayRaw = (Wire.read() << 8) | Wire.read();
  int16_t azRaw = (Wire.read() << 8) | Wire.read();

  int16_t tempRaw = (Wire.read() << 8) | Wire.read();

  int16_t gxRaw = (Wire.read() << 8) | Wire.read();
  int16_t gyRaw = (Wire.read() << 8) | Wire.read();
  int16_t gzRaw = (Wire.read() << 8) | Wire.read();

  // Convert raw values
  float ax = axRaw * 9.80665 / 16384.0;
  float ay = ayRaw * 9.80665 / 16384.0;
  float az = azRaw * 9.80665 / 16384.0;

  float gx = gxRaw / 131.0;
  float gy = gyRaw / 131.0;
  float gz = gzRaw / 131.0;

  float temperature = tempRaw / 340.0 + 36.53;

  // Print each measurement on a separate line
  Serial.println("----- MPU-6050 DATA -----");

  Serial.print("Accel X: ");
  Serial.println(ax, 3);

  Serial.print("Accel Y: ");
  Serial.println(ay, 3);

  Serial.print("Accel Z: ");
  Serial.println(az, 3);

  Serial.print("Gyro X:  ");
  Serial.println(gx, 3);

  Serial.print("Gyro Y:  ");
  Serial.println(gy, 3);

  Serial.print("Gyro Z:  ");
  Serial.println(gz, 3);

  Serial.print("Temp C:  ");
  Serial.println(temperature, 2);

  Serial.println("------------------------");
  Serial.println();

  delay(500);
}