#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

// Preferences
extern Preferences preferences;

// General settings
extern bool setting24Hour;
extern bool settingDate;
extern bool settingWeather;
extern bool settingLightSensor;
extern bool settingPIR;

// Display timing settings
extern unsigned long localSensorsDisplayTime;
extern unsigned long weatherDisplayTime;

// Screensaver settings
extern bool settingScreenSaver;
extern unsigned long screenSaverMinutes;

// Load saved settings from Preferences
void loadSettings();

#endif