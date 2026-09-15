#include "WeatherService.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define WEATHER_LAT  39.8750
#define WEATHER_LON  20.0050

WeatherService::WeatherService()
{
    data.temperature = 0;
    data.humidity = 0;
    data.pressure = 0;
    data.windSpeed = 0;
    data.weatherCode = -1;
    data.valid = false;
}

bool WeatherService::update()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Weather: WiFi not connected");
        return false;
    }

    HTTPClient http;

    String url =
        "https://api.open-meteo.com/v1/forecast"
        "?latitude=39.8750"
        "&longitude=20.0050"
        "&current=temperature_2m,relative_humidity_2m,pressure_msl,wind_speed_10m,weather_code"
        "&timezone=Europe%2FTirane";

    Serial.println("Weather: requesting Open-Meteo...");

    http.begin(url);

    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.print("Weather HTTP error: ");
        Serial.println(httpCode);

        http.end();
        return false;
    }

    String payload = http.getString();

    http.end();

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Serial.print("Weather JSON error: ");
        Serial.println(error.c_str());
        return false;
    }

    JsonObject current = doc["current"];

    if (current.isNull())
    {
        Serial.println("Weather: current data missing");
        return false;
    }

    data.temperature = current["temperature_2m"] | 0.0;
    data.humidity = current["relative_humidity_2m"] | 0.0;
    data.pressure = current["pressure_msl"] | 0.0;
    data.windSpeed = current["wind_speed_10m"] | 0.0;
    data.weatherCode = current["weather_code"] | -1;

    data.valid = true;

    Serial.println("Weather OK");
    Serial.printf("Temperature: %.1f C\n", data.temperature);
    Serial.printf("Humidity: %.1f %%\n", data.humidity);
    Serial.printf("Pressure: %.1f hPa\n", data.pressure);
    Serial.printf("Wind: %.1f km/h\n", data.windSpeed);
    Serial.printf("Weather code: %d\n", data.weatherCode);

    return true;
}

const WeatherData& WeatherService::getData() const
{
    return data;
}