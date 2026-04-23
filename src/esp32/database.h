/**
 * @file database.h
 * @brief Header for Supabase database client
 * @author Project Contributor
 * @license MIT
 #ifndef DATABASE_H
 #define DATABASE_H

 #include <Arduino.h>
 #include <WiFi.h>
 #include <HTTPClient.h>
 #include <WiFiClientSecure.h>

 // Struct for passing data to the background task
 struct DBReading {
     float voltage;
     float current;
     float temperature;
     char status[16];
 };

 // Function declarations
 void databaseInit(const char* url, const char* key);
 void databaseStartTask(); // New: Start the background worker
 int databaseSendReading(float voltage, float current, float temperature, const char* batteryStatus);

 bool databaseConnected();
 unsigned long getLastPostTime();
 void setDatabaseConnected(bool status);
 int getRetryCount();

 #endif // DATABASE_H