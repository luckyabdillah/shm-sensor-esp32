#include "ButtonManager.h"

void ButtonManager::begin() {
    pinMode(HOLD_PIN, INPUT_PULLUP);
    pinMode(TARE_PIN, INPUT_PULLUP);
    pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
}

void ButtonManager::update() {
    unsigned long currentTime = millis();
    
    // Reset press flags
    holdPressed = false;
    tarePressed = false;
    modePressed = false;
    
    // Read button states
    bool currentHold = digitalRead(HOLD_PIN);
    bool currentTare = digitalRead(TARE_PIN);
    bool currentMode = digitalRead(MODE_SWITCH_PIN);
    
    // HOLD button
    if (currentHold != lastHoldButtonState) {
        lastHoldTime = currentTime;
    }
    
    if ((currentTime - lastHoldTime) > debounceDelay) {
        if (currentHold == LOW && lastHoldButtonState == HIGH) {
            holdPressed = true;
            Serial.println("Button HOLD pressed");
        }
    }
    lastHoldButtonState = currentHold;
    
    // TARE button
    if (currentTare != lastTareButtonState) {
        lastTareTime = currentTime;
    }
    
    if ((currentTime - lastTareTime) > debounceDelay) {
        if (currentTare == LOW && lastTareButtonState == HIGH) {
            tarePressed = true;
            Serial.println("Button TARE pressed");
        }
    }
    lastTareButtonState = currentTare;
    
    // MODE button
    if (currentMode != lastModeButtonState) {
        lastModeTime = currentTime;
    }
    
    if ((currentTime - lastModeTime) > debounceDelay) {
        if (currentMode == LOW && lastModeButtonState == HIGH) {
            modePressed = true;
            Serial.println("Button MODE pressed");
        }
    }
    lastModeButtonState = currentMode;
}

bool ButtonManager::isHoldPressed() {
    return holdPressed;
}

bool ButtonManager::isTarePressed() {
    return tarePressed;
}

bool ButtonManager::isModePressed() {
    return modePressed;
}

void ButtonManager::resetPresses() {
    holdPressed = false;
    tarePressed = false;
    modePressed = false;
}