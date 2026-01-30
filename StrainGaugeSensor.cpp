#include "StrainGaugeSensor.h"
#include <Arduino.h>

void StrainGaugeSensor::begin() {
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    
    // Initialize buffer
    for (int i = 0; i < N; i++) {
        adcBuffer[i] = 0;
    }
    adcSum = 0;
    
    tare();
}

void StrainGaugeSensor::update() {
    if (!holdMode) {
        adcSum -= adcBuffer[idx];
        adcBuffer[idx] = analogRead(SENSOR_PIN);
        adcSum += adcBuffer[idx];
        idx = (idx + 1) % N;

        adcAvg = adcSum / (float)N;
        adcNet = offsetAdc - adcAvg;  // REVERSE POLARITY

        // Filter noise
        if (abs(adcNet) < noiseThresholdAdc) adcNet = 0;

        Vout = adcNet * Vref / ADC_MAX;
        Vr = Vout / (Vref * gain);
        strain = (4 * Vr) / (gf * (1 + 2 * Vr));

        deltaL = strain * panjangPlat;
        stress = strain * modulusE;
        loadPercent = calculateLoadPercent(strain);

        updateStrainHoldValues();
    }
    
    // Update alert tracking
    SystemStatus currentStatus = getStatus();
    if (currentStatus != lastAlertStatus) {
        alertSent = false;
        lastAlertStatus = currentStatus;
    }
}

void StrainGaugeSensor::tare() {
    isTaring = true;

    Serial.println("\n=== TARE STRAIN GAUGE START ===");
    Serial.println("Pastikan beban = 0 dan plat diam...");
    delay(1000);

    const int samples = 400;
    long sum = 0;
    uint64_t sumSq = 0;

    for (int i = 0; i < samples; i++) {
        int adc = analogRead(SENSOR_PIN);
        sum += adc;
        sumSq += (uint64_t)adc * adc;
        delay(5);
    }

    offsetAdc = sum / (float)samples;

    float meanSq = sumSq / (float)samples;
    noiseAdc = sqrt(meanSq - offsetAdc * offsetAdc);
    noiseThresholdAdc = 3 * noiseAdc;   // 3σ

    // Reset moving average buffer
    adcSum = 0;
    for (int i = 0; i < N; i++) {
        adcBuffer[i] = offsetAdc;
        adcSum += offsetAdc;
    }

    Serial.println("=== TARE STRAIN GAUGE DONE ===");
    Serial.print("Offset ADC       : "); Serial.println(offsetAdc, 3);
    Serial.print("Noise ADC (σ)    : "); Serial.println(noiseAdc, 3);
    Serial.print("Threshold ADC    : "); Serial.println(noiseThresholdAdc, 3);

    isTaring = false;
}

void StrainGaugeSensor::toggleHold() {
    holdMode = !holdMode;
    if (holdMode) {
        updateStrainHoldValues();
    }
}

void StrainGaugeSensor::updateStrainHoldValues() {
    holdAdcAvg = adcAvg;
    holdAdcNet = adcNet;
    holdVout = Vout;
    holdVr = Vr;
    holdStrain = strain;
    holdDeltaL = deltaL;
    holdStress = stress;
    holdLoadPercent = loadPercent;
}

float StrainGaugeSensor::calculateLoadPercent(float strain) {
    float percent = (strain / STRAIN_MAX) * 100.0;
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    return percent;
}

SystemStatus StrainGaugeSensor::getStatus() const {
    float currentLoad = holdMode ? holdLoadPercent : loadPercent;
    if (currentLoad >= 90) return STATUS_DANGER;
    if (currentLoad >= 70) return STATUS_WARNING;
    if (currentLoad >= 50) return STATUS_NOTICE;
    return STATUS_NORMAL;
}

float StrainGaugeSensor::getLoadPercent() const {
    return holdMode ? holdLoadPercent : loadPercent;
}

float StrainGaugeSensor::getStrain() const {
    return holdMode ? holdStrain : strain;
}

float StrainGaugeSensor::getVout() const {
    return holdMode ? holdVout : Vout;
}

float StrainGaugeSensor::getDeltaL() const {
    return holdMode ? holdDeltaL : deltaL;
}

float StrainGaugeSensor::getStress() const {
    return holdMode ? holdStress : stress;
}

float StrainGaugeSensor::getVr() const {
    return holdMode ? holdVr : Vr;
}

bool StrainGaugeSensor::isHold() const {
    return holdMode;
}

void StrainGaugeSensor::updateBuzzerAndLED() {
    SystemStatus status = getStatus();
    
    switch (status) {
        case STATUS_NORMAL:
            digitalWrite(BUZZER_PIN, LOW);
            digitalWrite(LED_PIN, LOW);
            break;
        case STATUS_NOTICE:
            digitalWrite(BUZZER_PIN, LOW);
            digitalWrite(LED_PIN, HIGH);
            break;
        case STATUS_WARNING:
        case STATUS_DANGER:
            digitalWrite(BUZZER_PIN, HIGH);
            digitalWrite(LED_PIN, HIGH);
            break;
    }
}

bool StrainGaugeSensor::shouldSendAlert() const {
    SystemStatus status = getStatus();
    return (status != STATUS_NORMAL && !alertSent);
}

const char* StrainGaugeSensor::getAlertMessage() const {
    SystemStatus status = getStatus();
    switch (status) {
        case STATUS_NOTICE: return "Strain level elevated";
        case STATUS_WARNING: return "High strain detected";
        case STATUS_DANGER: return "Load capacity exceeded (≥90%)";
        default: return "";
    }
}

const char* StrainGaugeSensor::getAlertType() const {
    SystemStatus status = getStatus();
    switch (status) {
        case STATUS_NOTICE: return "info";
        case STATUS_WARNING: return "warning";
        case STATUS_DANGER: return "danger";
        default: return "";
    }
}

void StrainGaugeSensor::printSerialOutput() const {
    Serial.println("\n================================");
    Serial.print("Mode                     : ");
    Serial.println(holdMode ? "HOLD" : "LIVE");

    Serial.print("Beban (%)                : ");
    Serial.print(getLoadPercent(), 1);
    Serial.println("%");

    Serial.print("ADC avg (analog read)    : ");
    Serial.print(holdMode ? holdAdcNet : adcNet, 3);
    Serial.println();

    Serial.print("Tegangan rata-rata       : ");
    Serial.print(getVout(), 6);
    Serial.println(" V");

    Serial.print("Vr                       : ");
    Serial.print(getVr(), 6);
    Serial.println();

    Serial.print("Strain                   : ");
    Serial.print(getStrain(), 6);
    Serial.println();

    Serial.print("Pertambahan Panjang (ΔL) : ");
    Serial.print(getDeltaL(), 9);
    Serial.print(" m  |  ");
    Serial.print(getDeltaL() * 1000, 3);
    Serial.println(" mm");

    Serial.print("Tegangan / Stress        : ");
    Serial.print(getStress(), 2);
    Serial.println(" Pa");
}