#include <Arduino.h>
#include "FirebaseManager.h"

void FirebaseManager::begin() {
    config.api_key = apiKey;
    config.database_url = firebaseHost;

    auth.user.email = userEmail;
    auth.user.password = userPassword;

    // First try to sign up with email/password (only needed once).
    bool signupOk = Firebase.signUp(&config, &auth, userEmail, userPassword);
    if (!signupOk) {
        // If the user already exists, continue with the provided credentials.
        if (config.signer.signupError.message.indexOf("EMAIL_EXISTS") == -1) {
            Serial.printf("Firebase signup error: %s\n", config.signer.signupError.message.c_str());
        }
    }

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    initialized = Firebase.ready();

    if (initialized) {
        Serial.println("Firebase connected");
    } else {
        Serial.println("Firebase not ready, check API key / email / password / DB rules");
    }
}

bool FirebaseManager::isReady() const {
    return initialized && Firebase.ready();
}

void FirebaseManager::sendLoadCellData(float load) {
    if (!isReady()) return;

    time_t ts = time(nullptr);
    String path = "/loadCells/" + String(ts) + "/load";

    if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), load)) {
        Serial.println("Firebase OK - Load Cell");
    } else {
        Serial.printf("Firebase error: %s\n", fbdo.errorReason().c_str());
    }
}

void FirebaseManager::sendStrainGaugeData(float avgVoltage, float deltaL, float load, 
                                         float strain, float stress, float vr) {
    if (!isReady()) return;

    time_t ts = time(nullptr);
    String path = "/strainGauges/" + String(ts);

    FirebaseJson json;
    json.set("avgVoltage", avgVoltage);
    json.set("deltaL", deltaL);
    json.set("load", load);
    json.set("strain", strain);
    json.set("stress", stress);
    json.set("vr", vr);

    if (Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
        Serial.println("Firebase OK - Strain Gauge");
    } else {
        Serial.printf("Firebase error: %s\n", fbdo.errorReason().c_str());
    }
}

void FirebaseManager::sendAlert(const char* message, const char* type) {
    if (!isReady()) return;

    time_t ts = time(nullptr);
    String path = "/alerts/" + String(ts);

    FirebaseJson json;
    json.set("message", message);
    json.set("type", type);

    if (Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
        Serial.println("Firebase OK - Alert");
    } else {
        Serial.printf("Firebase error: %s\n", fbdo.errorReason().c_str());
    }
}