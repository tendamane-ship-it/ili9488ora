#ifndef ABOUT_SCREEN_H
#define ABOUT_SCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>

struct AboutScreenData
{
    bool wifiConnected;
    String ipAddress;
    int wifiRssi;
    bool timeSynchronized;

    bool bmeFound;
    bool bmeAvailable;
    float bmeTemperature;
    float bmeHumidity;

    bool lightSensorFound;
    bool lightSensorAvailable;
    float lightLevel;

    bool pirEnabled;
    bool localDisplayEnabled;

    bool weatherReady;
    bool weatherValid;
    float weatherTemperature;

    bool tuyaOnline;
    float tuyaTemperature;
    float tuyaHumidity;
    int tuyaBattery;

    bool weatherDisplayEnabled;
    bool tuyaDisplayEnabled;
    uint32_t freeHeapKb;
};

void drawAboutScreen(TFT_eSPI& display, const AboutScreenData& data);

#endif
