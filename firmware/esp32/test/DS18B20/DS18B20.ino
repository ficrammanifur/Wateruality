/*
  DS18B20 ESP32
  GPIO 34

  Library yang dibutuhkan:
  - OneWire
  - DallasTemperature
*/

#include <OneWire.h>
#include <DallasTemperature.h>

// =============================
// Konfigurasi
// =============================
#define DS18B20_PIN 34

OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

DeviceAddress sensorAddress;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("========================================");
  Serial.printf("Memulai inisialisasi sensor DS18B20 pada GPIO %d...\n", DS18B20_PIN);

  sensors.begin();

  int jumlahSensor = sensors.getDeviceCount();

  if (jumlahSensor == 0) {
    Serial.println("========================================");
    Serial.println("⚠️  PERINGATAN PENTING ⚠️");
    Serial.println("Tidak ada sensor DS18B20 yang terdeteksi!");
    Serial.println("Mohon periksa:");
    Serial.println("1. Resistor Pull-Up 4.7kΩ antara DATA dan 3.3V");
    Serial.println("2. Sambungan kabel VCC, GND, DATA");
    Serial.println("3. Sensor masih berfungsi");
    Serial.println("========================================");

    while (true) {
      delay(1000);
    }
  }

  Serial.printf("✅ Berhasil! Ditemukan %d sensor DS18B20\n", jumlahSensor);

  for (int i = 0; i < jumlahSensor; i++) {
    if (sensors.getAddress(sensorAddress, i)) {
      Serial.print("Sensor ");
      Serial.print(i + 1);
      Serial.print(" Address: ");

      for (uint8_t j = 0; j < 8; j++) {
        if (sensorAddress[j] < 16) Serial.print("0");
        Serial.print(sensorAddress[j], HEX);
      }

      Serial.println();
    }
  }

  sensors.setResolution(12);

  Serial.println("------------------------------");
  Serial.println("Memulai pembacaan suhu...");
}

void loop() {

  sensors.requestTemperatures();

  int jumlahSensor = sensors.getDeviceCount();

  for (int i = 0; i < jumlahSensor; i++) {

    float suhu = sensors.getTempCByIndex(i);

    if (suhu == DEVICE_DISCONNECTED_C) {
      Serial.print("Sensor ");
      Serial.print(i + 1);
      Serial.println(" : ERROR membaca suhu!");
    } else {
      Serial.print("Sensor ");
      Serial.print(i + 1);
      Serial.print(" : ");
      Serial.print(suhu, 2);
      Serial.println(" °C");
    }
  }

  Serial.println("------------------------------");

  delay(2000);
}
