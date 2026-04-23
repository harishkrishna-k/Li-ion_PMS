/**
 * @file main.ino
 * @brief Li-ion Battery Performance Monitoring System v3.0 (Kalman + Async)
 * @author Harish Krishna K, Jayaram H, Navin Y
 * @license MIT
 * 
 * Hardware: NodeMCU ESP32 + LM35 + ACS712-5A + 16x2 LCD I2C
 */

#include <Arduino.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <esp_task_wdt.h>

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

const float CHARGING_FULL = 12.6f;     // 3S Li-ion standard (4.2V/cell)
const float DISCHARGING_CUTOFF = 9.0f; // 3S Li-ion standard (3.0V/cell)
const float TEMP_CRITICAL = 55.0f;

const int PIN_CHARGER_DETECT = 32;
const int WDT_TIMEOUT_S = 10;

// Timing intervals (non-blocking)
const unsigned long SENSOR_READ_INTERVAL = 500; 
const unsigned long DISPLAY_INTERVAL = 1000;
const unsigned long DB_POST_INTERVAL = 10000;  

// ============================================================================
// Main Application State
// ============================================================================

enum SystemMode { MODE_IDLE, MODE_CHARGING, MODE_DISCHARGING, MODE_FAULT };

Sensors sensors;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Task timing
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastDbPost = 0;

// State
struct SystemState {
    SystemMode mode = MODE_IDLE;
    bool wifiConnected = false;
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
    Serial.println("\n=== Li-ion BMS v3.0 (Kalman + Async) ===");

    sensors.init();
    lcd.begin(21, 22);
    lcd.backlight();
    lcd.clear();
    
    relayInit();
    databaseInit(SUPABASE_URL, SUPABASE_ANON_KEY);
    databaseStartTask();

    pinMode(PIN_CHARGER_DETECT, INPUT_PULLUP);

    lcd.setCursor(0, 0);
    lcd.print("BMS v3.0 Async");
    
    // Initialize WDT
    esp_task_wdt_init(WDT_TIMEOUT_S, true);
    esp_task_wdt_add(NULL);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int wifiAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
        delay(500);
        Serial.print(".");
        wifiAttempts++;
        esp_task_wdt_reset();
    }
    
    state.wifiConnected = (WiFi.status() == WL_CONNECTED);
    Serial.println(state.wifiConnected ? "\nWiFi: Connected" : "\nWiFi: Failed");

    lastSensorRead = millis();
    lastDisplayUpdate = millis();
    lastDbPost = millis();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    esp_task_wdt_reset();
    unsigned long now = millis();

    // ===== Sensor Reading =====
    if (now - lastSensorRead >= SENSOR_READ_INTERVAL) {
        state.voltage = sensors.readVoltage();
        state.current = sensors.readCurrent();
        state.temperature = sensors.readTemperature();
        state.soc = sensors.calculateSOC(state.voltage);
        
        updateSystemMode();
        manageBattery(state.voltage, state.temperature);
        
        lastSensorRead = now;
    }

    // ===== Display Update =====
    if (now - lastDisplayUpdate >= DISPLAY_INTERVAL) {
        displayData(state.voltage, state.current, state.temperature, state.soc);
        lastDisplayUpdate = now;
    }

    // ===== Database Post (Non-blocking Queue) =====
    if (now - lastDbPost >= DB_POST_INTERVAL && state.wifiConnected) {
        const char* statusStr = (state.mode == MODE_CHARGING) ? "charging" : 
                               (state.mode == MODE_FAULT) ? "critical" : "discharging";
        
        databaseSendReading(state.voltage, state.current, state.temperature, statusStr);
        lastDbPost = now;
    }
}

// ============================================================================
// Functions
// ============================================================================

void updateSystemMode() {
    bool chargerPresent = (digitalRead(PIN_CHARGER_DETECT) == LOW);
    
    if (state.temperature > TEMP_CRITICAL) {
        state.mode = MODE_FAULT;
    } else if (chargerPresent) {
        state.mode = MODE_CHARGING;
    } else if (abs(state.current) > 0.1) {
        state.mode = MODE_DISCHARGING;
    } else {
        state.mode = MODE_IDLE;
    }
}

void displayData(float voltage, float current, float temperature, float soc) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("V:"); lcd.print(voltage, 1);
    lcd.print(" I:"); lcd.print(current, 1);
    
    lcd.setCursor(0, 1);
    const char* modeLabel = (state.mode == MODE_CHARGING) ? "CHG" : 
                           (state.mode == MODE_FAULT) ? "ERR" : "DSG";
    lcd.print(modeLabel);
    lcd.print(" T:"); lcd.print(temperature, 0);
    lcd.print(" SOC:"); lcd.print(soc, 0);
    lcd.print("%");
}

void manageBattery(float voltage, float temperature) {
    // Fan control with PID (Now enabled for reliability)
    controlFan(temperature, state.mode == MODE_CHARGING, true);
    
    // Safety Relay Logic with Hysteresis
    static bool relayEngaged = true;
    
    if (state.mode == MODE_FAULT) {
        relayEngaged = false;
    } else if (state.mode == MODE_CHARGING) {
        if (voltage >= CHARGING_FULL) relayEngaged = false;
        else if (voltage < CHARGING_FULL - 0.2) relayEngaged = true;
    } else {
        if (voltage <= DISCHARGING_CUTOFF) relayEngaged = false;
        else if (voltage > DISCHARGING_CUTOFF + 0.5) relayEngaged = true;
    }
    
    setRelay(relayEngaged);
}
