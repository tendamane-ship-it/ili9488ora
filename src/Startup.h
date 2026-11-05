
#ifndef STARTUP_H
#define STARTUP_H

#include <Arduino.h>
#include <TFT_eSPI.h>

enum StartupItem
{
    STARTUP_WIFI,
    STARTUP_BME680,
    STARTUP_BH1750,
    STARTUP_NTP,
    STARTUP_WEATHER,
    STARTUP_TUYA,
    STARTUP_ITEM_COUNT
};

enum StartupStatus
{
    STARTUP_WAITING,
    STARTUP_CHECKING,
    STARTUP_OK,
    STARTUP_ERROR,
    STARTUP_PORTAL,
    STARTUP_SKIPPED,
    STARTUP_BACKGROUND
};

void runStartup(TFT_eSPI* display);
void setStartupStatus(StartupItem item, StartupStatus status);
void refreshStartup();
bool isStartupComplete();

#endif
