#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

static constexpr char PROJECT_NAMESPACE[] = "ili9488ora";

extern Preferences preferences;

// Cilësimet e përgjithshme
extern bool setting24Hour;
extern bool settingDate;
extern bool settingWeather;
extern bool settingLightSensor;
extern bool settingTuyaDisplay;
extern bool settingPIR;

// Intervalet e përditësimit të ekranit
extern unsigned long localSensorsDisplayTime;
extern unsigned long weatherDisplayTime;
extern unsigned long tuyaDisplayTime;

extern bool settingScreenSaver;
extern unsigned long screenSaverMinutes;

extern bool settingGreeting;

extern bool settingNightMode;
extern float nightSleepLux;
extern float nightWakeLux;

extern bool settingBacklightAuto;
extern uint8_t settingBacklightBrightness;


// Ngarko cilësimet e ruajtura nga memoria Preferences.
void loadSettings();

#endif