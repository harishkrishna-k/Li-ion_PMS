/**
 * @file database.cpp
 * @brief Supabase database client for remote metrics storage
 * @author Project Contributor
 * @license MIT
 */

#include "database.h"
#include <HTTPClient.h>

// WiFi client for HTTP requests
WiFiClientSecure WiFiClient;

// Database configuration (set these in config.env or hardcode for testing)
const char* SUPABASE_URL = "https://your-project-id.supabase.co";
const char* SUPABASE_ANON_KEY = "your-anon-key";

// Table name
const char* TABLE_NAME = "sensor_readings";

Database::Database() {
    _connected = false;
    _lastPostTime = 0;
}

void Database::init(const char* url, const char* key) {
    SUPABASE_URL = url;
    SUPABASE_ANON_KEY = key;
    _connected = true;
}

/**
 * @brief Send sensor data to Supabase
 * @param voltage Battery voltage
 * @param current Battery current
 * @param temperature Battery temperature
 * @param batteryStatus "charging", "discharging", or "full"
 * @return HTTP response code (negative = error)
 */
int Database::sendReading(float voltage, float current, float temperature, const char* batteryStatus) {
    if (!_connected) {
        return -1;
    }

    HTTPClient http;
    WiFiClient client;
    
    // Build the REST API URL
    String url = String(SUPABASE_URL) + "/rest/v1/" + TABLE_NAME;
    
    // Configure HTTP client
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", SUPABASE_ANON_KEY);
    http.addHeader("Authorization", String("Bearer ") + SUPABASE_ANON_KEY);
    http.addHeader("Prefer", "return=minimal");

    // Create JSON payload
    String payload = "{";
    payload += "\"voltage\":" + String(voltage, 2) + ",";
    payload += "\"current\":" + String(current, 2) + ",";
    payload += "\"temperature\":" + String(temperature, 1) + ",";
    payload += "\"battery_status\":\"" + String(batteryStatus) + "\"";
    payload += "}";

    // Send POST request
    int httpCode = http.POST(payload);

    // Clean up
    http.end();

    return httpCode;
}

/**
 * @brief Check database connection status
 * @return Connection status
 */
bool Database::isConnected() {
    return _connected;
}

/**
 * @brief Get last post timestamp
 * @return Last successful post time
 */
unsigned long Database::getLastPostTime() {
    return _lastPostTime;
}

/**
 * @brief Set connection status
 * @param connected Connection status
 */
void Database::setConnected(bool connected) {
    _connected = connected;
}