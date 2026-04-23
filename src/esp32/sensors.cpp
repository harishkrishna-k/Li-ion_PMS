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
    : tempFilter(10)
    , voltageFilter(10)
    , currentFilter(10)
    , voltageOffset(0.0f)
    , currentOffset(0.0f)
    , tempOffset(0.0f) {}

void Sensors::init() {
    pinMode(TEMP_SENSOR_PIN, INPUT);
    pinMode(VOLTAGE_SENSOR_PIN, INPUT);
    pinMode(CURRENT_SENSOR_PIN, INPUT);

    analogReadResolution(12);
    analogSetAttenuation(ADC_0_3V);

    Serial.println("Sensors initialized (simple averaging)");
}

float Sensors::readTemperature() {
    int raw = analogRead(TEMP_SENSOR_PIN);
    float voltage = (raw / 4095.0f) * ADC_REF_VOLTAGE;
    float temperature = voltage * 100.0f + tempOffset;
    return tempFilter.update(temperature);
}

float Sensors::readVoltage() {
    int raw = analogRead(VOLTAGE_SENSOR_PIN);
    float adcVoltage = (raw / 4095.0f) * ADC_REF_VOLTAGE;
    float actualVoltage = adcVoltage * ((R1 + R2) / R2) + voltageOffset;
    return voltageFilter.update(actualVoltage);
}

float Sensors::readCurrent() {
    int raw = analogRead(CURRENT_SENSOR_PIN);
    float voltage = (raw / 4095.0f) * ADC_REF_VOLTAGE;
    float current = (voltage - 2.5f) / (ACS712_SENSITIVITY / 1000.0f);
    if (current < 0.01f) current = 0;
    current += currentOffset;
    return currentFilter.update(current);
}

float Sensors::calculateSOC(float voltage) {
    const float MAX_VOLTAGE = 12.9f;
    const float MIN_VOLTAGE = 8.0f;
    float soc = ((voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100.0f;
    return constrain(soc, 0.0f, 100.0f);
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