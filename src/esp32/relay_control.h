/**
 * @file relay_control.h
 * @brief Header for relay and fan control
 * @author Project Contributor
 * @license MIT
 */

#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

#include <Arduino.h>

// Function declarations
void relayInit();
void setRelay(bool enable);
bool getRelay();
void controlFan(float temperature, bool isCharging, bool usePid = false);
void controlFanSimple(float temperature, bool isCharging);
void controlFanPID(float temperature);
bool getFan();
int getFanPwm();
void fanOff();
void fanOn(int pwm);

// PID configuration
void setPidGains(float kp, float ki, float kd);
void setTargetTemp(float temp);

#endif // RELAY_CONTROL_H