/**
 * @file lcd_display.h
 * @brief Header file for LCD display
 * @author Project Contributor
 * @license MIT
 */

#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class LCDDisplay {
public:
    LCDDisplay();
    void init();
    void displayStatus(float voltage, float current, float temperature, float soc);
    void displayChargingStatus(bool isCharging);
    void displayAlert(const char* message);
    void clear();
    void displayStartup();
    void displayWiFiStatus(bool connected);
    void displayDBStatus(bool connected);

private:
    unsigned long _lastUpdateTime;
};

#endif // LCD_DISPLAY_H