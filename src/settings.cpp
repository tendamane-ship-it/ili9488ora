#include "settings.h"

// Preferencat
Preferences preferences;

// Cilësimet e përgjithshme
bool setting24Hour = true;
bool settingDate = true;
bool settingWeather = true;
bool settingLightSensor = true;
bool settingTuyaDisplay = true;
bool settingPIR = true;
bool settingGreeting = true;

// Intervalet e përditësimit të ekranit
unsigned long localSensorsDisplayTime = 10000UL;
unsigned long weatherDisplayTime = 10000UL;
unsigned long tuyaDisplayTime = 10000UL;

// Cilësimet e ruajtësit të ekranit
bool settingScreenSaver = true;
unsigned long screenSaverMinutes = 15UL;
// Cilësimet e modalitetit të natës
bool settingNightMode = false;
float nightSleepLux = 5.0f;
float nightWakeLux = 20.0f;
bool settingBacklightAuto = true;
uint8_t settingBacklightBrightness = 255;

void loadSettings()
{
    preferences.begin(PROJECT_NAMESPACE, false);

    setting24Hour = preferences.getBool("24hour", true);
    settingDate = preferences.getBool("date", true);
    settingWeather = preferences.getBool("weather", true);
    settingLightSensor = preferences.getBool("light", true);
    settingTuyaDisplay = preferences.getBool("tuyaDisplay", true);
    settingPIR = preferences.getBool("pir", true);

    localSensorsDisplayTime =
        preferences.getULong("localTime", 10000UL);

    weatherDisplayTime =
        preferences.getULong("weatherTime", 10000UL);

    tuyaDisplayTime =
        preferences.getULong("tuyaTime", 10000UL);

    settingScreenSaver =
        preferences.getBool("screensaver", true);

    screenSaverMinutes =
        preferences.getULong("screenSaverMin", 15UL);
        settingGreeting =
    preferences.getBool("greeting", true);

    // Mbaje kohën e pritjes së ruajtësit të ekranit brenda kufijve të lejuar.
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

    settingBacklightAuto =
        preferences.getBool("backlightAuto", true);
    settingBacklightBrightness =
        static_cast<uint8_t>(preferences.getUChar("backlightLevel", 255));

    if (nightSleepLux < 0.0f)
        nightSleepLux = 0.0f;

    if (nightWakeLux <= nightSleepLux)
        nightWakeLux = nightSleepLux + 1.0f;
}