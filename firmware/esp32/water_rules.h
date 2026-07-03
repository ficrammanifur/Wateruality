/*
 * ============================================================
 *   WATER QUALITY RULES
 *   Rule-Based System for water quality assessment
 * ============================================================
 */

#ifndef WATER_RULES_H
#define WATER_RULES_H

#include <Arduino.h>

// ==================== WATER STANDARDS ====================
struct WaterStandards {
  float phMin = 6.5;
  float phMax = 8.5;
  float tdsMax = 500.0;        // mg/L atau ppm
  float turbidityMin = 50.0;   // Persen kejernihan
  float tempMin = 15.0;
  float tempMax = 35.0;
};

// ==================== FUNCTION PROTOTYPES ====================
bool isWaterLayak(float ph, float tds, float turbidity, float temp);
String getWaterStatus(float ph, float tds, float turbidity, float temp);
bool isPHNormal(float ph);
bool isTDSNormal(float tds);
bool isTurbidityNormal(float turbidity);
bool isTemperatureNormal(float temp);

// ============================================================
//   IMPLEMENTATION (inline functions)
// ============================================================

/**
 * Cek apakah pH dalam batas normal
 * @param ph Nilai pH (0-14)
 * @return true jika normal (6.5 - 8.5)
 */
inline bool isPHNormal(float ph) {
  return (ph >= 6.5 && ph <= 8.5);
}

/**
 * Cek apakah TDS dalam batas normal
 * @param tds Nilai TDS dalam ppm
 * @return true jika normal (< 500 ppm)
 */
inline bool isTDSNormal(float tds) {
  return (tds <= 500.0);
}

/**
 * Cek apakah turbidity dalam batas normal
 * @param turbidity Persentase kejernihan (0-100%)
 * @return true jika normal (> 50%)
 */
inline bool isTurbidityNormal(float turbidity) {
  return (turbidity >= 50.0);
}

/**
 * Cek apakah suhu dalam batas normal
 * @param temp Suhu dalam Celcius
 * @return true jika normal (15-35°C)
 */
inline bool isTemperatureNormal(float temp) {
  return (temp >= 15.0 && temp <= 35.0);
}

/**
 * Menentukan apakah air LAYAK atau TIDAK LAYAK
 * Berdasarkan semua parameter kualitas air
 * @param ph Nilai pH
 * @param tds Nilai TDS (ppm)
 * @param turbidity Persentase kejernihan (%)
 * @param temp Suhu (°C)
 * @return true jika LAYAK, false jika TIDAK LAYAK
 */
inline bool isWaterLayak(float ph, float tds, float turbidity, float temp) {
  return (isPHNormal(ph) && 
          isTDSNormal(tds) && 
          isTurbidityNormal(turbidity) && 
          isTemperatureNormal(temp));
}

/**
 * Mendapatkan status air dalam bentuk string
 * @param ph Nilai pH
 * @param tds Nilai TDS (ppm)
 * @param turbidity Persentase kejernihan (%)
 * @param temp Suhu (°C)
 * @return String "LAYAK" atau "TIDAK LAYAK"
 */
inline String getWaterStatus(float ph, float tds, float turbidity, float temp) {
  return isWaterLayak(ph, tds, turbidity, temp) ? "LAYAK" : "TIDAK LAYAK";
}

#endif // WATER_RULES_H
