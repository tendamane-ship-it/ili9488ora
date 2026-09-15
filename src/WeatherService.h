#ifndef WEATHER_SERVICE_H
#define WEATHER_SERVICE_H

#include <Arduino.h>

struct WeatherData
{
    float temperature;
    float humidity;
    float pressure;
    float windSpeed;
    int weatherCode;
    bool valid;
};

class WeatherService
{
public:
    WeatherService();

    bool update();
    const WeatherData& getData() const;
   

private:
    WeatherData data;
};

#endif