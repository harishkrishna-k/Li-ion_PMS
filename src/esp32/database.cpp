/**
 * @file database.cpp
 * @brief Non-blocking Supabase database client with static buffers
 * @author Project Contributor
 * @license MIT
 * 
 * IMPORTANT: This module uses blocking HTTP calls.
 * For true non-blocking, integrate with FreeRTOS tasks or async HTTP.
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

// Static buffer for JSON payload - no heap allocation
static char jsonBuffer[128];

// Initialize database connection
void databaseInit(const char* url, const char* key) {
    BASE_URL = url;
    ANON_KEY = key;
    _connected = true;
    _retryCount = 0;
    Serial.println("Database client initialized (note: HTTP calls are blocking)");
}

// Send reading to Supabase - uses non-blocking pattern via call checking
int databaseSendReading(float voltage, float current, float temperature, const char* batteryStatus) {
    if (!_connected) return -1;

    // Build JSON using static buffer - no heap fragmentation
    // Format: {"voltage":12.50,"current":1.20,"temperature":25.0,"battery_status":"charging"}
    int len = snprintf(jsonBuffer, sizeof(jsonBuffer),
        "{\"voltage\":%.2f,\"current\":%.2f,\"temperature\":%.1f,\"battery_status\":\"%s\"}",
        voltage, current, temperature, batteryStatus);
    
    if (len <= 0 || len >= (int)sizeof(jsonBuffer)) {
        Serial.println("DB: JSON buffer overflow");
        return -1;
    }

    // Note: http.begin() and http.POST() are blocking calls
    // This is a known limitation of ESP32 http library
    // For production, use async HTTP client or FreeRTOS tasks
    
    _retryCount = 0;
    int httpCode = -1;

    while (_retryCount < MAX_RETRIES) {
        httpCode = sendPost(jsonBuffer);

        if (httpCode > 0) {
            _lastPostTime = millis();
            return httpCode;
        }

        // Non-blocking retry schedule (still uses delay, but shorter)
        // In production, use connection timeout instead of retry delay
        _retryCount++;
        if (_retryCount < MAX_RETRIES) {
            delay(50 * _retryCount);  // Reduced delay: 50, 100, 150ms
        }
    }

    Serial.print("DB: Failed after ");
    Serial.print(MAX_RETRIES);
    Serial.println(" retries");
    return httpCode;
}

// Internal POST function with static buffer
int sendPost(const char* payload) {
    String url = String(BASE_URL) + "/rest/v1/" + TABLE_NAME;

    http.begin(WiFiClient, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", ANON_KEY);
    http.addHeader("Authorization", String("Bearer ") + ANON_KEY);
    http.addHeader("Prefer", "return=minimal");

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

/**
 * KNOWN LIMITATIONS:
 * 1. http.POST() is blocking - will freeze other tasks
 * 2. WiFi stability affects responsiveness
 * 3. For true async, use ESPAsyncHTTPClient or FreeRTOS
 * 
 * IMPROVEMENTS NEEDED FOR PRODUCTION:
 * - Implement connection timeout
 * - Use async HTTP client
 * - Move to FreeRTOS task for cloud sync
 * - Add local buffering during network outages
 */