#pragma once

#include <Arduino.h>
#include "config.h"

class ButtonManager {
private:
    // Pin definitions (override in config.h to match wiring)
    static const int HOLD_PIN = BTN_HOLD_PIN;
    static const int TARE_PIN = BTN_TARE_PIN;
    static const int MODE_SWITCH_PIN = BTN_MODE_PIN;
    
    // Button states (current debounced)
    bool holdButtonState = HIGH;
    bool tareButtonState = HIGH;
    bool modeButtonState = HIGH;
    
    // Last confirmed stable states
    bool lastHoldButtonState = HIGH;    // For state-change detection
    bool lastTareButtonState = HIGH;    // For edge detection
    bool lastModeButtonState = HIGH;    // For edge detection
    
    // Timers for debounce
    unsigned long lastHoldTime = 0;
    unsigned long lastTareTime = 0;
    unsigned long lastModeTime = 0;
    const unsigned long debounceDelay = 50;
    
    // Press detection flags
    bool holdPressed = false;
    bool tarePressed = false;
    bool modePressed = false;

    // Edge detection state for momentary buttons (TARE, MODE)
    bool tareActive = false;
    bool modeActive = false;
    
public:
    void begin();
    void update();
    
    // Returns true once per physical press and consumes the latched flag
    bool isHoldPressed();
    bool isTarePressed();
    bool isModePressed();
    
    void resetPresses();
};