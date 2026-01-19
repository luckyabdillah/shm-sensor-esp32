#pragma once

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "SensorMode.h"
#include "SystemStatus.h"

class DisplayManager {
private:
    LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4);
    bool lcdAvailable = false;
    unsigned long lastLcdUpdate = 0;
    const unsigned long lcdInterval = 500;
    
    bool isI2CDeviceConnected(uint8_t address);
    
public:
    void begin();
    bool isAvailable() const;
    
    void showLoadCell(float weight, bool holdMode);
    void showStrainGauge(float loadPercent, SystemStatus status, float strain, bool holdMode);
    void showModeChange(SensorMode mode);
    void showMessage(const char* line1, const char* line2 = "");
    void printCentered(uint8_t row, const char* text);
    void clear();
    
    // Untuk tampilan sementara
    void setCursor(uint8_t col, uint8_t row);
    void print(const String& text);
    void print(const char* text);
};