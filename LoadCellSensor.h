#pragma once

#include <HX711.h>

class LoadCellSensor {
private:
    HX711 scale;
    float calibration_factor = -430.0;
    float currentWeight = 0;
    float holdWeight = 0;
    bool holdMode = false;
    
    // Pin definitions
    static const int DOUT_PIN = 34;
    static const int CLK_PIN = 32;
    
public:
    void begin();
    void update();
    void tare();
    void toggleHold();
    float getWeight() const;
    bool isHold() const;
};