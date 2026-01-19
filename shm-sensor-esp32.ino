#include "LoadCellSensor.h"
#include "StrainGaugeSensor.h"
#include "DisplayManager.h"
#include "ButtonManager.h"
#include "FirebaseManager.h"
#include "config.h"
#include <WiFi.h>
#include "time.h"

// =============== Wi-Fi ==================
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// =============== TIME ===================
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7*3600; // GMT+7
const int   daylightOffset_sec = 0;

LoadCellSensor loadCell;
StrainGaugeSensor strainGauge;
DisplayManager display;
ButtonManager buttons;
FirebaseManager firebase;

SensorMode currentMode = MODE_LOAD_CELL;
unsigned long lastSend = 0;
const unsigned long sendInterval = 5000;

void setupWiFi() {
    WiFi.begin(ssid, password);
    
    char wifiText[20];
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        snprintf(wifiText, sizeof(wifiText), "%s", "Connecting Wi-Fi");
        display.printCentered(0, wifiText);
    }
    
    Serial.println("\nConnected to WiFi");
    snprintf(wifiText, sizeof(wifiText), "%s", " Wi-Fi Connected ");
    display.printCentered(0, wifiText);
}

void setupTime() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("Waiting for NTP time...");
    
    time_t now = time(nullptr);
    while (now < 100000) {
        delay(500);
        now = time(nullptr);
    }
    
    Serial.println("Time acquired: " + String(now));
    
    display.setCursor(0,2);
    display.print("Time Now: ");
    display.print(String(now));
    delay(1000);
}

void setup() {
    Serial.begin(115200);
    
    // Initialize display first
    display.begin();
    
    // Setup WiFi
    display.clear();
    setupWiFi();
    
    // Setup NTP Time
    setupTime();
    display.clear();
    
    // Initialize semua komponen
    buttons.begin();
    #if DEBUG_BUTTONS
    Serial.println("\n[BUTTON DEBUG] Tekan tombol, baca raw level (LOW = ditekan)");
    unsigned long _t0 = millis();
    while (millis() - _t0 < 5000) {
        int h = digitalRead(BTN_HOLD_PIN);
        int t = digitalRead(BTN_TARE_PIN);
        int m = digitalRead(BTN_MODE_PIN);
        Serial.print("H/T/M: ");
        Serial.print(h); Serial.print("/");
        Serial.print(t); Serial.print("/");
        Serial.println(m);
        delay(250);
    }
    #endif
    loadCell.begin();
    strainGauge.begin();
    firebase.begin();
    
    display.showMessage("TIMBANGAN DIGITAL", "Siap Digunakan");
    delay(2000);
    display.clear();
}

void loop() {
    // Update tombol
    buttons.update();
    
    // Cek jika tombol mode ditekan
    if (buttons.isModePressed()) {
        currentMode = (currentMode == MODE_LOAD_CELL) ? MODE_STRAIN_GAUGE : MODE_LOAD_CELL;
        display.showModeChange(currentMode);
        delay(1000);
        display.clear();
    }
    
    // Proses sensor berdasarkan mode
    if (currentMode == MODE_LOAD_CELL) {
        loadCell.update();
        
        // Cek tombol hold/tare untuk load cell
        if (buttons.isHoldPressed()) loadCell.toggleHold();
        if (buttons.isTarePressed()) {
            loadCell.tare();
            display.showMessage("LOAD CELL", "TARE DONE");
            delay(1000);
            display.clear();
        }
        
        // Update display
        display.showLoadCell(loadCell.getWeight(), loadCell.isHold());
        
        // Serial Output
        Serial.print("Berat: ");
        Serial.print(loadCell.getWeight(), 2);
        Serial.println(" gram");
        
        // Kirim ke Firebase
        if (millis() - lastSend > sendInterval) {
            lastSend = millis();
            firebase.sendLoadCellData(loadCell.getWeight());
        }
        
    } else { // MODE_STRAIN_GAUGE
        strainGauge.update();
        
        // Cek tombol hold/tare untuk strain gauge
        if (buttons.isHoldPressed()) strainGauge.toggleHold();
        if (buttons.isTarePressed()) {
            strainGauge.tare();
            display.showMessage("STRAIN GAUGE", "TARE DONE");
            delay(1000);
            display.clear();
        }
        
        // Update display
        display.showStrainGauge(
            strainGauge.getLoadPercent(),
            strainGauge.getStatus(),
            strainGauge.getStrain(),
            strainGauge.isHold()
        );
        
        // Update buzzer dan LED
        strainGauge.updateBuzzerAndLED();
        
        // Kirim alert jika perlu
        if (strainGauge.shouldSendAlert()) {
            firebase.sendAlert(strainGauge.getAlertMessage(), strainGauge.getAlertType());
        }
        
        // Kirim ke Firebase
        if (millis() - lastSend > sendInterval) {
            lastSend = millis();
            firebase.sendStrainGaugeData(
                strainGauge.getVout(),
                strainGauge.getDeltaL(),
                strainGauge.getLoadPercent(),
                strainGauge.getStrain(),
                strainGauge.getStress(),
                strainGauge.getVr()
            );
        }
    }
    
    delay(100);
}