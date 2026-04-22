/**
 * @file main.ino
 * @brief Li-ion Battery Performance Monitoring System (Li-ion_PMS)
 * @author Harish Krishna K, Jayaram H, Navin Y
 * @license MIT
 *
 * Hardware: NodeMCU ESP32
 * Sensors: LM35 (temperature), ACS712 (current), Voltage Divider
 * Display: 16x2 LCD (I2C)
 * Storage: Supabase PostgreSQL (remote)
 *
 * Pin Configuration:
 * - GPIO 34: LM35 Temperature Sensor
 * - GPIO 39: Voltage Divider
 * - GPIO 36: ACS712 Current Sensor
 * - GPIO 32: Charger Detection
 * - GPIO 27: Relay Control
 * - GPIO 26: BLDC Fan Control
 * - GPIO 21: LCD SDA
 * - GPIO 22: LCD SCL
 */

#include <Arduino.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>

// Include custom libraries
#include "sensors.h"
#include "lcd_display.h"
#include "relay_control.h"
#include "database.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

// WiFi credentials - UPDATE THESE
const char* WIFI_SSID = "YourNetworkName";
const char* WIFI_PASSWORD = "YourPassword";

// Supabase configuration - UPDATE THESE (or use config.env)
const char* SUPABASE_URL = "https://your-project-id.supabase.co";
const char* SUPABASE_ANON_KEY = "your-anon-key";

// Device name
const char* DEVICE_NAME = "Li-ion_PMS_001";

// Battery thresholds
const float CHARGING_THRESHOLD = 12.9;    // Stop charging at this voltage
const float DISCHARGING_THRESHOLD = 8.0;  // Cutoff voltage

// Timing
const unsigned long POST_INTERVAL = 5000;  // Post to DB every 5 seconds
const unsigned long DISPLAY_INTERVAL = 1500; // Update display every 1.5 seconds

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

Sensors sensors;
LCDDisplay display;
RelayControl relay;
Database database;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

bool chargerConnected = false;
unsigned long lastDisplayUpdate = 0;
unsigned long lastDBPost = 0;
bool wifiConnected = false;
bool dbConnected = false;

// ============================================================================
// PROTOTYPES
// ============================================================================

void connectWiFi();
void checkChargerStatus();
void handleBatteryManagement(float voltage, float temperature);


// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== Li-ion Battery Performance Monitoring System ===");
    Serial.println();

    // Initialize components
    sensors.init();
    display.init();
    relay.init();
    database.init(SUPABASE_URL, SUPABASE_ANON_KEY);

    // Display startup message
    display.displayStartup();

    // Connect to WiFi
    connectWiFi();

    Serial.println("Setup complete!");
    Serial.println();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    // Read sensor data
    float voltage = sensors.readVoltage();
    float current = sensors.readCurrent();
    float temperature = sensors.readTemperature();
    float soc = sensors.calculateSOC(voltage);

    // Check charger status
    checkChargerStatus();

    // Battery management (relay and fan control)
    handleBatteryManagement(voltage, temperature);

    // Update display
    unsigned long currentTime = millis();
    if (currentTime - lastDisplayUpdate >= DISPLAY_INTERVAL) {
        display.displayStatus(voltage, current, temperature, soc);
        display.displayChargingStatus(chargerConnected);
        display.displayWiFiStatus(wifiConnected);
        display.displayDBStatus(dbConnected);
        lastDisplayUpdate = currentTime;
    }

    // Post to Supabase
    if (currentTime - lastDBPost >= POST_INTERVAL && wifiConnected) {
        const char* status = chargerConnected ? "charging" : "discharging";
        int result = database.sendReading(voltage, current, temperature, status);

        if (result > 0) {
            dbConnected = true;
            Serial.print("DB: Posted - V:");
            Serial.print(voltage);
            Serial.print(" I:");
            Serial.print(current);
            Serial.print(" T:");
            Serial.println(temperature);
        } else {
            dbConnected = false;
            Serial.print("DB Error: ");
            Serial.println(result);
        }
        lastDBPost = currentTime;
    }

    // Log to serial
    Serial.print("V:");
    Serial.print(voltage);
    Serial.print(" I:");
    Serial.print(current);
    Serial.print(" T:");
    Serial.print(temperature);
    Serial.print(" SOC:");
    Serial.print(soc);
    Serial.print(" Chg:");
    Serial.println(chargerConnected ? "Yes" : "No");
}


// ============================================================================
// FUNCTIONS
// ============================================================================

/**
 * @brief Connect to WiFi
 */
void connectWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println();
        Serial.print("WiFi connected! IP: ");
        Serial.println(WiFi.localIP());
    } else {
        wifiConnected = false;
        Serial.println();
        Serial.println("WiFi connection failed!");
    }
}

/**
 * @brief Check charger connection status
 */
void checkChargerStatus() {
    // Using GPIO 32 with internal pull-up
    // Charger pulls this pin LOW when connected
    chargerConnected = (digitalRead(32) == LOW);
}

/**
 * @brief Handle battery management (charging/discharging relay)
 */
void handleBatteryManagement(float voltage, float temperature) {
    // Control fan based on temperature
    relay.controlFan(temperature, chargerConnected);

    // Battery relay control
    if (chargerConnected) {
        // Charging mode
        if (voltage < CHARGING_THRESHOLD && voltage > DISCHARGING_THRESHOLD) {
            relay.setRelay(true);  // Enable charging
        } else if (voltage >= CHARGING_THRESHOLD) {
            relay.setRelay(true);  // Keep connected until removed
            display.displayAlert("FULL");
        } else {
            relay.setRelay(true);  // Keep charging if low
        }
    } else {
        // Discharging mode
        if (voltage < DISCHARGING_THRESHOLD) {
            relay.setRelay(false);  // Cutoff
            display.displayAlert("LOW BATT");
        } else {
            relay.setRelay(false);  // Allow discharge
        }
    }
}