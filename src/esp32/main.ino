/**
 * @file main.ino
 * @brief Li-ion Battery Performance Monitoring System v2.0
 * @author Harish Krishna K, Jayaram H, Navin Y
 * @license MIT
 *
 * Hardware: NodeMCU ESP32 + LM35 + ACS712 + 16x2 LCD I2C
 * Cloud: Supabase PostgreSQL
 * 
 * Features:
 * - Non-blocking architecture (millis timers)
 * - Signal processing (oversampling + moving average)
 * - PID fan control
 * - WiFi Manager (web portal)
 * - OTA updates
 * - Deep sleep power saving
 */

#include <Arduino.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>

// Custom includes
#include "sensors.h"
#include "lcd_display.h"
#include "relay_control.h"
#include "database.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

// WiFi (handled by WiFi Manager - credentials stored in flash)
char WIFI_SSID[32] = "";
char WIFI_PASSWORD[32] = "";

// Supabase configuration
const char* SUPABASE_URL = "https://your-project.supabase.co";
const char* SUPABASE_ANON_KEY = "your-anon-key";
const char* DEVICE_NAME = "Li-ion_PMS_001";

// Battery thresholds
const float CHARGING_FULL = 12.9;
const float DISCHARGING_CUTOFF = 8.0;

// Timing (millis-based - non-blocking)
const unsigned long SENSOR_READ_INTERVAL = 1000;    // Read sensors every 1 second
const unsigned long DISPLAY_INTERVAL = 2000;      // Update LCD every 2 seconds
const unsigned long DB_POST_INTERVAL = 5000;    // Post to DB every 5 seconds
const unsigned long SLEEP_INTERVAL = 30000;        // Sleep after 30 seconds of inactivity

// Power management
bool deepSleepEnabled = true;
unsigned long lastActivityTime = 0;

// Mode flags
bool normalMode = true;
bool chargingMode = false;
bool wifiConnected = false;
bool dbConnected = false;

// Task timers
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastDbPost = 0;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== Li-ion Battery Monitoring System v2.0 ===");
    Serial.println("Features: Non-blocking, PID control, WiFi Manager, OTA");

    // Initialize components
    sensorsInit();
    lcdInit();
    relayInit();
    databaseInit(SUPABASE_URL, SUPABASE_ANON_KEY);

    // Display startup
    displayStartup();

    // Initialize WiFi Manager
    WiFiManager wifiManager;
    wifiManager.setTimeout(60);
    
    // Try to connect, or start AP
    if (!wifiManager.autoConnect("Li-ion_PMS_AP")) {
        Serial.println("Failed to connect - starting AP");
        lcdClear();
        lcd.print("AP Mode");
    }

    // Connected
    wifiConnected = (WiFi.status() == WL_CONNECTED);
    displayWiFiStatus(wifiConnected);

    if (wifiConnected) {
        Serial.print("Connected! IP: ");
        Serial.println(WiFi.localIP());
    }

    // Initialize OTA
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {
            type = "filesystem";
        }
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();

    // Update activity time
    lastActivityTime = millis();

    Serial.println("Setup complete!");
    Serial.println();
}

// ============================================================================
// MAIN LOOP (Non-blocking)
// ============================================================================

void loop() {
    // Handle OTA updates
    ArduinoOTA.handle();

    unsigned long now = millis();

    // Wake from deep sleep on USB activity or timer
    if (Serial.available()) {
        lastActivityTime = now;
    }

    // ========== SENSOR READING ==========
    if (now - lastSensorRead >= SENSOR_READ_INTERVAL) {
        readSensors();
        lastSensorRead = now;
        lastActivityTime = now;
    }

    // ========== LCD UPDATE ==========
    if (now - lastDisplayUpdate >= DISPLAY_INTERVAL) {
        updateDisplay();
        lastDisplayUpdate = now;
    }

    // ========== DATABASE POST ==========
    if (now - lastDbPost >= DB_POST_INTERVAL && wifiConnected) {
        postToDatabase();
        lastDbPost = now;
    }

    // ========== POWER MANAGEMENT ==========
    if (deepSleepEnabled && (now - lastActivityTime > SLEEP_INTERVAL)) {
        enterDeepSleep();
    }

    // Small delay to prevent watchdog
    delay(10);
}

// ============================================================================
// FUNCTIONS
// ============================================================================

void readSensors() {
    // Read all sensors with filtering
    float voltage = readVoltage();
    float current = readCurrent();
    float temperature = readTemperature();
    float soc = calculateSOC(voltage);

    // Determine charging mode
    chargingMode = checkChargerStatus();

    // Battery management
    manageBattery(voltage, temperature);

    // Serial logging
    Serial.print("V:");
    Serial.print(voltage);
    Serial.print(" I:");
    Serial.print(current);
    Serial.print(" T:");
    Serial.print(temperature);
    Serial.print(" SOC:");
    Serial.print(soc);
    Serial.print(" | ");
    Serial.println(chargingMode ? "Charging" : "Discharging");
}

void updateDisplay() {
    float voltage = readVoltage();
    float current = readCurrent();
    float temperature = readTemperature();
    float soc = calculateSOC(voltage);

    // Use rotating display for more info
    displayRotating(voltage, current, temperature, soc, chargingMode);
}

void postToDatabase() {
    float voltage = readVoltage();
    float current = readCurrent();
    float temperature = readTemperature();

    const char* status = chargingMode ? "charging" : "discharging";
    int result = databaseSendReading(voltage, current, temperature, status);

    dbConnected = (result > 0);

    displayDBStatus(dbConnected);

    if (result > 0) {
        Serial.println("DB: Posted successfully");
    } else {
        Serial.print("DB Error: ");
        Serial.println(result);
    }
}

void manageBattery(float voltage, float temperature) {
    // Control fan with PID
    controlFan(temperature, chargingMode, true);

    // Relay control
    if (chargingMode) {
        if (voltage < CHARGING_FULL && voltage > DISCHARGING_CUTOFF) {
            setRelay(true);
        } else if (voltage >= CHARGING_FULL) {
            setRelay(true);
            displayAlert("FULL");
        } else {
            setRelay(true);
        }
    } else {
        if (voltage < DISCHARGING_CUTOFF) {
            setRelay(false);
            displayAlert("LOW");
        } else {
            setRelay(false);
        }
    }
}

bool checkChargerStatus() {
    pinMode(32, INPUT_PULLUP);
    return (digitalRead(32) == LOW);
}

void enterDeepSleep() {
    Serial.println("Entering deep sleep...");
    lcdClear();
    lcd.print("Sleeping...");
    delay(500);

    // Turn off peripherals
    digitalWrite(26, LOW);  // Fan off
    digitalWrite(27, LOW);  // Relay off

    // Configure wake sources
    esp_sleep_enable_gpio_wakeup();
    esp_sleep_enable_timer_wakeup(60 * 1000000);  // 60 seconds

    // Enter deep sleep
    esp_deep_sleep_start();
}