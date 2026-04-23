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
// Kalman Filter Class (Simple 1D)
// ============================================================================

class KalmanFilter {
private:
    float q; // Process noise covariance
    float r; // Measurement noise covariance
    float x; // Value
    float p; // Estimation error covariance
    float k; // Kalman gain

public:
    KalmanFilter(float processNoise = 0.01, float measurementNoise = 0.1, float initialValue = 0)
        : q(processNoise), r(measurementNoise), x(initialValue), p(1.0), k(0) {}

    float update(float measurement) {
        // Prediction update
        p = p + q;

        // Measurement update
        k = p / (p + r);
        x = x + k * (measurement - x);
        p = (1 - k) * p;

        return x;
    }

    float getValue() const { return x; }
};

// ============================================================================
// Sensors Class
// ============================================================================

class Sensors {
private:
    MovingAverageFilter tempFilter;
    KalmanFilter voltageFilter;
    KalmanFilter currentFilter;
    
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