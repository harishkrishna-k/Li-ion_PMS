/**
 * @file sensors.h
 * @brief Header for sensor functions with signal processing
 * @author Project Contributor
 * @license MIT
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Function declarations
void sensorsInit();
float readTemperature();
float readCurrent();
float readVoltage();
float calculateSOC(float voltage);
void calibrateSensors();

// Raw read functions (for testing)
float readRawTemperature();
float readRawCurrent();
float readRawVoltage();

// Fault detection
bool checkSensorFault();

#endif // SENSORS_H