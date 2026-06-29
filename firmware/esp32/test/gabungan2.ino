/*
 * Water Quality Monitor ESP32 
 * Flow Sensor - Pembacaan per 250 mL + Auto Kalibrasi
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ==================== PIN ====================
#define PH_PIN          32
#define TDS_PIN         33
#define ONE_WIRE_BUS    34
#define TURBIDITY_PIN   35
#define FLOW_PIN        18
#define BUZZER_PIN       2
#define LCD_ADDR        0x27

// ==================== FLOW KALIBRASI ====================
float PULSES_PER_LITER   = 310.0;   // ← 620 pulsa / 2 L
float CALIBRATION_FACTOR = 1.082;   // ← 2.0 / 1.848 (koreksi sisa error)
const float STEP_ML      = 250.0;   // Snap per 250 mL
const float TOLERANCE_L  = 0.05;    // Toleransi ±50 mL untuk snap

// ==================== MODE KALIBRASI ====================
bool  calibMode       = false;
float calibTargetL    = 0.0;
unsigned long calibStartPulse = 0;

// ==================== GLOBALS ====================
LiquidCrystal_I2C lcd(LCD_ADDR, 20, 4);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float temperatureC     = 25.0;
float phValue          = 7.0;
float tdsValue         = 0.0;
float turbidityPercent = 0.0;

volatile unsigned long pulseCount = 0;
float totalVolumeL     = 0.0;
float displayVolumeL   = 0.0;

// Buffers
const int AVG_SAMPLES = 12;
float phBuffer[AVG_SAMPLES];   int phIdx   = 0;
float tdsBuffer[AVG_SAMPLES];  int tdsIdx  = 0;
int   turbBuffer[AVG_SAMPLES]; int turbIdx = 0;

// Timing
unsigned long lastSensor = 0;
unsigned long lastLCD    = 0;
unsigned long lastSerial = 0;

// ==================== ISR ====================
void IRAM_ATTR flowISR() {
  pulseCount++;
}

// ==================== BEEP ====================
void beep(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH); delay(80);
    digitalWrite(BUZZER_PIN, LOW);  delay(120);
  }
}

// ==================== UPDATE FLOW ====================
void updateFlow() {
  noInterrupts();
  unsigned long currentPulse = pulseCount;
  interrupts();

  // Volume aktual
  totalVolumeL = (float)currentPulse / PULSES_PER_LITER * CALIBRATION_FACTOR;

  // Snap ke kelipatan 250 mL
  float stepL = STEP_ML / 1000.0;  // 0.250 L
  float snapped = floor(totalVolumeL / stepL) * stepL;

  // Snap ke atas jika dalam toleransi kelipatan berikutnya
  float nextStep = snapped + stepL;
  if (abs(totalVolumeL - nextStep) <= TOLERANCE_L) {
    displayVolumeL = nextStep;
  } else {
    displayVolumeL = snapped;
  }
}

// ==================== HELPERS ====================
float getFloatAvg(float* buf, int size) {
  float sum = 0.0;
  for (int i = 0; i < size; i++) sum += buf[i];
  return sum / size;
}

int getIntAvg(int* buf, int size) {
  long sum = 0;
  for (int i = 0; i < size; i++) sum += buf[i];
  return (int)(sum / size);
}

void printVolumeLCD(float vol) {
  int mL = (int)round(vol * 1000.0);
  lcd.setCursor(0, 0);

  char buf[21];
  // Format: "Vol:1.250L (1250mL) "
  sprintf(buf, "Vol:%.3fL (%4dmL)", vol, mL);
  lcd.print(buf);
}

// ==================== KALIBRASI ====================
void startCalibration(float targetLiter) {
  noInterrupts();
  calibStartPulse = pulseCount;
  interrupts();
  calibTargetL = targetLiter;
  calibMode    = true;

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("=== KALIBRASI ===   ");
  lcd.setCursor(0, 1); lcd.print("Target: ");
  lcd.print(targetLiter, 1);
  lcd.print(" L        ");
  lcd.setCursor(0, 2); lcd.print("Alirkan air sekarang");
  lcd.setCursor(0, 3); lcd.print("Kirim 'k' jika selesai");

  Serial.printf("\n[KALIBRASI] Target: %.1f L\n", targetLiter);
  Serial.println("Alirkan air tepat sebanyak target, lalu kirim 'k'");
}

void finishCalibration() {
  noInterrupts();
  unsigned long endPulse = pulseCount;
  interrupts();

  unsigned long deltaPulse = endPulse - calibStartPulse;

  if (deltaPulse < 10) {
    Serial.println("[KALIBRASI] Gagal: pulsa terlalu sedikit!");
    calibMode = false;
    return;
  }

  float newPPL = (float)deltaPulse / calibTargetL;
  PULSES_PER_LITER = newPPL;

  Serial.println("\n[KALIBRASI SELESAI]");
  Serial.printf("Delta Pulsa    : %lu\n", deltaPulse);
  Serial.printf("Target Volume  : %.3f L\n", calibTargetL);
  Serial.printf("PULSES_PER_LITER baru: %.2f\n", newPPL);
  Serial.println("Simpan nilai ini ke konstanta jika sudah OK!");

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("KALIBRASI SELESAI!");
  lcd.setCursor(0, 1); lcd.print("P/L baru:");
  lcd.print(newPPL, 2);
  lcd.setCursor(0, 2); lcd.print("Pulsa: ");
  lcd.print(deltaPulse);
  lcd.setCursor(0, 3); lcd.print("Kirim 'r' reset vol ");

  beep(3);
  calibMode = false;
  delay(4000);
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), flowISR, FALLING);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  sensors.begin();

  for (int i = 0; i < AVG_SAMPLES; i++) {
    phBuffer[i]   = 1.1;
    tdsBuffer[i]  = 0.8;
    turbBuffer[i] = 3800;
  }

  lcd.setCursor(0, 0); lcd.print("Flow Monitor v2     ");
  lcd.setCursor(0, 1); lcd.print("P/L :"); lcd.print(PULSES_PER_LITER, 1);
  lcd.setCursor(0, 2); lcd.print("Step: 250 mL        ");
  lcd.setCursor(0, 3); lcd.print("c1=kalib 1L, c2=2L  ");

  beep(2);

  Serial.println("=== WATER QUALITY MONITOR v2 ===");
  Serial.printf("PULSES_PER_LITER  : %.2f\n", PULSES_PER_LITER);
  Serial.println("--- PERINTAH SERIAL ---");
  Serial.println("'r'      = Reset volume");
  Serial.println("'c1'     = Mulai kalibrasi 1 Liter");
  Serial.println("'c2'     = Mulai kalibrasi 2 Liter");
  Serial.println("'k'      = Selesai kalibrasi (saat air cukup)");
  Serial.println("-----------------------");

  delay(2000);
  lcd.clear();
}

// ==================== LOOP ====================
void loop() {
  unsigned long now = millis();

  updateFlow();

  // Sensor lain
  if (!calibMode && now - lastSensor >= 1000) {
    lastSensor = now;

    sensors.requestTemperatures();
    float t = sensors.getTempCByIndex(0);
    if (t > -50 && t < 100) temperatureC = t;

    float vph = (analogRead(PH_PIN) * 3.3) / 4095.0;
    phBuffer[phIdx] = vph;
    phIdx = (phIdx + 1) % AVG_SAMPLES;
    phValue = getFloatAvg(phBuffer, AVG_SAMPLES);

    turbBuffer[turbIdx] = analogRead(TURBIDITY_PIN);
    turbIdx = (turbIdx + 1) % AVG_SAMPLES;
    int avgT = getIntAvg(turbBuffer, AVG_SAMPLES);
    turbidityPercent = constrain((float)avgT / 40.95, 0, 100);
  }

  // LCD update
  if (!calibMode && now - lastLCD >= 1200) {
    lastLCD = now;
    lcd.clear();

    // Baris 0: Volume display
    printVolumeLCD(displayVolumeL);

    // Baris 1: Pulsa & raw
    lcd.setCursor(0, 1);
    char buf1[21];
    sprintf(buf1, "Pls:%-5lu Raw:%.3fL", pulseCount, totalVolumeL);
    lcd.print(buf1);

    // Baris 2: pH
    lcd.setCursor(0, 2);
    char buf2[21];
    sprintf(buf2, "pH   : %.2f", phValue);
    lcd.print(buf2);

    // Baris 3: Suhu
    lcd.setCursor(0, 3);
    char buf3[21];
    sprintf(buf3, "Temp : %.1f C", temperatureC);
    lcd.print(buf3);
  }

  // Serial log
  if (!calibMode && now - lastSerial >= 3000) {
    lastSerial = now;
    int mL = (int)round(displayVolumeL * 1000.0);
    Serial.println("\n==========================================");
    Serial.printf("Volume      : %.3f L (%d mL)\n", displayVolumeL, mL);
    Serial.printf("Raw Volume  : %.3f L\n", totalVolumeL);
    Serial.printf("Total Pulses: %lu\n", pulseCount);
    Serial.printf("pH          : %.2f\n", phValue);
    Serial.printf("Temperature : %.2f C\n", temperatureC);
    Serial.println("==========================================");
  }

  // Serial command
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "r" || cmd == "R") {
      noInterrupts();
      pulseCount = 0;
      interrupts();
      totalVolumeL   = 0.0;
      displayVolumeL = 0.0;
      Serial.println(">>> VOLUME RESET <<<");

    } else if (cmd == "c1") {
      startCalibration(1.0);

    } else if (cmd == "c2") {
      startCalibration(2.0);

    } else if ((cmd == "k" || cmd == "K") && calibMode) {
      finishCalibration();
    }
  }

  delayMicroseconds(150);
}
