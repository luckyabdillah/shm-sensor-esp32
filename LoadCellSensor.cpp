#include "LoadCellSensor.h"
#include "DisplayManager.h"

extern DisplayManager display;

void LoadCellSensor::begin() {
    scale.begin(DOUT_PIN, CLK_PIN);
    scale.set_scale(calibration_factor);
    tare();
}

void LoadCellSensor::update() {
    if (!holdMode) {
        currentWeight = scale.get_units(40);
        // dead zone biar nol bersih
        if (abs(currentWeight) < 5) currentWeight = 0;
    }
}

void LoadCellSensor::tare() {
    // Show loading message on LCD during tare
    display.showMessage("TARE LOAD CELL", "Loading...");

    Serial.println("TARE LOAD CELL START");
    scale.tare();
    currentWeight = 0;
    holdWeight = 0;

    // Clear loading message after tare
    display.clear();
    Serial.println("TARE LOAD CELL DONE");
}

void LoadCellSensor::toggleHold() {
    holdMode = !holdMode;
    if (holdMode) {
        holdWeight = currentWeight;
    }
}

float LoadCellSensor::getWeight() const {
    return holdMode ? holdWeight : currentWeight;
}

bool LoadCellSensor::isHold() const {
    return holdMode;
}