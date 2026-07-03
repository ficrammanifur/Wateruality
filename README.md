# 💧 Smart Water Quality Monitoring System for Reverse Osmosis Depot

Sistem monitoring kualitas air berbasis ESP32 untuk Depot Air Minum Reverse Osmosis (RO) dengan tampilan dashboard real-time.

## 📋 Deskripsi Project

Sistem ini dirancang untuk memonitor kualitas air RO secara real-time menggunakan beberapa sensor dan menampilkan data melalui dashboard web. Sistem menggunakan rule-based system untuk menentukan kelayakan air dan menghitung kesehatan filter secara otomatis.

## ✨ Fitur

- **Monitoring Real-time**: pH, TDS, Turbidity, Temperature, Flow Rate
- **Penilaian Air Otomatis**: LAYAK / TIDAK LAYAK (Rule-Based)
- **Kesehatan Filter**: Perhitungan otomatis berdasarkan volume, TDS, dan turbidity
- **Estimasi Ganti Filter**: Prediksi hari penggantian filter
- **Kontrol Otomatis**: Relay pompa ON/OFF berdasarkan kualitas air
- **Dashboard Web**: Tampilan modern dengan grafik real-time (Chart.js)
- **MQTT Communication**: Publish data ke broker MQTT
- **WiFiManager**: Mudah setting WiFi tanpa hardcode SSID/password

## 🏗️ Arsitektur Sistem

```
┌─────────────────────────────────────────────────────────────┐
│                         ESP32                               │
├─────────────────────────────────────────────────────────────┤
│  Sensors:                                                   │
│  ├── pH Sensor (GPIO32)                                     │
│  ├── TDS Sensor (GPIO33)                                    │
│  ├── Turbidity Sensor (GPIO35)                              │
│  ├── DS18B20 Temperature (GPIO25)                           │
│  └── Flow Sensor (GPIO18 - Interrupt)                       │
├─────────────────────────────────────────────────────────────┤
│  Outputs:                                                   │
│  ├── LCD I2C 20x4                                           │
│  ├── LED Hijau (GPIO26)                                     │
│  ├── LED Merah (GPIO27)                                     │
│  └── Relay Pump (GPIO4)                                     │
├─────────────────────────────────────────────────────────────┤
│  Communication:                                             │
│  └── MQTT → Dashboard Web                                   │
└─────────────────────────────────────────────────────────────┘
```

## 📁 Struktur Folder

```
Project/
├── Project.ino                 # Main program
├── sensors.h                   # Sensor management
├── water_rules.h              # Water quality rules
├── mqtt_handler.h             # MQTT communication
├── lcd_display.h              # LCD display
├── dashboard/                  # Web dashboard
│   ├── index.html
│   ├── style.css
│   └── script.js
└── README.md
```

## 🔌 Wiring ESP32

| Komponen | GPIO ESP32 |
|----------|------------|
| pH Sensor | GPIO32 |
| TDS Sensor | GPIO33 |
| Turbidity Sensor | GPIO35 |
| DS18B20 | GPIO25 |
| Flow Sensor | GPIO18 (Interrupt) |
| Relay Pump | GPIO4 |
| LED Hijau | GPIO26 |
| LED Merah | GPIO27 |
| LCD SDA | GPIO21 |
| LCD SCL | GPIO22 |

## 📊 Penjelasan Sensor

### pH Sensor
- Menggunakan 3-point calibration (pH 4, 7, 9)
- Moving average + EMA smoothing
- Range: 0-14 pH

### TDS Sensor
- Menggunakan formula DFRobot
- Kompensasi suhu otomatis
- Range: 0-1000 ppm

### Turbidity Sensor
- Mengukur kekeruhan air
- Output: Persentase kejernihan (0-100%)
- Kalibrasi di air jernih dan udara

### DS18B20 Temperature
- Sensor suhu digital
- Resolusi 12-bit
- Range: -55°C to 125°C

### Flow Sensor
- Menggunakan interrupt untuk akurasi
- Kalibrasi dengan volume aktual
- Menghitung flow rate (L/menit)

