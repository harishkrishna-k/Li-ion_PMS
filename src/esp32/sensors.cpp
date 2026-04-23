/**
 * @file sensors.cpp
 * @brief Sensor processing with proper encapsulation
 * @author Project Contributor
 * @license MIT
 * 
 * Note: This implementation uses simple arrays for clarity.
 * For production, consider using ESP-IDF's filter component.
 */

#include "sensors.h"

// ============================================================================
// Configuration
// ============================================================================

const int TEMP_SENSOR_PIN = 34;
const int VOLTAGE_SENSOR_PIN = 39;
const int CURRENT_SENSOR_PIN = 36;

const float R1 = 30000.0;
const float R2 = 7500.0;
const float ADC_REF_VOLTAGE = 3.3;
const float ACS712_SENSITIVITY = 185.0;

// ============================================================================
// Moving Average Filter Class Implementation
// ============================================================================

MovingAverageFilter::MovingAverageFilter(int size) : size(size), index(0), initialized(false) {
    buffer = new float[size];
    for (int i = 0; i < size; i++) buffer[i] = 0;
}

MovingAverageFilter::~MovingAverageFilter() {
    delete[] buffer;
}

float MovingAverageFilter::update(float value) {
    buffer[index] = value;
    index = (index + 1) % size;
    if (!initialized && index == 0) initialized = true;
    
    return getAverage();
}

float MovingAverageFilter::getAverage() {
    int count = initialized ? size : index;
    if (count == 0) return 0;
    
    float sum = 0;
    for (int i = 0; i < count; i++) sum += buffer[i];
    return sum / count;
}

void MovingAverageFilter::reset() {
    for (int i = 0; i < size; i++) buffer[i] = 0;
    index = 0;
    initialized = false;
}

// ============================================================================
// Global Sensor Object
// ============================================================================

Sensors::Sensors() 
    : tempFilter(15)
    , voltageFilter(0.005, 0.05, 12.0) // Process noise, Measurement noise, Initial value
    , currentFilter(0.01, 0.5, 0.0)    // Current is noisier, higher R
    , voltageOffset(0.0f)
    , currentOffset(0.0f)
    , tempOffset(0.0f) {}

void Sensors::init() {
    pinMode(TEMP_SENSOR_PIN, INPUT);
    pinMode(VOLTAGE_SENSOR_PIN, INPUT);
    pinMode(CURRENT_SENSOR_PIN, INPUT);

    analogReadResolution(12);
    // Use 11dB attenuation for 0-3.6V range on ESP32
    analogSetAttenuation(ADC_ATTEN_DB_11);

    Serial.println("Sensors initialized (Kalman filtering)");
}

float Sensors::readTemperature() {
    int raw = analogRead(TEMP_SENSOR_PIN);
    // 12-bit ADC, 11dB attenuation gives ~3.6V full scale
    float voltage = (raw / 4095.0f) * 3.6f;
    float temperature = voltage * 100.0f + tempOffset;
    return tempFilter.update(temperature);
}

float Sensors::readVoltage() {
    int raw = analogRead(VOLTAGE_SENSOR_PIN);
    float adcVoltage = (raw / 4095.0f) * 3.6f;
    float actualVoltage = adcVoltage * ((R1 + R2) / R2) + voltageOffset;
    return voltageFilter.update(actualVoltage);
}

float Sensors::readCurrent() {
    int raw = analogRead(CURRENT_SENSOR_PIN);
    float voltage = (raw / 4095.0f) * 3.6f;
    // ACS712-5A is 185mV/A. If powered by 5V, offset is 2.5V.
    // If we use a divider to bring 5V range to 3.3V range for ESP32:
    // We assume the user has a divider or a 3.3V version.
    // Standard ACS712-5A: I = (Vout - Vcc/2) / sensitivity
    float current = (voltage - 2.5f) / (ACS712_SENSITIVITY / 1000.0f);
    if (abs(current) < 0.05f) current = 0; // Deadzone for noise
    current += currentOffset;
    return currentFilter.update(current);
}

float Sensors::calculateSOC(float voltage) {
    // 3S Li-ion Battery Discharge Curve (Typical)
    // Voltage per cell: 3.0V (0%) to 4.2V (100%)
    // Pack voltage: 9.0V to 12.6V
    
    struct VoltagePoint {
        float voltage;
        float soc;
    };

    static const VoltagePoint lut[] = {
        {12.60, 100}, {12.30, 90}, {12.10, 80}, {11.90, 70},
        {11.70, 60},  {11.45, 50}, {11.30, 40}, {11.10, 30},
        {10.80, 20},  {10.50, 10}, {9.00, 0}
    };
    const int lutSize = sizeof(lut) / sizeof(lut[0]);

    if (voltage >= lut[0].voltage) return 100.0f;
    if (voltage <= lut[lutSize - 1].voltage) return 0.0f;

    for (int i = 0; i < lutSize - 1; i++) {
        if (voltage <= lut[i].voltage && voltage > lut[i+1].voltage) {
            // Linear interpolation between points
            float vRange = lut[i].voltage - lut[i+1].voltage;
            float socRange = lut[i].soc - lut[i+1].soc;
            return lut[i+1].soc + (voltage - lut[i+1].voltage) * (socRange / vRange);
        }
    }
    return 0.0f;
}

void Sensors::calibrate() {
    Serial.println("Calibration: waiting 3 seconds for stabilization...");
    delay(3000);
    
    float sumV = 0, sumC = 0, sumT = 0;
    const int samples = 20;
    
    for (int i = 0; i < samples; i++) {
        sumV += readRawVoltage();
        sumC += readRawCurrent();
        sumT += readRawTemperature();
        delay(100);
    }
    
    voltageOffset = -(sumV / samples);
    currentOffset = -(sumC / samples);
    tempOffset = -(sumT / samples);
    
    Serial.println("Calibration complete");
}

float Sensors::readRawTemperature() {
    int raw = analogRead(TEMP_SENSOR_PIN);
    return (raw / 4095.0f) * ADC_REF_VOLTAGE * 100.0f;
}

float Sensors::readRawVoltage() {
    int raw = analogRead(VOLTAGE_SENSOR_PIN);
    float adcVoltage = (raw / 4095.0f) * ADC_REF_VOLTAGE;
    return adcVoltage * ((R1 + R2) / R2);
}

float Sensors::readRawCurrent() {
    int raw = analogRead(CURRENT_SENSOR_PIN);
    float voltage = (raw / 4095.0f) * ADC_REF_VOLTAGE;
    float current = (voltage - 2.5f) / (ACS712_SENSITIVITY / 1000.0f);
    return current < 0 ? 0 : current;
}

bool Sensors::checkFault() {
    float temp = readRawTemperature();
    float voltage = readRawVoltage();
    // Check for open circuit (0V or very low voltage at sensor)
    return (temp < 0.1f || temp > 3.2f || voltage < 0.1f);
}