#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME680.h>
#include <BH1750.h>

// ------------------------------------------------------------
// SENSOR OBJECTS
// ------------------------------------------------------------

extern Adafruit_BME680 bme;
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

extern float airBaseline;
extern bool airBaselineReady;

extern float airBaselineSum;
extern int airBaselineCount;

extern const int AIR_BASELINE_SAMPLES;

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
void updateAirQuality();
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