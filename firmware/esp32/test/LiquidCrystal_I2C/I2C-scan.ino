#include <Wire.h>

#define I2C_SDA 21
#define I2C_SCL 22

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\nI2C Scanner ESP32");
  Wire.begin(I2C_SDA, I2C_SCL);

  byte error, address;
  int deviceCount = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address: 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      deviceCount++;
    }
  }

  if (deviceCount == 0) {
    Serial.println("❌ No I2C devices found");
  } else {
    Serial.println("✅ Scan selesai");
  }
}

void loop() {
  // kosong
}

// I2C Scanner ESP32
// I2C device found at address: 0x27
// ✅ Scan selesai 
