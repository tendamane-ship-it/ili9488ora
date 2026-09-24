#ifndef TUYASERVICE_H
#define TUYASERVICE_H

#include <Arduino.h>

class TuyaService
{
public:
    TuyaService();

    bool begin();
    bool update();

    float getTemperature();
    float getHumidity();
    int getBattery();

    bool isOnline();

private:
    float temperature;
    float humidity;
    int battery;
    bool online;

    String getAccessToken();
};

#endif