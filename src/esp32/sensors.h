/**
 * @file sensors.h
 * @brief Header file for sensor functions
 * @author Project Contributor
 * @license MIT
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

class Sensors {
public:
    Sensors();
    void init();
    float readTemperature();
    float readVoltage();
    float readCurrent();
    float getLastVoltage();
    float getLastCurrent();
    float getLastTemperature();
    float calculateSOC(float voltage);

private:
    float _lastVoltage;
    float _lastCurrent;
    float _lastTemperature;
};

#endif // SENSORS_H