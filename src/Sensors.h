#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <bsec2.h>
#include <BH1750.h>

// ------------------------------------------------------------
// SENSOR OBJECTS
// ------------------------------------------------------------

extern bool bme680OK;
extern bool bmeReadingOK;
extern unsigned long lastBmeReading;

extern BH1750 lightMeter;
extern bool bh1750OK;
extern bool bh1750ReadingOK;
extern unsigned long lastLuxReading;

// ------------------------------------------------------------
// SENSOR VALUES
// ------------------------------------------------------------

extern float bmeTemp;
extern float bmeHumidity;
extern float bmePressure;
extern float bmeGas;
extern float bh1750Lux;

// ------------------------------------------------------------
// AIR QUALITY
// ------------------------------------------------------------

extern float bmeIaq;
extern bool bmeIaqReady;
extern uint8_t bmeIaqAccuracy;
extern int8_t bmeIaqStabilizationStatus;
extern int8_t bmeIaqRunInStatus;

const char* airQualityCategory();
const char* bmeIaqAccuracyLabel();

// ------------------------------------------------------------
// SENSOR TIMING
// ------------------------------------------------------------

extern unsigned long lastSensorRead;
extern const unsigned long SENSOR_READ_INTERVAL;

// ------------------------------------------------------------
// FUNCTIONS
// ------------------------------------------------------------

void initSensors();
void updateLocalSensors();
void runBsec();
// ------------------------------------------------------------
// PIR
// ------------------------------------------------------------

#define PIR_PIN 42

extern bool pirLastState;

extern unsigned long pirIgnoreUntil;

extern int pirStableState;
extern int pirCandidateState;

extern unsigned long pirStateChangeTime;

extern const unsigned long PIR_DEBOUNCE_TIME;

void initPIR();
int readPIR();

#endif