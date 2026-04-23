/**
 * @file database.h
 * @brief Header for Supabase database client
 * @author Project Contributor
 * @license MIT
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Function declarations
void databaseInit(const char* url, const char* key);
int databaseSendReading(float voltage, float current, float temperature, const char* batteryStatus);
bool databaseConnected();
unsigned long getLastPostTime();
void setDatabaseConnected(bool status);
int getRetryCount();

#endif // DATABASE_H