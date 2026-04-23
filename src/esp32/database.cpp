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

// FreeRTOS components
static QueueHandle_t dbQueue = NULL;
static TaskHandle_t dbTaskHandle = NULL;

// Forward declaration of the internal POST function
int sendPost(const char* payload);

// The worker task that runs in the background
void dbWorkerTask(void* parameter) {
    DBReading reading;
    char payload[160];

    Serial.println("DB Task: Started on Core " + String(xPortGetCoreID()));

    for (;;) {
        // Wait for data in the queue (blocks this task, but not the main loop)
        if (xQueueReceive(dbQueue, &reading, portMAX_DELAY) == pdPASS) {
            if (WiFi.status() == WL_CONNECTED) {
                snprintf(payload, sizeof(payload),
                    "{\"voltage\":%.2f,\"current\":%.2f,\"temperature\":%.1f,\"battery_status\":\"%s\"}",
                    reading.voltage, reading.current, reading.temperature, reading.status);

                int httpCode = sendPost(payload);
                
                if (httpCode > 0) {
                    _lastPostTime = millis();
                    Serial.println("DB Task: Record sent successfully");
                } else {
                    Serial.println("DB Task: POST failed, code: " + String(httpCode));
                }
            } else {
                Serial.println("DB Task: WiFi disconnected, skipping");
            }
        }
    }
}

// Initialize database connection
void databaseInit(const char* url, const char* key) {
    BASE_URL = url;
    ANON_KEY = key;
    _connected = true;
    
    // Create queue for 10 readings
    dbQueue = xQueueCreate(10, sizeof(DBReading));
    
    Serial.println("Database client initialized (Async via FreeRTOS)");
}

// Start the background task
void databaseStartTask() {
    if (dbTaskHandle == NULL) {
        // Run on Core 0 to leave Core 1 for main application/WiFi
        xTaskCreatePinnedToCore(
            dbWorkerTask,   // Task function
            "DBWorker",     // Name
            8192,           // Stack size
            NULL,           // Parameter
            1,              // Priority
            &dbTaskHandle,  // Handle
            0               // Core
        );
    }
}

// Send reading to queue (non-blocking)
int databaseSendReading(float voltage, float current, float temperature, const char* batteryStatus) {
    if (!_connected || dbQueue == NULL) return -1;

    DBReading reading;
    reading.voltage = voltage;
    reading.current = current;
    reading.temperature = temperature;
    strncpy(reading.status, batteryStatus, sizeof(reading.status) - 1);
    reading.status[sizeof(reading.status) - 1] = '\0';

    // Try to add to queue, don't wait if full (non-blocking)
    if (xQueueSend(dbQueue, &reading, 0) == pdPASS) {
        return 1; // Success (queued)
    } else {
        Serial.println("DB: Queue full, dropping record");
        return -2;
    }
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