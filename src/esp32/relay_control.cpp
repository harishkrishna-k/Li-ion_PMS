/**
 * @file relay_control.cpp
 * @brief Relay and PID fan control
 * @author Project Contributor
 * @license MIT
 */

#include "relay_control.h"

// Pin definitions
const int RELAY_PIN = 27;
const int FAN_PIN = 26;

// Temperature thresholds
const float TEMP_THRESHOLD_COOLING_CHARGING = 40.0;
const float TEMP_THRESHOLD_COOLING_DISCHARGING = 50.0;
const float TEMP_CRITICAL = 60.0;

// PID parameters
float Kp = 2.0;
float Ki = 0.5;
float Kd = 1.0;

// PID variables
float targetTemp = 35.0;
float lastError = 0;
float sumError = 0;
unsigned long lastPidTime = 0;

// State
bool _relayState = false;
bool _fanState = false;
int _fanPwm = 0;

// Initialize relay and fan control
void relayInit() {
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);

    // Initialize pins to OFF
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(FAN_PIN, LOW);

    // Initialize PWM
    ledcSetup(0, 25000, 8);  // Channel 0, 25kHz, 8-bit
    ledcAttachPin(FAN_PIN, 0);
    ledcWrite(0, 0);

    lastPidTime = millis();
    Serial.println("Relay and fan control initialized with PID");
}

// Set relay state
void setRelay(bool enable) {
    digitalWrite(RELAY_PIN, enable ? HIGH : LOW);
    _relayState = enable;
}

// Get relay state
bool getRelay() {
    return _relayState;
}

// PID control for fan speed
void controlFanPID(float temperature) {
    unsigned long now = millis();
    float dt = (now - lastPidTime) / 1000.0;
    lastPidTime = now;

    // Calculate error
    float error = temperature - targetTemp;

    // Proportional term
    float pTerm = Kp * error;

    // Integral term (with anti-windup)
    sumError += error * dt;
    sumError = constrain(sumError, -100, 100);
    float iTerm = Ki * sumError;

    // Derivative term
    float dTerm = Kd * (error - lastError) / dt;
    lastError = error;

    // Calculate output
    float output = pTerm + iTerm + dTerm;
    _fanPwm = constrain((int)output, 0, 255);

    // Apply PWM to fan
    ledcWrite(0, _fanPwm);
    _fanPwm = _fanPwm > 0;
}

// Simple ON/OFF fan control (for fallback)
void controlFanSimple(float temperature, bool isCharging) {
    bool shouldFanBeOn;

    if (isCharging) {
        shouldFanBeOn = (temperature > TEMP_THRESHOLD_COOLING_CHARGING);
    } else {
        shouldFanBeOn = (temperature > TEMP_THRESHOLD_COOLING_DISCHARGING);
    }

    digitalWrite(FAN_PIN, shouldFanBeOn ? HIGH : LOW);
    _fanState = shouldFanBeOn;
}

// Combined fan control with PID and safety
void controlFan(float temperature, bool isCharging, bool usePid = false) {
    // Critical temperature - force fan on maximum
    if (temperature > TEMP_CRITICAL) {
        ledcWrite(0, 255);
        _fanPwm = 255;
        _fanState = true;
        return;
    }

    if (usePid) {
        controlFanPID(temperature);
    } else {
        controlFanSimple(temperature, isCharging);
    }
}

// Get fan state
bool getFan() {
    return _fanState;
}

// Get fan PWM value
int getFanPwm() {
    return _fanPwm;
}

// Turn off fan
void fanOff() {
    ledcWrite(0, 0);
    _fanPwm = 0;
    _fanState = false;
}

// Turn on fan at PWM
void fanOn(int pwm) {
    ledcWrite(0, constrain(pwm, 0, 255));
    _fanPwm = pwm;
    _fanState = pwm > 0;
}

// Set PID parameters
void setPidGains(float kp, float ki, float kd) {
    Kp = kp;
    Ki = ki;
    Kd = kd;
}

// Set target temperature
void setTargetTemp(float temp) {
    targetTemp = temp;
}