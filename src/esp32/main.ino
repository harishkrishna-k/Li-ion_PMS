/**
 * @file main.ino
 * @brief Li-ion Battery Performance Monitoring System v2.1
 * @author Harish Krishna K, Jayaram H, Navin Y
 * @license MIT
 *
 * IMPORTANT DISCLAIMERS - READ BEFORE USE:
 * 
 * 1. BLOCKING OPERATIONS: This code uses blocking HTTP calls for Supabase.
 *    The http.POST() call will freeze the ESP32 during network operations.
 *    For production, use FreeRTOS tasks or async HTTP client.
 * 
 * 2. MOCK DATA: The portfolio dashboard (/docs) displays MOCK DATA,
 *    not real cloud data. The frontend generates random values for
 *    demonstration purposes. To see real data, connect to Supabase
 *    and implement REST API fetching in the frontend.
 * 
 * 3. MEMORY: This code uses simple averaging filters. For long-running
 *    deployments, consider using ESP-IDF's filter component to avoid
 *    heap fragmentation.
 * 
 * 4. SENSOR ACCURACY: Calibration offsets are set to 0.0 by default.
 *    Run calibrateSensors() with no load to calibrate sensors.
 * 
 * Hardware: NodeMCU ESP32 + LM35 + ACS712-5A + 16x2 LCD I2C
 */

#include <Arduino.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>

#include "sensors.h"
#include "lcd_display.h"
#include "relay_control.h"
#include "database.h"

// ============================================================================
// Configuration
// ============================================================================

const char* WIFI_SSID = "YourNetworkName";
const char* WIFI_PASSWORD = "YourPassword";

const char* SUPABASE_URL = "https://your-project.supabase.co";
const char* SUPABASE_ANON_KEY = "your-anon-key";
const char* DEVICE_NAME = "Li-ion_PMS_001";

const float CHARGING_FULL = 12.9f;
const float DISCHARGING_CUTOFF = 8.0f;
const float TEMP_CRITICAL = 60.0f;

// Timing intervals (non-blocking)
const unsigned long SENSOR_READ_INTERVAL = 1000;
const unsigned long DISPLAY_INTERVAL = 2000;
const unsigned long DB_POST_INTERVAL = 5000;

// ============================================================================
// Main Application State
// ============================================================================

Sensors sensors;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Task timing
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastDbPost = 0;

// State
struct SystemState {
    bool charging = false;
    bool wifiConnected = false;
    bool dbConnected = false;
    float voltage = 0;
    float current = 0;
    float temperature = 0;
    float soc = 0;
} state;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== Li-ion BMS v2.1 ===");
    Serial.println("Note: HTTP calls are blocking - see caveats in code");

    sensors.init();
    lcd.begin(21, 22);
    lcd.backlight();
    lcd.clear();
    
    relayInit();
    databaseInit(SUPABASE_URL, SUPABASE_ANON_KEY);

    lcd.setCursor(0, 0);
    lcd.print("Li-ion BMS v2.1");
    delay(1000);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int wifiAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && wifiAttempts < 30) {
        delay(500);
        Serial.print(".");
        wifiAttempts++;
    }
    
    state.wifiConnected = (WiFi.status() == WL_CONNECTED);
    
    Serial.println();
    if (state.wifiConnected) {
        Serial.print("WiFi: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi: Failed");
    }

    lastSensorRead = millis();
    lastDisplayUpdate = millis();
    lastDbPost = millis();

    Serial.println("Setup complete");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    unsigned long now = millis();

    // ===== Sensor Reading =====
    if (now - lastSensorRead >= SENSOR_READ_INTERVAL) {
        state.voltage = sensors.readVoltage();
        state.current = sensors.readCurrent();
        state.temperature = sensors.readTemperature();
        state.soc = sensors.calculateSOC(state.voltage);
        state.charging = checkCharger();
        
        manageBattery(state.voltage, state.temperature);
        
        lastSensorRead = now;
    }

    // ===== Display Update =====
    if (now - lastDisplayUpdate >= DISPLAY_INTERVAL) {
        displayData(state.voltage, state.current, state.temperature, state.soc);
        lastDisplayUpdate = now;
    }

    // ===== Database Post =====
    if (now - lastDbPost >= DB_POST_INTERVAL && state.wifiConnected) {
        const char* status = state.charging ? "charging" : "discharging";
        int result = databaseSendReading(state.voltage, state.current, state.temperature, status);
        state.dbConnected = (result > 0);
        
        Serial.print("DB: ");
        Serial.println(state.dbConnected ? "OK" : "FAILED");
        
        lastDbPost = now;
    }

    // Small delay to prevent watchdog
    delay(10);
}

// ============================================================================
// Functions
// ============================================================================

void displayData(float voltage, float current, float temperature, float soc) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("V:");
    lcd.print(voltage, 1);
    lcd.print(" I:");
    lcd.print(current, 1);
    
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(temperature, 0);
    lcd.print((char)223);
    lcd.print("C ");
    lcd.print("SOC:");
    lcd.print(soc, 0);
    lcd.print("%");
}

void manageBattery(float voltage, float temperature) {
    // Fan control with simple threshold
    relayControlSimple(temperature, state.charging);
    
    // Relay control
    if (state.charging) {
        if (voltage < CHARGING_FULL && voltage > DISCHARGING_CUTOFF) {
            setRelay(true);
        } else {
            setRelay(voltage >= CHARGING_FULL);
        }
    } else {
        setRelay(voltage < DISCHARGING_CUTOFF);
    }
    
    // Critical temperature warning
    if (temperature > TEMP_CRITICAL) {
        Serial.println("WARNING: Critical temperature!");
    }
}

bool checkCharger() {
    pinMode(32, INPUT_PULLUP);
    return digitalRead(32) == LOW;
}