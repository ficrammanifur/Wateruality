/*
  ESP32 - TDS Sensor
  Pin Analog : GPIO 33
*/

#define TDS_PIN 33
#define VREF 3.3          // Tegangan ADC ESP32
#define ADC_RESOLUTION 4095.0

void setup() {
  Serial.begin(115200);

  analogReadResolution(12); // ADC 12-bit

  Serial.println("=================================");
  Serial.println("TDS Sensor ESP32");
  Serial.println("GPIO : 33");
  Serial.println("=================================");
}

void loop() {

  // Baca ADC
  int adcValue = analogRead(TDS_PIN);

  // Konversi ke tegangan
  float voltage = adcValue * VREF / ADC_RESOLUTION;

  // Faktor kompensasi suhu (25°C)
  float compensationCoefficient = 1.0;
  float compensationVoltage = voltage / compensationCoefficient;

  // Rumus dari DFRobot
  float tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage
                  - 255.86 * compensationVoltage * compensationVoltage
                  + 857.39 * compensationVoltage)
                  * 0.5;

  Serial.print("ADC      : ");
  Serial.print(adcValue);

  Serial.print(" | Voltage : ");
  Serial.print(voltage, 3);
  Serial.print(" V");

  Serial.print(" | TDS : ");
  Serial.print(tdsValue, 2);
  Serial.println(" ppm");

  delay(1000);
}
