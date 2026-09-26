#include "Settings.h"

// Preferences
Preferences preferences;

// General settings
bool setting24Hour = true;
bool settingDate = true;
bool settingWeather = true;
bool settingLightSensor = true;
bool settingPIR = true;

// Display timing settings
unsigned long localSensorsDisplayTime = 10000UL;
unsigned long weatherDisplayTime = 10000UL;

// Screensaver settings
bool settingScreenSaver = true;
unsigned long screenSaverMinutes = 15UL;
// Night Mode settings
bool settingNightMode = false;
float nightSleepLux = 5.0f;
float nightWakeLux = 20.0f;

void loadSettings()
{
    preferences.begin("ora", false);

    setting24Hour = preferences.getBool("24hour", true);
    settingDate = preferences.getBool("date", true);
    settingWeather = preferences.getBool("weather", true);
    settingLightSensor = preferences.getBool("light", true);
    settingPIR = preferences.getBool("pir", true);

    localSensorsDisplayTime =
        preferences.getULong("localTime", 10000UL);

    weatherDisplayTime =
        preferences.getULong("weatherTime", 10000UL);

    settingScreenSaver =
        preferences.getBool("screensaver", true);

    screenSaverMinutes =
        preferences.getULong("screenSaverMin", 15UL);

    // Keep screensaver delay within allowed range
    if (screenSaverMinutes < 1UL)
        screenSaverMinutes = 1UL;

    if (screenSaverMinutes > 60UL)
        screenSaverMinutes = 60UL;
            settingNightMode =
        preferences.getBool("nightMode", false);

    nightSleepLux =
        preferences.getFloat("nightSleepLux", 5.0f);

    nightWakeLux =
        preferences.getFloat("nightWakeLux", 20.0f);

    if (nightSleepLux < 0.0f)
        nightSleepLux = 0.0f;

    if (nightWakeLux <= nightSleepLux)
        nightWakeLux = nightSleepLux + 1.0f;
}