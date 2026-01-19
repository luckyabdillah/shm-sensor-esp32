# SHM Sensor ESP32

Sistem monitoring beban dan regangan (strain gauge) berbasis ESP32 dengan dual-sensor capability, Firebase cloud integration, dan display real-time.

## 📋 Daftar Isi

- [Fitur](#fitur)
- [Hardware](#hardware)
- [Software Architecture](#software-architecture)
- [Instalasi & Setup](#instalasi--setup)
- [Konfigurasi](#konfigurasi)
- [Penggunaan](#penggunaan)
- [Troubleshooting](#troubleshooting)

## ✨ Fitur

### Dual Sensor Mode
- **Load Cell Sensor**: Mengukur berat beban dengan HX711 ADC
- **Strain Gauge Sensor**: Mengukur regangan dan status beban struktural

### Kontrol & Interface
- **3 Push Button**: Mode switch, Hold/Live toggle, Tare calibration
- **20x4 LCD Display**: Real-time monitoring dengan I2C
- **Buzzer & LED**: Alert visual/audio untuk status WARNING dan DANGER

### Cloud Integration
- **Firebase Realtime Database**: Kirim data sensor dan alert ke cloud
- **Email/Password Auth**: Autentikasi aman dengan Firebase
- **Timestamp**: Setiap data tercatat dengan waktu NTP

### Advanced Features
- **Hold Mode**: Freeze pengukuran untuk analisis value tertentu
- **Tare/Calibration**: Reset baseline pengukuran per sensor
- **Debouncing**: Button dengan state-change detection
- **Moving Average**: Filter noise dengan buffer 20 sample
- **Status Tracking**: 4-level alert system (NORMAL → NOTICE → WARNING → DANGER)

---

## 🛠️ Hardware

### Komponen Utama

| Komponen | Model/Spesifikasi | Pin GPIO | Catatan |
|----------|-------------------|----------|---------|
| Microcontroller | ESP32 | - | Board: ESP32 DevKit v1 |
| Load Cell ADC | HX711 | 34 (DOUT), 32 (CLK) | Input-only pin issue |
| Strain Gauge Sensor | Analog In | 35 | ADC pin |
| Display | 20x4 LCD I2C | 21 (SDA), 22 (SCL) | Address 0x27 |
| Push Button HOLD | Tactile Switch | 13 (BTN_HOLD_PIN) | Latching button |
| Push Button TARE | Tactile Switch | 14 (BTN_TARE_PIN) | Momentary button |
| Push Button MODE | Tactile Switch | 15 (BTN_MODE_PIN) | Momentary button |
| Buzzer | 5V Buzzer | 27 | Active HIGH |
| LED Alert | Generic LED + 220Ω | 16 | Active HIGH |

### Wiring Diagram (Button Section)
```
Button → GPIO (pulled up internally)
      └→ GND

Contoh:
HOLD Button:
  One side → GPIO 13
  Other side → GND
  (Internal pull-up menangani HIGH state)
```

### Pin Safety
- **GPIO 13, 14, 15**: Safe untuk input dengan pull-up
- **GPIO 34, 35**: Input-only pins (no internal pull-up support)
- Jika HX711 lib enable pull-up pada GPIO 34, abaikan warning "input-only pad has no internal PU"

---

## 🏗️ Software Architecture

### Modular Structure

```
shm-sensor-esp32.ino          Main sketch / orchestrator
├── ButtonManager (H/CPP)      Button input handling
├── DisplayManager (H/CPP)      LCD 20x4 I2C display
├── LoadCellSensor (H/CPP)      HX711 weight measurement
├── StrainGaugeSensor (H/CPP)   Analog strain + stress calculation
├── FirebaseManager (H/CPP)     Cloud data sync
├── config.h                    WiFi, Firebase, pin configuration
├── SensorMode.h                Enum untuk sensor selection
└── SystemStatus.h              Enum untuk alert levels
```

### Class Overview

#### 1. **ButtonManager**
```cpp
- begin()                    // Setup GPIO pins
- update()                   // Poll button states (call setiap loop)
- isHoldPressed()           // Return true if HOLD state changed (consume flag)
- isTarePressed()           // Return true if TARE edge detected
- isModePressed()           // Return true if MODE edge detected
```
**Logic**:
- HOLD: State-change detection (latch button behavior)
- TARE/MODE: Edge-triggered (momentary buttons)
- 50ms debounce untuk semua

#### 2. **LoadCellSensor**
```cpp
- begin()              // Inisialisasi HX711
- update()             // Baca ADC dan hitung berat
- tare()              // Kalibrasi ke zero point
- toggleHold()        // Freeze/unfreeze pengukuran
- getWeight()         // Return berat (gram)
- isHold()            // Return true jika dalam hold mode
```
**Kalibrasi**: Sesuaikan `calibration_factor` di LoadCellSensor.h

#### 3. **StrainGaugeSensor**
```cpp
- begin()                          // Setup ADC dan buzzer/LED
- update()                         // Baca analog, hitung strain/stress
- tare()                          // Kalibrasi offset ADC (dijalankan saat setup)
- toggleHold()                    // Freeze/unfreeze
- getLoadPercent()                // Return beban 0-100%
- getStatus()                     // Return STATUS_NORMAL/NOTICE/WARNING/DANGER
- getStrain()                     // Return strain value
- getVout(), getDeltaL(), getStress(), getVr()  // Return calculated values
- isHold()                        // Return hold mode state
- updateBuzzerAndLED()            // Set output berdasarkan status
- shouldSendAlert(), getAlertMessage(), getAlertType()  // Alert management
```
**Status Thresholds**:
- NORMAL: < 50%
- NOTICE: 50-70%
- WARNING: 70-90%
- DANGER: ≥ 90%

#### 4. **DisplayManager**
```cpp
- begin()                         // Inisialisasi LCD I2C
- isAvailable()                   // Check LCD connected
- showLoadCell(weight, hold)      // Display berat + mode
- showStrainGauge(load, status, strain, hold)  // Display strain data
- showModeChange(mode)            // Display mode switch banner
- showMessage(line1, line2)       // Custom message 2 line
- clear()                         // Clear LCD
```
**Update Rate**: 500ms untuk mencegah flicker

#### 5. **FirebaseManager**
```cpp
- begin()                                 // Connect ke Firebase + signup
- isReady()                               // Check connection status
- sendLoadCellData(load)                  // Send weight ke /loadCells
- sendStrainGaugeData(...)                // Send 6 param ke /strainGauges
- sendAlert(message, type)                // Send alert ke /alerts
```
**Auth Flow**:
1. Sign-up dengan email/password (first-time only)
2. Subsequent logins use stored credentials
3. Jika EMAIL_EXISTS error, continue (user sudah ada)

---

## 🔧 Instalasi & Setup

### Prerequisites
- **Arduino IDE** (v1.8.19+) atau **PlatformIO**
- **ESP32 Board Package**
- **Libraries**:
  - HX711 (untuk load cell)
  - LiquidCrystal_I2C (untuk LCD)
  - Firebase ESP Client
  - ArduinoJson

### Step 1: Install Board Package
1. Buka Arduino IDE → Preferences
2. Paste di "Additional Boards Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Tools → Board Manager → Search "esp32" → Install "esp32 by Espressif Systems"

### Step 2: Install Libraries
Tools → Manage Libraries, install:
- `HX711`
- `LiquidCrystal_I2C`
- `Firebase ESP Client` (by Mobizt)

### Step 3: Clone & Configure
```bash
git clone <repo-url>
cd shm-sensor-esp32
```

Edit `config.h`:
```cpp
#define WIFI_SSID "YourSSID"
#define WIFI_PASS "YourPassword"

#define FIREBASE_HOST "https://YOUR-PROJECT.firebasedatabase.app/"
#define API_KEY "YOUR_API_KEY"
#define DB_EMAIL "device@yourapp.local"
#define DB_PASS "secure_password"

// Sesuaikan pin jika wiring berbeda
#define BTN_HOLD_PIN 13
#define BTN_TARE_PIN 14
#define BTN_MODE_PIN 15
```

### Step 4: Setup Firebase
1. Buat project di [Firebase Console](https://console.firebase.google.com)
2. Enable Realtime Database (Start in Test Mode untuk development)
3. Enable Email/Password Authentication
4. Create user dengan email & password sesuai `config.h`
5. Setup Database Rules (contoh):
   ```json
   {
     "rules": {
       ".read": "auth.uid != null",
       ".write": "auth.uid != null"
     }
   }
   ```

### Step 5: Upload
1. Connect ESP32 via USB
2. Tools → Select Board "ESP32 Dev Module"
3. Tools → Port → Select COM port
4. Sketch → Upload
5. Monitor → Serial (115200 baud)

---

## 📝 Konfigurasi

### config.h

| Makro | Default | Deskripsi |
|-------|---------|-----------|
| `WIFI_SSID` | "Cancer" | WiFi network name |
| `WIFI_PASS` | "12345678" | WiFi password |
| `FIREBASE_HOST` | "https://...asia-southeast1.firebasedatabase.app" | Database URL |
| `API_KEY` | "AIzaSy..." | Firebase API key |
| `DB_EMAIL` | "yours@email.com" | Firebase auth email |
| `DB_PASS` | "yourpassword" | Firebase auth password |
| `BTN_HOLD_PIN` | 13 | GPIO untuk tombol HOLD |
| `BTN_TARE_PIN` | 14 | GPIO untuk tombol TARE |
| `BTN_MODE_PIN` | 15 | GPIO untuk tombol MODE |
| `DEBUG_BUTTONS` | 1 | Enable debug di startup (tekan tombol selama 5s) |

### LoadCellSensor.h Calibration
```cpp
float calibration_factor = -430.0;  // Sesuaikan dengan cell Anda
```
**Cara kalibrasi**:
1. Timbang beban referensi (misal 1kg = 1000g)
2. Adjust factor sampai output sesuai

### StrainGaugeSensor.h Constants
```cpp
const float Vref = 3.3;          // ADC reference voltage
const float STRAIN_MAX = 0.0008; // Maximum strain threshold
const float gain = 1215.34;      // Sensor circuit gain
const float gf = 2.14;           // Gauge factor
const float panjangPlat = 0.5;   // Plate length (m)
const float modulusE = 200e9;    // Young's modulus (Pa)
```

---

## 🎮 Penggunaan

### Startup Sequence
1. **Serial Monitor (5s)**: Debug buttons - tekan setiap tombol untuk verify
2. **WiFi Connect**: Board connect ke WiFi network
3. **NTP Time**: Sinkronisasi waktu dari internet
4. **Firebase Connect**: Authenticate dan upload capability test
5. **Ready**: "TIMBANGAN DIGITAL" pada LCD + Siap Digunakan

### Button Controls

#### HOLD Button (Latch/Toggle)
- **Press**: Toggle antara HOLD (freeze value) dan LIVE (real-time)
- **Display**: Berubah ke "LOAD CELL HOLD" atau "STRAIN GAUGE HOLD"
- **Behavior**: State-change detection (physical latch button)

#### TARE Button (Momentary)
- **Press**: Kalibrasi sensor ke zero point
- **Display**: Popup "TARE DONE" (1 detik)
- **Behavior**: Reset ADC offset dan reset measured values

#### MODE Button (Momentary)
- **Press**: Switch antara Load Cell ↔ Strain Gauge
- **Display**: Mode change banner (1 detik)
- **Behavior**: Toggle `currentMode` enum

### Display Modes

#### Load Cell Mode
```
LINE 0: LOAD CELL LIVE  (atau LOAD CELL HOLD)
LINE 1: Berat:
LINE 2: XXX.XX g
LINE 3: (empty)
```

#### Strain Gauge Mode
```
LINE 0: STRAIN LIVE     (atau STRAIN HOLD)
LINE 1: LOAD   : XX.X %
LINE 2: STATUS : NORMAL (atau NOTICE, WARNING, DANGER)
LINE 3: STRAIN : 0.000123
```

### Firebase Data Structure

**Load Cell Data**:
```
/loadCells/
  ├── 1768815322/
  │   └── load: 1234.56
  ├── 1768815327/
  │   └── load: 1234.58
  └── ...
```

**Strain Gauge Data**:
```
/strainGauges/
  ├── 1768815322/
  │   ├── avgVoltage: 0.0125
  │   ├── deltaL: 0.0000625
  │   ├── load: 50.5
  │   ├── strain: 0.000125
  │   ├── stress: 25000000
  │   └── vr: 0.00001
  └── ...
```

**Alerts**:
```
/alerts/
  ├── 1768815350/
  │   ├── message: "High strain detected"
  │   └── type: "warning"
  └── ...
```

### Serial Output Examples

**Boot**:
```
Time acquired: 1768815322
Buttons on pins H/T/M: 13/14/15

[BUTTON DEBUG] Tekan tombol, baca raw level (LOW = ditekan)
H/T/M: 1/1/1
H/T/M: 1/1/0  ← MODE button pressed
...

Connected to WiFi
Firebase connected
TIMBANGAN DIGITAL
Siap Digunakan
```

**During Operation** (Load Cell Mode):
```
Button HOLD state: LATCHED (in)
Berat: 1234.56 gram
Firebase OK - Load Cell
```

**During Operation** (Strain Gauge Mode):
```
Button MODE pressed
Button HOLD state: LATCHED (in)
Firebase OK - Strain Gauge
Button TARE pressed
=== TARE STRAIN GAUGE START ===
Pastikan beban = 0 dan plat diam...
=== TARE STRAIN GAUGE DONE ===
Offset ADC       : 2048.123
Noise ADC (σ)    : 12.456
Threshold ADC    : 37.368
```

## 🐛 Troubleshooting

### Button Tidak Bekerja

**Simptom**: Tombol tidak trigger action

**Solusi**:
1. **Cek wiring**: Pastikan tombol tersambung ke GPIO yang benar
   - One side ke GPIO
   - Other side ke GND
   - No external pull-up resistor (internal pull-up digunakan)

2. **Verifikasi pin mapping**:
   - Edit `config.h` sesuai wiring Anda
   - Upload dan monitor serial 5s startup debug
   - Setiap tombol harus menunjukkan LOW (0) saat ditekan

3. **Ganti pin jika perlu**:
   - GPIO 13, 14, 15 → safe untuk input dengan pull-up
   - Jika pin tidak bekerja, try: 33, 25, 26
   - Hindari: 6-11 (reserved), 34-39 (input-only), 36, 39 (noisy)

4. **Increase debounce** (jika flaky):
   - Edit ButtonManager.h: `const unsigned long debounceDelay = 100;` (from 50)

### Firebase Data Tidak Terkirim

**Simptom**: Serial shows "Firebase error" atau data tidak muncul di Firebase Console

**Solusi**:
1. **Cek WiFi connection**: Serial harus show "Connected to WiFi"
2. **Verifikasi Firebase credentials** di config.h:
   - API_KEY sesuai
   - Email/password benar
   - Database URL benar
3. **Check Database Rules**:
   - Harus allow read/write untuk authenticated user
   - Test Mode memungkinkan semua akses
4. **Restart board** dan cek serial output untuk error messages

### LCD Tidak Menampilkan

**Simptom**: Serial shows "LCD NOT DETECTED (I2C 0x27)"

**Solusi**:
1. **Cek address LCD**: Default 0x27, bisa juga 0x3F
   - Edit DisplayManager.cpp: `LiquidCrystal_I2C lcd(0x3F, 20, 4);`
2. **Verify wiring**:
   - SDA (ESP32 pin 21) ke LCD SDA
   - SCL (ESP32 pin 22) ke LCD SCL
   - GND dan 5V power
3. **Test I2C**: Scan untuk detect LCD pada I2C bus

### HX711 ADC Warning

**Simptom**: Serial shows `E (17543) gpio: gpio_pullup_en(78): GPIO number error`

**Solusi**: Abaikan—ini harmless warning karena GPIO 34 adalah input-only. HX711 module menyediakan signal yang dibutuhkan tanpa internal pull-up.

### Strain Gauge Offset Tidak Stabil

**Simptom**: Zero point berubah setelah tare

**Solusi**:
1. **Ensure plate is zero-load**: Pastikan beban = 0 saat tare
2. **Wait for stabilization**: Tare membutuhkan 2 detik untuk 400 samples
3. **Increase noise threshold**: Edit StrainGaugeSensor.h
   - Naikkan `noiseThresholdAdc = 3 * noiseAdc;` menjadi `5 * noiseAdc`
   - Re-tare sensor

## 📊 Performance Specs

| Parameter | Value |
|-----------|-------|
| Sensor Update Rate | 100ms |
| Firebase Send Interval | 5s |
| LCD Refresh Rate | 500ms |
| Button Debounce | 50ms |
| Moving Average Buffer | 20 samples |
| NTP Sync Timeout | Auto-retry |
| WiFi Reconnect | Automatic |


## 📄 License

MIT License - Feel free to modify and distribute

## 📞 Support

Jika ada pertanyaan atau bug report, buat issue di repository ini.

**Happy monitoring! 🚀**
