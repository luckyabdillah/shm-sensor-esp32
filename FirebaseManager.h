#pragma once

#include <Firebase_ESP_Client.h>
#include "config.h"
#include "time.h"

class FirebaseManager {
private:
    // Firebase configuration
    const char* firebaseHost = FIREBASE_HOST;
    const char* apiKey = API_KEY;
    const char* userEmail = DB_EMAIL;
    const char* userPassword = DB_PASS;
    
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;
    
    bool initialized = false;
    
public:
    void begin();
    bool isReady() const;
    
    void sendLoadCellData(float load);
    void sendStrainGaugeData(float avgVoltage, float deltaL, float load, 
                            float strain, float stress, float vr);
    void sendAlert(const char* message, const char* type);
};