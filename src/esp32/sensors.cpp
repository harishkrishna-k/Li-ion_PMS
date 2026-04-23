/**
 * @file sensors.cpp
 * @brief Advanced sensor reading functions with signal processing
 * @author Project Contributor
 * @license MIT
 */

#include "sensors.h"

// Analog pin definitions
const int TEMP_SENSOR_PIN = 34;
const int VOLTAGE_SENSOR_PIN = 39;
const int CURRENT_SENSOR_PIN = 36;

// Voltage divider resistors
const float R1 = 30000.0;
const float R2 = 7500.0;

// ADC reference voltage
const float ADC_REF_VOLTAGE = 33;

// ACS712 calibration
const float ACS712_SENSITIVITY = 185.0;

// Filter sample size
const int FILTER_SIZE = 10;

// Filter buffers
float tempBuffer[FILTER_SIZE];
float voltageBuffer[FILTER_SIZE];
float currentBuffer[FILTER_SIZE];

// Filter indices
int tempIndex = 0;
int voltageIndex = 0;
int currentIndex = 0;

// Filter initialized flags
bool tempFilterInit = false;
bool voltageFilterInit = false;
bool currentFilterInit = false;

// Calibration offsets
float voltageOffset = 0.0;
float currentOffset = 0.0;
float tempOffset = 0.0;

// Initialize sensor system
void sensorsInit() {
    pinMode(TEMP_SENSOR_PIN, INPUT);
    pinMode(VOLTAGE_SENSOR_PIN, INPUT);
    pinMode(CURRENT_SENSOR_PIN, INPUT);

    // Configure ESP32 ADC
    analogReadResolution(12);
    analogSetAttenuation(ADC_0_3V);

    // Initialize filter buffers
    for (int i = 0; i < FILTER_SIZE; i++) {
        tempBuffer[i] = 0;
        voltageBuffer[i] = 0;
        currentBuffer[i] = 0;
    }

    Serial.println("Sensors initialized with oversampling filter");
}

// Moving average filter
float movingAverage(float newValue, float* buffer, int& index, int size) {
    buffer[index] = newValue;
    index = (index + 1) % size;
    
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += buffer[i];
    }
    return sum / size;
}

// Oversampling ADC (32 samples for better resolution)
int oversampleADC(int pin, int samples = 32) {
    long sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(pin);
    }
    // Average and shift for oversampling (divide by 32 gives same result as input / 32)
    // But we want to add extra bits to resolution
    // 32 samples = 5 extra bits of resolution (log2(32) = 5)
    return sum >> 5;  // Divide by 32
}

// Read temperature from LM35 with filtering
float readTemperature() {
    int rawValue = oversampleADC(TEMP_SENSOR_PIN, 32);
    
    // Convert ADC to voltage (10mV per degree Celsius)
    float voltage = (rawValue / 4095.0) * ADC_REF_VOLTAGE;
    float temperature = voltage * 100.0;

    // Apply calibration offset
    temperature += tempOffset;

    // Apply moving average filter
    temperature = movingAverage(temperature, tempBuffer, tempIndex, FILTER_SIZE);

    return temperature;
}

// Read voltage from divider with filtering
float readVoltage() {
    int rawValue = oversampleADC(VOLTAGE_SENSOR_PIN, 32);

    // Convert ADC to voltage
    float adcVoltage = (rawValue / 4095.0) * ADC_REF_VOLTAGE;
    
    // Apply voltage divider formula
    float actualVoltage = adcVoltage * ((R1 + R2) / R2);

    // Apply calibration offset
    actualVoltage += voltageOffset;

    // Apply moving average filter
    actualVoltage = movingAverage(actualVoltage, voltageBuffer, voltageIndex, FILTER_SIZE);

    return actualVoltage;
}

// Read current from ACS712 with filtering
float readCurrent() {
    int rawValue = oversampleADC(CURRENT_SENSOR_PIN, 32);

    // Convert ADC to voltage
    float voltage = (rawValue / 4095.0) * ADC_REF_VOLTAGE;

    // Calculate current (subtract 2.5V offset for bidirectional)
    float current = (voltage - 2.5) / (ACS712_SENSITIVITY / 1000.0);

    // Apply calibration offset
    current += currentOffset;

    // Handle negative current (discharge)
    if (current < 0.01) {
        current = 0;
    }

    // Apply moving average filter
    current = movingAverage(current, currentBuffer, currentIndex, FILTER_SIZE);

    return current;
}

// Calculate State of Charge
float calculateSOC(float voltage) {
    const float MAX_VOLTAGE = 12.9;
    const float MIN_VOLTAGE = 8.0;

    float soc = ((voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100.0;

    if (soc > 100.0) soc = 100.0;
    if (soc < 0.0) soc = 0.0;

    return soc;
}

// Calibration routine
void calibrateSensors() {
    Serial.println("Starting sensor calibration...");
    Serial.println("Ensure no load on system for 5 seconds...");

    // Wait for settling
    delay(5000);

    // Capture offset values (no load condition)
    float sumVoltage = 0;
    float sumCurrent = 0;
    float sumTemp = 0;
    int samples = 100;

    for (int i = 0; i < samples; i++) {
        sumVoltage += readVoltage();
        sumCurrent += readCurrent();
        sumTemp += readTemperature();
        delay(50);
    }

    voltageOffset = -(sumVoltage / samples);
    currentOffset = -(sumCurrent / samples);
    tempOffset = -(sumTemp / samples);

    Serial.println("Calibration complete!");
    Serial.print("Voltage Offset: ");
    Serial.println(voltageOffset);
    Serial.print("Current Offset: ");
    Serial.println(currentOffset);
    Serial.print("Temperature Offset: ");
    Serial.println(tempOffset);
}

// Read raw values without filter (for testing)
float readRawTemperature() {
    int rawValue = analogRead(TEMP_SENSOR_PIN);
    float voltage = (rawValue / 4095.0) * ADC_REF_VOLTAGE;
    return voltage * 100.0;
}

float readRawVoltage() {
    int rawValue = analogRead(VOLTAGE_SENSOR_PIN);
    float adcVoltage = (rawValue / 4095.0) * ADC_REF_VOLTAGE;
    return adcVoltage * ((R1 + R2) / R2);
}

float readRawCurrent() {
    int rawValue = analogRead(CURRENT_SENSOR_PIN);
    float voltage = (rawValue / 4095.0) * ADC_REF_VOLTAGE;
    float current = (voltage - 2.5) / (ACS712_SENSITIVITY / 1000.0);
    return current < 0 ? 0 : current;
}

// Check for sensor faults
bool checkSensorFault() {
    float temp = readRawTemperature();
    float voltage = readRawVoltage();
    
    // Check for open circuit (0V or max)
    if (temp < 0.1 || temp > 3.3) return true;
    if (voltage < 0.1) return true;

    return false;
}