/*
 * ============================================================
 * WATER QUALITY RULES - PROFESSIONAL VERSION
 * ============================================================
 */
#ifndef WATER_RULES_H
#define WATER_RULES_H
#include <Arduino.h>

struct WaterStandards {
    float phMin = 6.5;
    float phMax = 9.8;
    float tdsMax = 500.0;
    float tdsIdealMax = 50.0;
    float ntuMax = 5.0;
    float ntuIdealMax = 1.0;
    float tempMin = 15.0;
    float tempMax = 35.0;
};

enum WaterQualityLevel {
    QUALITY_LAYAK,
    QUALITY_CUKUP,
    QUALITY_TIDAK_LAYAK
};

bool isWaterLayak(float ph, float tds, float ntu, float temp);
String getWaterStatus(float ph, float tds, float ntu, float temp);
WaterQualityLevel getWaterQualityLevel(float ph, float tds, float ntu, float temp);
String getQualityDescription(WaterQualityLevel level);
bool isPHNormal(float ph);
bool isTDSNormal(float tds);
bool isTurbidityNormal(float ntu);
bool isTemperatureNormal(float temp);
String getDetailedStatus(float ph, float tds, float ntu, float temp);
String getUnlayakReason(float ph, float tds, float ntu, float temp);
String getTurbidityDescription(float ntu);

// ============================================================
// IMPLEMENTATION
// ============================================================
inline bool isPHNormal(float ph) {
    return (ph >= 6.5 && ph <= 9.8);
}

inline bool isPHExcellent(float ph) {
    return (ph >= 7.0 && ph <= 8.5);
}

inline bool isTDSNormal(float tds) {
    return (tds <= 500.0);
}

inline bool isTDSExcellent(float tds) {
    return (tds <= 50.0);
}

inline bool isTurbidityNormal(float ntu) {
    return (ntu <= 5.0);
}

inline bool isTurbidityExcellent(float ntu) {
    return (ntu <= 1.0);
}

inline bool isTemperatureNormal(float temp) {
    return (temp >= 15.0 && temp <= 35.0);
}

inline String getTurbidityDescription(float ntu) {
    if (ntu <= 10) return "SANGAT JERNIH";
    if (ntu <= 25) return "JERNIH";
    if (ntu <= 50) return "CUKUP JERNIH";
    if (ntu <= 75) return "AGAK KERUH";
    return "KERUH";
}

bool isWaterLayak(float ph, float tds, float ntu, float temp) {
    return (ph >= 6.5 && ph <= 9.8) &&
           (tds <= 500.0) &&
           (ntu <= 5.0) &&
           (temp >= 15 && temp <= 35);
}

inline WaterQualityLevel getWaterQualityLevel(float ph, float tds, float ntu, float temp) {
    bool phOK = isPHNormal(ph);
    bool tdsOK = isTDSNormal(tds);
    bool turbOK = isTurbidityNormal(ntu);
    bool tempOK = isTemperatureNormal(temp);
    
    if (phOK && tdsOK && turbOK && tempOK) {
        return QUALITY_LAYAK;
    }
    
    int failCount = 0;
    if (!phOK) failCount++;
    if (!tdsOK) failCount++;
    if (!turbOK) failCount++;
    if (!tempOK) failCount++;
    
    if (failCount <= 1) {
        return QUALITY_CUKUP;
    }
    return QUALITY_TIDAK_LAYAK;
}

inline String getQualityDescription(WaterQualityLevel level) {
    switch(level) {
        case QUALITY_LAYAK:
            return "LAYAK - Air sangat baik ✓";
        case QUALITY_CUKUP:
            return "CUKUP - Perlu perhatian ⚠️";
        case QUALITY_TIDAK_LAYAK:
            return "TIDAK LAYAK - Tidak aman ✗";
        default:
            return "UNKNOWN";
    }
}

inline String getWaterStatus(float ph, float tds, float ntu, float temp) {
    return isWaterLayak(ph, tds, ntu, temp) ? "LAYAK" : "TIDAK LAYAK";
}

inline String getDetailedStatus(float ph, float tds, float ntu, float temp) {
    String status = getWaterStatus(ph, tds, ntu, temp);
    if (status == "LAYAK") {
        return status + " ✓";
    } else {
        return status + " ✗ (" + getUnlayakReason(ph, tds, ntu, temp) + ")";
    }
}

inline String getUnlayakReason(float ph, float tds, float ntu, float temp) {
    String reasons = "";
    bool first = true;
    String separator;
    
    if (!isPHNormal(ph)) {
        separator = first ? "" : ", ";
        reasons += separator + "pH (" + String(ph, 2) + ")";
        first = false;
    }
    
    if (!isTDSNormal(tds)) {
        separator = first ? "" : ", ";
        reasons += separator + "TDS (" + String(tds, 0) + " ppm)";
        first = false;
    }
    
    if (!isTurbidityNormal(ntu)) {
        separator = first ? "" : ", ";
        reasons += separator + "Kekeruhan (" + String(ntu, 2) + " NTU)";
        first = false;
    }
    
    if (!isTemperatureNormal(temp)) {
        separator = first ? "" : ", ";
        reasons += separator + "Suhu (" + String(temp, 1) + "°C)";
        first = false;
    }
    
    if (reasons.isEmpty()) {
        reasons = "Semua parameter normal";
    }
    return reasons;
}

#endif // WATER_RULES_H