## 📦 Library yang Dibutuhkan

### Arduino Libraries (Install via Library Manager)
1. **LiquidCrystal_I2C** - untuk LCD I2C
2. **OneWire** - untuk DS18B20
3. **DallasTemperature** - untuk DS18B20
4. **WiFiManager** - untuk manajemen WiFi
5. **PubSubClient** - untuk MQTT

### Web Dashboard Libraries (CDN)
- Chart.js - Untuk grafik real-time
- Paho MQTT - Untuk koneksi MQTT via WebSocket

## 🚀 Cara Install

### 1. Clone Repository
```bash
git clone https://github.com/username/smart-ro-monitor.git
```

### 2. Install Arduino Libraries
Buka Arduino IDE → Sketch → Include Library → Manage Libraries
Cari dan install library yang dibutuhkan.

### 3. Upload ke ESP32
1. Buka file `Project.ino` dengan Arduino IDE
2. Pilih board: ESP32 Dev Module
3. Pilih port yang sesuai
4. Upload sketch

### 4. Setting WiFi
1. Setelah upload, ESP32 akan membuat hotspot "WaterMonitor"
2. Connect ke hotspot dan buka browser ke `192.168.4.1`
3. Pilih WiFi network dan masukkan password

### 5. Running Dashboard
1. Upload folder `dashboard` ke GitHub Pages
2. Atau jalankan local server:
```bash
cd dashboard
python -m http.server 8000
```
3. Buka browser ke `http://localhost:8000`

## 📡 Format MQTT

### Topic: `watermon/all`

```json
{
  "ph": 7.20,
  "tds": 18,
  "turbidity": 85,
  "temperature": 27.5,
  "status": "LAYAK",
  "health": 84,
  "days_left": 24,
  "volume": 18000.5,
  "flow_rate": 2.5,
  "pump": "ON"
}
```

### Individual Topics
- `watermon/ph` - pH value
- `watermon/tds` - TDS value (ppm)
- `watermon/temp` - Temperature (°C)
- `watermon/turbidity` - Turbidity (%)
- `watermon/volume` - Total volume (L)
- `watermon/flow_rate` - Flow rate (L/min)
- `watermon/status` - System status

## 🖥️ Dashboard Screenshot

*(Tambahkan screenshot dashboard di sini)*

## 🔧 Troubleshooting

### Sensor tidak terbaca
- Periksa koneksi wiring
- Pastikan pull-up resistor terpasang (kecuali sensor internal)
- Coba kalibrasi ulang

### WiFi tidak connect
- Reset WiFi settings: `wifi` command di Serial Monitor
- Pastikan mode WiFi 2.4GHz
- Coba gunakan hotspot smartphone untuk testing

### MQTT tidak terhubung
- Periksa koneksi internet
- Coba ganti broker MQTT
- Pastikan tidak ada firewall blocking port 1883

## 📝 Serial Commands

| Command | Deskripsi |
|---------|-----------|
| `r` | Reset volume flow sensor |
| `c1` | Mulai kalibrasi flow 1 liter |
| `c2` | Mulai kalibrasi flow 2 liter |
| `k` | Selesai kalibrasi flow |
| `status` | Tampilkan semua status sensor |
| `reset` | Reset filter health |
| `wifi` | Reset WiFi dan masuk portal |

## 🔮 Future Development

- [ ] Mobile App (Flutter/React Native)
- [ ] Historical data dengan InfluxDB
- [ ] Email/SMS notification when water not LAYAK
- [ ] Multiple filter stages monitoring
- [ ] Automatic filter replacement reminder
- [ ] Machine Learning for predictive maintenance
- [ ] Integration with Google Assistant/Alexa

## 📄 License

MIT License - feel free to use for education and commercial projects.

## 👨‍💻 Kontributor

- Nama Anda - [email@domain.com](mailto:email@domain.com)

---

**⭐ Jangan lupa star repository ini jika bermanfaat!**
