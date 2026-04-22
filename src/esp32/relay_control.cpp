/**
 * @file relay_control.cpp
 * @brief Relay and cooling fan control
 * @author Project Contributor
 * @license MIT
 */

#include "relay_control.h"

const int RELAY_PIN = 27;     // GPIO 27 for relay
const int FAN_PIN = 26;       // GPIO 26 for BLDC fan

const float TEMP_THRESHOLD_COOLING_CHARGING = 40.0;  // Fan on during charging
const float TEMP_THRESHOLD_COOLING_DISCHARGING = 50.0; // Fan on during discharging

RelayControl::RelayControl() {
    _relayState = false;
    _fanState = false;
}

void RelayControl::init() {
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);

    // Initialize pins to OFF
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(FAN_PIN, LOW);

    _relayState = false;
    _fanState = false;
}

/**
 * @brief Control relay based on battery state
 * @param enable Enable or disable relay
 */
void RelayControl::setRelay(bool enable) {
    digitalWrite(RELAY_PIN, enable ? HIGH : LOW);
    _relayState = enable;
}

/**
 * @brief Get relay state
 * @return Current relay state
 */
bool RelayControl::getRelay() {
    return _relayState;
}

/**
 * @brief Control cooling fan based on temperature
 * @param temperature Current temperature
 * @param isCharging Whether battery is charging
 */
void RelayControl::controlFan(float temperature, bool isCharging) {
    bool shouldFanBeOn;

    if (isCharging) {
        // During charging, activate fan at lower threshold
        shouldFanBeOn = (temperature > TEMP_THRESHOLD_COOLING_CHARGING);
    } else {
        // During discharging, activate fan at higher threshold
        shouldFanBeOn = (temperature > TEMP_THRESHOLD_COOLING_DISCHARGING);
    }

    digitalWrite(FAN_PIN, shouldFanBeOn ? HIGH : LOW);
    _fanState = shouldFanBeOn;
}

/**
 * @brief Get fan state
 * @return Current fan state
 */
bool RelayControl::getFan() {
    return _fanState;
}

/**
 * @brief Turn off fan manually
 */
void RelayControl::fanOff() {
    digitalWrite(FAN_PIN, LOW);
    _fanState = false;
}

/**
 * @brief Turn on fan manually
 */
void RelayControl::fanOn() {
    digitalWrite(FAN_PIN, HIGH);
    _fanState = true;
}