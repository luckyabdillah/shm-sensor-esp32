#include "ButtonManager.h"

void ButtonManager::begin() {
    pinMode(HOLD_PIN, INPUT_PULLUP);
    pinMode(TARE_PIN, INPUT_PULLUP);
    pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);

    // Initialize all states based on actual pin levels to avoid false triggers
    bool holdPin = digitalRead(HOLD_PIN);
    bool tarePin = digitalRead(TARE_PIN);
    bool modePin = digitalRead(MODE_SWITCH_PIN);
    
    holdButtonState = holdPin;
    lastHoldButtonState = holdPin;
    
    lastTareButtonState = tarePin;
    taredebouncedState = tarePin;
    
    lastModeButtonState = modePin;
    modeDebouncedState = modePin;

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
    
    // ========== HOLD button (toggle/latch button) ==========
    // Detects state transitions and triggers toggle
    if (currentHold != holdButtonState) {
        holdButtonState = currentHold;
        lastHoldTime = currentTime;
    }
    // Apply debounce: check if enough time passed since last change
    if ((currentTime - lastHoldTime) >= debounceDelay) {
        if (holdButtonState != lastHoldButtonState) {
            holdPressed = true;
            lastHoldButtonState = holdButtonState;
            Serial.print("Button HOLD state: ");
            Serial.println(holdButtonState == LOW ? "LATCHED (in)" : "RELEASED (out)");
        }
    }

    // ========== TARE button (momentary button) ==========
    // Detect transition from HIGH to LOW (button press)
    if (currentTare != lastTareButtonState) {
        taredebouncedState = lastTareButtonState;
        lastTareTime = currentTime;
        lastTareButtonState = currentTare;
    }
    // Apply debounce and detect LOW state
    if ((currentTime - lastTareTime) >= debounceDelay) {
        if (currentTare == LOW && taredebouncedState == HIGH) {
            tarePressed = true;
            Serial.println("Button TARE pressed");
        }
        taredebouncedState = currentTare;
    }

    // ========== MODE button (momentary button) ==========
    // Detect transition from HIGH to LOW (button press)
    if (currentMode != lastModeButtonState) {
        modeDebouncedState = lastModeButtonState;
        lastModeTime = currentTime;
        lastModeButtonState = currentMode;
    }
    // Apply debounce and detect LOW state
    if ((currentTime - lastModeTime) >= debounceDelay) {
        if (currentMode == LOW && modeDebouncedState == HIGH) {
            modePressed = true;
            Serial.println("Button MODE pressed");
        }
        modeDebouncedState = currentMode;
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