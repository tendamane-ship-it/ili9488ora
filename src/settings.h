#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

static constexpr char PROJECT_NAMESPACE[] = "ili9488ora";

extern Preferences preferences;

// General settings
extern bool setting24Hour;
extern bool settingDate;
extern bool settingWeather;
extern bool settingLightSensor;
extern bool settingTuyaDisplay;
extern bool settingPIR;

// Display timing settings
extern unsigned long localSensorsDisplayTime;
extern unsigned long weatherDisplayTime;
extern unsigned long tuyaDisplayTime;

extern bool settingScreenSaver;
extern unsigned long screenSaverMinutes;

extern bool settingGreeting;

extern bool settingNightMode;
extern float nightSleepLux;
extern float nightWakeLux;


// Load saved settings from Preferences
void loadSettings();

#endif