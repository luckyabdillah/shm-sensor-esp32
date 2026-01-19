#include <Arduino.h>
#include "DisplayManager.h"

bool DisplayManager::isI2CDeviceConnected(uint8_t address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}

void DisplayManager::begin() {
    Wire.begin(21, 22); // SDA, SCL ESP32
    
    if (isI2CDeviceConnected(0x27)) {
        lcdAvailable = true;
        lcd.init();
        lcd.backlight();
        lcd.clear();
        Serial.println("LCD DETECTED (I2C 0x27)");
    } else {
        lcdAvailable = false;
        Serial.println("LCD NOT DETECTED (I2C 0x27)");
    }
}

bool DisplayManager::isAvailable() const {
    return lcdAvailable;
}

void DisplayManager::printCentered(uint8_t row, const char* text) {
    if (!lcdAvailable) return;
    
    int len = strlen(text);
    if (len > 20) len = 20;

    int col = (20 - len) / 2;
    if (col < 0) col = 0;

    lcd.setCursor(col, row);
    lcd.print(text);
}

void DisplayManager::clear() {
    if (!lcdAvailable) return;
    lcd.clear();
}

void DisplayManager::setCursor(uint8_t col, uint8_t row) {
    if (!lcdAvailable) return;
    lcd.setCursor(col, row);
}

void DisplayManager::print(const String& text) {
    if (!lcdAvailable) return;
    lcd.print(text);
}

void DisplayManager::print(const char* text) {
    if (!lcdAvailable) return;
    lcd.print(text);
}

void DisplayManager::showLoadCell(float weight, bool holdMode) {
    if (!lcdAvailable) return;
    
    if (millis() - lastLcdUpdate < lcdInterval) return;
    lastLcdUpdate = millis();
    
    char modeText[20];
    snprintf(modeText, sizeof(modeText), "%s", holdMode ? "LOAD CELL HOLD" : "LOAD CELL LIVE");
    printCentered(0, modeText);

    setCursor(0, 1);
    print("Berat:");
    
    setCursor(0, 2);
    print(String(weight, 2) + " g       ");

    setCursor(0, 3);
    print("                ");
}

void DisplayManager::showStrainGauge(float loadPercent, SystemStatus status, float strain, bool holdMode) {
    if (!lcdAvailable) return;
    
    if (millis() - lastLcdUpdate < lcdInterval) return;
    lastLcdUpdate = millis();
    
    char modeText[20];
    snprintf(modeText, sizeof(modeText), "%s", holdMode ? "STRAIN HOLD" : "STRAIN LIVE");
    printCentered(0, modeText);

    setCursor(0, 1);
    print("LOAD   : " + String(loadPercent, 1) + " %   ");

    const char* statusText;
    switch (status) {
        case STATUS_NORMAL:  statusText = "NORMAL"; break;
        case STATUS_NOTICE:  statusText = "NOTICE"; break;
        case STATUS_WARNING: statusText = "WARNING"; break;
        case STATUS_DANGER:  statusText = "DANGER"; break;
        default:             statusText = "UNKNOWN"; break;
    }

    setCursor(0, 2);
    print("STATUS : " + String(statusText) + "   ");

    setCursor(0, 3);
    print("STRAIN : " + String(strain, 6) + "   ");
}

void DisplayManager::showModeChange(SensorMode mode) {
    if (!lcdAvailable) return;
    
    clear();
    if (mode == MODE_LOAD_CELL) {
        printCentered(1, "MODE: LOAD CELL");
    } else {
        printCentered(1, "MODE: STRAIN GAUGE");
    }
}

void DisplayManager::showMessage(const char* line1, const char* line2) {
    if (!lcdAvailable) return;
    
    clear();
    printCentered(1, line1);
    if (strlen(line2) > 0) {
        printCentered(2, line2);
    }
}