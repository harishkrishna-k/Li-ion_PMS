/**
 * @file lcd_display.cpp
 * @brief LCD 16x2 display interface
 * @author Project Contributor
 * @license MIT
 */

#include "lcd_display.h"

// LCD pin definitions (I2C)
// SDA -> GPIO 21
// SCL -> GPIO 22
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16 cols, 2 rows

LCDDisplay::LCDDisplay() {
    _lastUpdateTime = 0;
}

void LCDDisplay::init() {
    lcd.begin(21, 22);  // SDA=21, SCL=22 for ESP32
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Li-ion BMS Init");
    delay(1000);
    lcd.clear();
}

/**
 * @brief Display battery status on LCD
 * @param voltage Battery voltage
 * @param current Battery current
 * @param temperature Battery temperature
 * @param soc State of charge percentage
 */
void LCDDisplay::displayStatus(float voltage, float current, float temperature, float soc) {
    // First line: Voltage and Current
    lcd.setCursor(0, 0);
    lcd.print("V:");
    lcd.print(voltage, 1);
    lcd.print("V ");
    lcd.print("I:");
    lcd.print(current, 1);
    lcd.print("A ");

    // Second line: Temperature and SOC
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(temperature, 0);
    lcd.print((char)223);  // Degree symbol
    lcd.print("C ");
    lcd.print("SOC:");
    lcd.print(soc, 0);
    lcd.print("%");
}

/**
 * @brief Display charging status
 * @param isCharging True if charger connected
 */
void LCDDisplay::displayChargingStatus(bool isCharging) {
    lcd.setCursor(10, 1);
    if (isCharging) {
        lcd.print("CHG ");
    } else {
        lcd.print("DIS ");
    }
}

/**
 * @brief Display alert message
 * @param message Alert text (max 16 chars)
 */
void LCDDisplay::displayAlert(const char* message) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALERT:");
    lcd.setCursor(0, 1);
    lcd.print(message);
}

/**
 * @brief Clear LCD screen
 */
void LCDDisplay::clear() {
    lcd.clear();
}

/**
 * @brief Display startup screen
 */
void LCDDisplay::displayStartup() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Li-ion Battery");
    lcd.setCursor(0, 1);
    lcd.print("Monitoring System");
    delay(2000);
    lcd.clear();
}

/**
 * @brief Display WiFi connection status
 * @param connected Connection status
 */
void LCDDisplay::displayWiFiStatus(bool connected) {
    lcd.setCursor(12, 0);
    if (connected) {
        lcd.print("WiFi");
    } else {
        lcd.print("---");
    }
}

/**
 * @brief Display DB connection status
 * @param connected Connection status
 */
void LCDDisplay::displayDBStatus(bool connected) {
    lcd.setCursor(12, 1);
    if (connected) {
        lcd.print("DB");
    } else {
        lcd.print("--");
    }
}