/**
 * @file lcd_display.cpp
 * @brief LCD 16x2 display interface with optimized refresh
 * @author Project Contributor
 * @license MIT
 */

#include "lcd_display.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long lastUpdateTime = 0;
const unsigned long UPDATE_INTERVAL = 2000;  // Update LCD every 2 seconds

bool lcdInitialized = false;

// Last displayed values (to avoid flicker)
float lastVoltage = 0;
float lastCurrent = 0;
float lastTemp = 0;
float lastSoc = 0;
int displayPage = 0;
unsigned long pageChangeTime = 0;
const unsigned long PAGE_INTERVAL = 3000;  // Rotate pages every 3 seconds

// Initialize LCD
void lcdInit() {
    lcd.begin(21, 22);
    lcd.backlight();
    lcd.clear();
    
    lcd.setCursor(0, 0);
    lcd.print("Li-ion PMS Init");
    delay(1000);
    
    lcdInitialized = true;
    pageChangeTime = millis();
    Serial.println("LCD display initialized");
}

// Display main status (voltage, current, temp, SOC)
void displayStatus(float voltage, float current, float temperature, float soc) {
    unsigned long now = millis();
    
    // Check if we should update
    if (now - lastUpdateTime < UPDATE_INTERVAL) {
        // Only update on significant change
        if (abs(voltage - lastVoltage) < 0.1 && 
            abs(current - lastCurrent) < 0.1 &&
            abs(temperature - lastTemp) < 1.0) {
            return;
        }
    }
    
    lastUpdateTime = now;
    lastVoltage = voltage;
    lastCurrent = current;
    lastTemp = temperature;
    lastSoc = soc;

    // Page 0: Voltage and Current
    if (displayPage == 0) {
        lcd.setCursor(0, 0);
        lcd.print("V:");
        lcd.print(voltage, 1);
        lcd.print("V ");
        lcd.print("I:");
        lcd.print(current, 1);
        lcd.print("A ");
        
        lcd.setCursor(0, 1);
        lcd.print("T:");
        lcd.print(temperature, 0);
        lcd.print((char)223);
        lcd.print("C SOC:");
        lcd.print(soc, 0);
        lcd.print("%");
    }
    
    // Fill remaining spaces with spaces to clear old values
    lcd.print("                ");
}

// Display charging status
void displayChargingStatus(bool isCharging) {
    lcd.setCursor(14, 0);
    if (isCharging) {
        lcd.print("CHG");
    } else {
        lcd.print("DIS");
    }
}

// Display alert message
void displayAlert(const char* message) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALERT:");
    lcd.setCursor(0, 1);
    lcd.print(message);
    // Ensure cursor doesn't stick
    lcd.cursor();
    delay(100);
    lcd.noCursor();
}

// Clear LCD
void lcdClear() {
    lcd.clear();
}

// Display startup screen
void displayStartup() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Li-ion Battery");
    lcd.setCursor(0, 1);
    lcd.print("Monitor System");
    delay(2000);
    lcd.clear();
}

// Display WiFi status
void displayWiFiStatus(bool connected) {
    lcd.setCursor(12, 0);
    if (connected) {
        lcd.print("WiF");
    } else {
        lcd.print("---");
    }
}

// Display DB status
void displayDBStatus(bool connected) {
    lcd.setCursor(12, 1);
    if (connected) {
        lcd.print("DB");
    } else {
        lcd.print("--");
    }
}

// Display rotating information pages
void displayRotating(float voltage, float current, float temperature, float soc, bool charging) {
    unsigned long now = millis();
    
    // Change page every 3 seconds
    if (now - pageChangeTime > PAGE_INTERVAL) {
        displayPage = (displayPage + 1) % 3;
        pageChangeTime = now;
        lcd.clear();
    }
    
    switch(displayPage) {
        case 0:  // Quick status
            displayStatus(voltage, current, temperature, soc);
            break;
            
        case 1:  // Battery info
            lcd.setCursor(0, 0);
            lcd.print("Battery Monitor");
            lcd.setCursor(0, 1);
            if (charging) {
                lcd.print("Mode: Charging ");
            } else {
                lcd.print("Mode: Using   ");
            }
            break;
            
        case 2:  // System info
            lcd.setCursor(0, 0);
            lcd.print("Li-ion_PMS v2.0");
            lcd.setCursor(0, 1);
            lcd.print("ESP32 IoT System");
            break;
    }
}

// Display with animation
void displayAnimated(float voltage, float current, float temperature, float soc) {
    // Simple loading animation for busy state
    static int animFrame = 0;
    const char animChars[] = {'|', '/', '-', '\\'};
    
    lcd.setCursor(15, 1);
    lcd.print(animChars[animFrame]);
    animFrame = (animFrame + 1) % 4;
    
    displayStatus(voltage, current, temperature, soc);
}