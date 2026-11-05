#ifndef WEATHER_SERVICE_H
#define WEATHER_SERVICE_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

struct WeatherData
{
    // Current weather
    float temperature;
    float humidity;
    float pressure;
    float windSpeed;
    float windDirection;
    float windGusts;
    float precipitation;
    float rain;
    float cloudCover;
    float apparentTemperature;

    int weatherCode;
    bool isDay;

    bool valid;
};

struct DailyWeather
{
    String date;

    float temperatureMin;
    float temperatureMax;

    float precipitationSum;
    int precipitationProbability;

    float windSpeedMax;

    String sunrise;
    String sunset;

    int weatherCode;
};

class WeatherService
{
public:
    WeatherService();

    bool update();

    WeatherData getData() const;

    DailyWeather getDaily(int index) const;

    static const int FORECAST_DAYS = 7;

private:
    WeatherData data;
    DailyWeather daily[FORECAST_DAYS];
    mutable SemaphoreHandle_t dataMutex;
};

#endif