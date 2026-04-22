/**
 * @file relay_control.h
 * @brief Header file for relay and fan control
 * @author Project Contributor
 * @license MIT
 */

#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

#include <Arduino.h>

class RelayControl {
public:
    RelayControl();
    void init();
    void setRelay(bool enable);
    bool getRelay();
    void controlFan(float temperature, bool isCharging);
    bool getFan();
    void fanOff();
    void fanOn();

private:
    bool _relayState;
    bool _fanState;
};

#endif // RELAY_CONTROL_H