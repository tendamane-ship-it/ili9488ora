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

extern BH1750 lightMeter;
extern bool bh1750OK;

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

#endif