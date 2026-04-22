/**
 * @file database.h
 * @brief Header file for Supabase database client
 * @author Project Contributor
 * @license MIT
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

class Database {
public:
    Database();
    void init(const char* url, const char* key);
    int sendReading(float voltage, float current, float temperature, const char* batteryStatus);
    bool isConnected();
    unsigned long getLastPostTime();
    void setConnected(bool connected);

private:
    bool _connected;
    unsigned long _lastPostTime;
};

#endif // DATABASE_H