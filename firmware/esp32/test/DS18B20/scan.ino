#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 18

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);

  sensors.begin();

  Serial.println("DS18B20 Ready...");
}

void loop() {

  sensors.requestTemperatures();

  float temperature = sensors.getTempCByIndex(0);

  if (temperature == DEVICE_DISCONNECTED_C) {
    Serial.println("Sensor DS18B20 tidak terdeteksi!");
  } else {
    Serial.print("Temperature : ");
    Serial.print(temperature);
    Serial.println(" °C");
  }

  delay(1000);
}
