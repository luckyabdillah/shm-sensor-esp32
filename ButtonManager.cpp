#include "ButtonManager.h"

void ButtonManager::begin() {
    pinMode(HOLD_PIN, INPUT_PULLUP);
    pinMode(TARE_PIN, INPUT_PULLUP);
    pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);

    // Initialize last states based on actual pin levels to avoid false triggers
    lastHoldButtonState = digitalRead(HOLD_PIN);
    lastTareButtonState = digitalRead(TARE_PIN);
    lastModeButtonState = digitalRead(MODE_SWITCH_PIN);

    Serial.print("Buttons on pins H/T/M: ");
    Serial.print(HOLD_PIN); Serial.print("/");
    Serial.print(TARE_PIN); Serial.print("/");
    Serial.println(MODE_SWITCH_PIN);
}

void ButtonManager::update() {
    unsigned long currentTime = millis();
    
    // Read button states
    bool currentHold = digitalRead(HOLD_PIN);
    bool currentTare = digitalRead(TARE_PIN);
    bool currentMode = digitalRead(MODE_SWITCH_PIN);
    
    // HOLD button (debounced tap detection that fires once while held)
    if (currentHold != lastHoldButtonState) {
        lastHoldTime = currentTime;
        lastHoldButtonState = currentHold;
    }
    if ((currentTime - lastHoldTime) > debounceDelay) {
        if (currentHold == LOW && !holdActive) {
            holdActive = true;
            holdPressed = true; // latch until consumed
            Serial.println("Button HOLD pressed");
        } else if (currentHold == HIGH && holdActive) {
            holdActive = false; // ready for next press
        }
    }

    // TARE button
    if (currentTare != lastTareButtonState) {
        lastTareTime = currentTime;
        lastTareButtonState = currentTare;
    }
    if ((currentTime - lastTareTime) > debounceDelay) {
        if (currentTare == LOW && !tareActive) {
            tareActive = true;
            tarePressed = true;
            Serial.println("Button TARE pressed");
        } else if (currentTare == HIGH && tareActive) {
            tareActive = false;
        }
    }

    // MODE button
    if (currentMode != lastModeButtonState) {
        lastModeTime = currentTime;
        lastModeButtonState = currentMode;
    }
    if ((currentTime - lastModeTime) > debounceDelay) {
        if (currentMode == LOW && !modeActive) {
            modeActive = true;
            modePressed = true;
            Serial.println("Button MODE pressed");
        } else if (currentMode == HIGH && modeActive) {
            modeActive = false;
        }
    }
}

bool ButtonManager::isHoldPressed() {
    bool wasPressed = holdPressed;
    holdPressed = false;
    return wasPressed;
}

bool ButtonManager::isTarePressed() {
    bool wasPressed = tarePressed;
    tarePressed = false;
    return wasPressed;
}

bool ButtonManager::isModePressed() {
    bool wasPressed = modePressed;
    modePressed = false;
    return wasPressed;
}

void ButtonManager::resetPresses() {
    holdPressed = false;
    tarePressed = false;
    modePressed = false;
}