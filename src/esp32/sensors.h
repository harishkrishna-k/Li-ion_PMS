/**
 * @file sensors.h
 * @brief Header for sensor processing with proper encapsulation
 * @author Project Contributor
 * @license MIT
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// ============================================================================
// Moving Average Filter Class
// ============================================================================

class MovingAverageFilter {
private:
    float* buffer;
    int size;
    int index;
    bool initialized;

public:
    MovingAverageFilter(int size = 10);
    ~MovingAverageFilter();
    float update(float value);
    float getAverage();
    void reset();
};

// ============================================================================
// Sensors Class
// ============================================================================

class Sensors {
private:
    MovingAverageFilter tempFilter;
    MovingAverageFilter voltageFilter;
    MovingAverageFilter currentFilter;
    
    float voltageOffset;
    float currentOffset;
    float tempOffset;

public:
    Sensors();
    void init();
    float readTemperature();
    float readVoltage();
    float readCurrent();
    float calculateSOC(float voltage);
    void calibrate();
    
    // Raw reads for testing/troubleshooting
    float readRawTemperature();
    float readRawVoltage();
    float readRawCurrent();
    
    // Fault detection
    bool checkFault();
};

#endif // SENSORS_H