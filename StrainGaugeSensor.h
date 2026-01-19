#pragma once

#include "SystemStatus.h"
#include <Arduino.h>

class StrainGaugeSensor {
private:
    // Pin definitions
    static const int SENSOR_PIN = 35;
    static const int BUZZER_PIN = 27;
    static const int LED_PIN = 16;
    
    // Constants
    const float Vref = 3.3;
    const int ADC_MAX = 4095;
    const float STRAIN_MAX = 0.0008;
    const float gain = 1215.34;
    const float gf = 2.14;
    const float panjangPlat = 0.5;
    const float modulusE = 200e9;
    
    // Variables
    float adcAvg = 0;
    float adcNet = 0;
    float offsetAdc = 0;
    float noiseAdc = 0;
    float noiseThresholdAdc = 0;
    float loadPercent = 0;
    float Vout = 0;
    float Vr = 0;
    float strain = 0;
    float deltaL = 0;
    float stress = 0;
    
    // Hold variables
    float holdAdcAvg = 0, holdAdcNet = 0, holdVout = 0, holdVr = 0;
    float holdStrain = 0, holdDeltaL = 0, holdStress = 0, holdLoadPercent = 0;
    bool holdMode = false;
    
    // Moving average
    static const int N = 20;
    int adcBuffer[N];
    int idx = 0;
    long adcSum = 0;
    
    // Taring
    bool isTaring = false;
    
    // Alert tracking
    SystemStatus lastAlertStatus = STATUS_NORMAL;
    bool alertSent = false;
    
    // Helper methods
    void updateStrainHoldValues();
    float calculateLoadPercent(float strain);
    
public:
    void begin();
    void update();
    void tare();
    void toggleHold();
    
    // Getter methods
    float getLoadPercent() const;
    SystemStatus getStatus() const;
    float getStrain() const;
    float getVout() const;
    float getDeltaL() const;
    float getStress() const;
    float getVr() const;
    bool isHold() const;
    
    // Buzzer and LED
    void updateBuzzerAndLED();
    
    // Alert methods
    bool shouldSendAlert() const;
    const char* getAlertMessage() const;
    const char* getAlertType() const;
    
    // Serial output
    void printSerialOutput() const;
};