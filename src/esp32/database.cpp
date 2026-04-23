/**
 * @file database.cpp
 * @brief Supabase database client with retry logic
 * @author Project Contributor
 * @license MIT
 */

#include "database.h"

WiFiClientSecure WiFiClient;
HTTPClient http;

const char* BASE_URL = "";
const char* ANON_KEY = "";
const char* TABLE_NAME = "sensor_readings";

bool _connected = false;
unsigned long _lastPostTime = 0;
int _retryCount = 0;
const int MAX_RETRIES = 3;

// Initialize database connection
void databaseInit(const char* url, const char* key) {
    BASE_URL = url;
    ANON_KEY = key;
    _connected = true;
    _retryCount = 0;
    Serial.println("Database client initialized");
}

// Send reading to Supabase with retry logic
int databaseSendReading(float voltage, float current, float temperature, const char* batteryStatus) {
    if (!_connected) return -1;

    _retryCount = 0;
    int httpCode = -1;

    while (_retryCount < MAX_RETRIES) {
        httpCode = sendPost(voltage, current, temperature, batteryStatus);

        if (httpCode > 0) {
            _lastPostTime = millis();
            return httpCode;
        }

        _retryCount++;
        delay(100 * _retryCount);  // Backoff
    }

    Serial.print("DB: Failed after ");
    Serial.print(MAX_RETRIES);
    Serial.println(" retries");
    return httpCode;
}

// Internal POST function
int sendPost(float voltage, float current, float temperature, const char* batteryStatus) {
    String url = String(BASE_URL) + "/rest/v1/" + TABLE_NAME;

    http.begin(WiFiClient, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", ANON_KEY);
    http.addHeader("Authorization", String("Bearer ") + ANON_KEY);
    http.addHeader("Prefer", "return=minimal");

    // JSON payload
    String payload = "{";
    payload += "\"voltage\":" + String(voltage, 2) + ",";
    payload += "\"current\":" + String(current, 2) + ",";
    payload += "\"temperature\":" + String(temperature, 1) + ",";
    payload += "\"battery_status\":\"" + String(batteryStatus) + "\"";
    payload += "}";

    int code = http.POST(payload);
    http.end();

    return code;
}

// Check connection status
bool databaseConnected() {
    return _connected;
}

// Get last post time
unsigned long getLastPostTime() {
    return _lastPostTime;
}

// Set connection status
void setDatabaseConnected(bool status) {
    _connected = status;
}

// Get retry count
int getRetryCount() {
    return _retryCount;
}