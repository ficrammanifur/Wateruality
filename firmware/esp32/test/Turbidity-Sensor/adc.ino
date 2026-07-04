/*
 * =====================================================
 * AUTO CALIBRATION TURBIDITY SENSOR (ADC)
 * ESP32 - GPIO35
 * =====================================================
 */

#define TURBIDITY_PIN 35

int adcAir = 0;
int adcUdara = 0;

// ===============================
// Membaca ADC rata-rata
// ===============================
int bacaADC(uint8_t sample = 30)
{
  long total = 0;

  for (int i = 0; i < sample; i++)
  {
    total += analogRead(TURBIDITY_PIN);
    delay(10);
  }

  return total / sample;
}

// ===============================
// Menunggu Enter dari Serial
// ===============================
void tungguEnter()
{
  while (Serial.available())
    Serial.read();

  while (!Serial.available())
  {
    delay(10);
  }

  while (Serial.available())
    Serial.read();
}

void setup()
{
  Serial.begin(115200);

  analogReadResolution(12);
  analogSetPinAttenuation(TURBIDITY_PIN, ADC_11db);

  delay(1000);

  Serial.println();
  Serial.println("======================================");
  Serial.println(" TURBIDITY SENSOR AUTO CALIBRATION");
  Serial.println("======================================");

  //====================================
  // KALIBRASI AIR
  //====================================

  Serial.println();
  Serial.println("STEP 1");
  Serial.println("Celupkan sensor ke AIR JERNIH");
  Serial.println("Tekan ENTER jika sudah siap...");

  tungguEnter();

  adcAir = bacaADC();

  Serial.print("ADC AIR = ");
  Serial.println(adcAir);

  //====================================
  // KALIBRASI UDARA
  //====================================

  Serial.println();
  Serial.println("STEP 2");
  Serial.println("Angkat sensor ke UDARA");
  Serial.println("Tekan ENTER jika sudah siap...");

  tungguEnter();

  adcUdara = bacaADC();

  Serial.print("ADC UDARA = ");
  Serial.println(adcUdara);

  Serial.println();
  Serial.println("======================================");
  Serial.println("HASIL KALIBRASI");
  Serial.println("======================================");

  Serial.print("ADC AIR    : ");
  Serial.println(adcAir);

  Serial.print("ADC UDARA  : ");
  Serial.println(adcUdara);

  Serial.println();
  Serial.println("Copy nilai berikut ke program utama:");
  Serial.println();

  Serial.print("const int ADC_AIR    = ");
  Serial.print(adcAir);
  Serial.println(";");

  Serial.print("const int ADC_UDARA = ");
  Serial.print(adcUdara);
  Serial.println(";");

  Serial.println();
}

void loop()
{
  int adc = bacaADC();

  int kejernihan;

  if (adc >= adcAir)
    kejernihan = 100;
  else if (adc <= adcUdara)
    kejernihan = 0;
  else
    kejernihan = map(adc,
                     adcUdara,
                     adcAir,
                     0,
                     100);

  int kekeruhan = 100 - kejernihan;

  Serial.print("ADC : ");
  Serial.print(adc);

  Serial.print(" | Jernih : ");
  Serial.print(kejernihan);
  Serial.print("%");

  Serial.print(" | Keruh : ");
  Serial.print(kekeruhan);
  Serial.print("%");

  Serial.println();

  delay(500);
}
