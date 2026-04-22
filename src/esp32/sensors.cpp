/**
 * @file sensors.cpp
 * @brief Sensor reading functions for LM35, ACS712, and voltage divider
 * @author Project Contributor
 * @license MIT
 */

#include "sensors.h"

// Analog pin definitions
const int TEMP_SENSOR_PIN = 34;    // LM35 on GPIO 34
const int VOLTAGE_SENSOR_PIN = 39;  // Voltage divider on GPIO 39
const int CURRENT_SENSOR_PIN = 36;   // ACS712 on GPIO 36

// Voltage divider resistors (modify according to your circuit)
const float R1 = 30000.0;  // 30kΩ resistor
const float R2 = 7500.0;    // 7.5kΩ resistor

// ADC reference voltage
const float ADC_REF_VOLTAGE = 3.3;

// ACS712 calibration (mV per ampere)
// ACS712-5A: 185 mV/A
// ACS712-20A: 100 mV/A
// ACS712-30A: 66 mV/A
const float ACS712_SENSITIVITY = 185.0;  // mV/A for 5A module

Sensors::Sensors() {
    _lastVoltage = 0.0;
    _lastCurrent = 0.0;
    _lastTemperature = 0.0;
}

void Sensors::init() {
    pinMode(TEMP_SENSOR_PIN, INPUT);
    pinMode(VOLTAGE_SENSOR_PIN, INPUT);
    pinMode(CURRENT_SENSOR_PIN, INPUT);

    // Initialize ADC resolution for ESP32
    analogReadResolution(12);
    analogSetAttenuation(ADC_0_3V);  // 0-3.3V range
}

/**
 * @brief Read temperature from LM35 sensor
 * @return Temperature in Celsius
 */
float Sensors::readTemperature() {
    int adcValue = analogRead(TEMP_SENSOR_PIN);

    // Convert ADC value to voltage (LM35: 10mV/°C)
    float voltage = (adcValue / 4095.0) * ADC_REF_VOLTAGE;
    float temperature = voltage * 100.0;  // 10mV per degree Celsius

    _lastTemperature = temperature;
    return temperature;
}

/**
 * @brief Read voltage from battery via voltage divider
 * @return Voltage in volts
 */
float Sensors::readVoltage() {
    int adcValue = analogRead(VOLTAGE_SENSOR_PIN);

    // Convert ADC to actual voltage (accounting for divider)
    float adcVoltage = (adcValue / 4095.0) * ADC_REF_VOLTAGE;
    float actualVoltage = adcVoltage * ((R1 + R2) / R2);

    _lastVoltage = actualVoltage;
    return actualVoltage;
}

/**
 * @brief Read current from ACS712 sensor
 * @return Current in amperes
 */
float Sensors::readCurrent() {
    int adcValue = analogRead(CURRENT_SENSOR_PIN);

    // Convert ADC to voltage
    float voltage = (adcValue / 4095.0) * ADC_REF_VOLTAGE;

    // Calculate current (subtract 2.5V offset for bidirectional sensing)
    float current = (voltage - 2.5) / (ACS712_SENSITIVITY / 1000.0);

    // Handle negative current (discharging)
    if (current < 0.01) {
        current = 0.0;
    }

    _lastCurrent = current;
    return current;
}

/**
 * @brief Get last stored voltage reading
 * @return Last voltage value
 */
float Sensors::getLastVoltage() {
    return _lastVoltage;
}

/**
 * @brief Get last stored current reading
 * @return Last current value
 */
float Sensors::getLastCurrent() {
    return _lastCurrent;
}

/**
 * @brief Get last stored temperature reading
 * @return Last temperature value
 */
float Sensors::getLastTemperature() {
    return _lastTemperature;
}

/**
 * @brief Calculate battery state of charge percentage
 * @param voltage Current battery voltage
 * @return SOC percentage (0-100)
 */
float Sensors::calculateSOC(float voltage) {
    // For 3S Li-ion pack (11.1V nominal)
    // Full: 12.9V, Empty: 8.0V
    const float MAX_VOLTAGE = 12.9;
    const float MIN_VOLTAGE = 8.0;

    float soc = ((voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100.0;

    // Clamp to 0-100%
    if (soc > 100.0) soc = 100.0;
    if (soc < 0.0) soc = 0.0;

    return soc;
}