#ifndef TUYASERVICE_H
#define TUYASERVICE_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

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
    mutable SemaphoreHandle_t dataMutex;

    String getAccessToken();
    void setOffline();
};

#endif