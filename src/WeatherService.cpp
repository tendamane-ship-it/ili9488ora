#include "WeatherService.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "SerialLog.h"

#define WEATHER_LAT  39.8750
#define WEATHER_LON  20.0050

WeatherService::WeatherService()
    : dataMutex(xSemaphoreCreateMutex())
{
    configASSERT(dataMutex != nullptr);

    // Current weather
    data.temperature = 0;
    data.humidity = 0;
    data.pressure = 0;
    data.windSpeed = 0;
    data.windDirection = 0;
    data.windGusts = 0;
    data.precipitation = 0;
    data.rain = 0;
    data.cloudCover = 0;
    data.apparentTemperature = 0;

    data.weatherCode = -1;
    data.isDay = false;
    data.valid = false;

    // Daily forecast
    for (int i = 0; i < FORECAST_DAYS; i++)
    {
        daily[i].date = "";
        daily[i].temperatureMin = 0;
        daily[i].temperatureMax = 0;
        daily[i].precipitationSum = 0;
        daily[i].precipitationProbability = 0;
        daily[i].windSpeedMax = 0;
        daily[i].sunrise = "";
        daily[i].sunset = "";
        daily[i].weatherCode = -1;
    }
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
        "&current="
        "temperature_2m,"
        "relative_humidity_2m,"
        "pressure_msl,"
        "wind_speed_10m,"
        "wind_direction_10m,"
        "wind_gusts_10m,"
        "precipitation,"
        "rain,"
        "cloud_cover,"
        "apparent_temperature,"
        "weather_code,"
        "is_day"
        "&daily="
        "temperature_2m_max,"
        "temperature_2m_min,"
        "precipitation_sum,"
        "precipitation_probability_max,"
        "wind_speed_10m_max,"
        "sunrise,"
        "sunset,"
        "weather_code"
        "&forecast_days=7"
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

    // --------------------------------------------------------
    // CURRENT WEATHER
    // --------------------------------------------------------

    JsonObject current = doc["current"];

    if (current.isNull())
    {
        Serial.println("Weather: current data missing");
        return false;
    }

    WeatherData updatedData{};
    DailyWeather updatedDaily[FORECAST_DAYS];

    updatedData.temperature =
        current["temperature_2m"] | 0.0;

    updatedData.humidity =
        current["relative_humidity_2m"] | 0.0;

    updatedData.pressure =
        current["pressure_msl"] | 0.0;

    updatedData.windSpeed =
        current["wind_speed_10m"] | 0.0;

    updatedData.windDirection =
        current["wind_direction_10m"] | 0.0;

    updatedData.windGusts =
        current["wind_gusts_10m"] | 0.0;

    updatedData.precipitation =
        current["precipitation"] | 0.0;

    updatedData.rain =
        current["rain"] | 0.0;

    updatedData.cloudCover =
        current["cloud_cover"] | 0.0;

    updatedData.apparentTemperature =
        current["apparent_temperature"] | 0.0;

    updatedData.weatherCode =
        current["weather_code"] | -1;

    updatedData.isDay =
        current["is_day"] | false;

    updatedData.valid = true;

    // --------------------------------------------------------
    // DAILY FORECAST
    // --------------------------------------------------------

    JsonObject dailyData = doc["daily"];

    if (dailyData.isNull())
    {
        Serial.println("Weather: daily data missing");
        return false;
    }

    JsonArray dates =
        dailyData["time"].as<JsonArray>();

    JsonArray tempMax =
        dailyData["temperature_2m_max"].as<JsonArray>();

    JsonArray tempMin =
        dailyData["temperature_2m_min"].as<JsonArray>();

    JsonArray precipitationSum =
        dailyData["precipitation_sum"].as<JsonArray>();

    JsonArray precipitationProbability =
        dailyData["precipitation_probability_max"].as<JsonArray>();

    JsonArray windSpeedMax =
        dailyData["wind_speed_10m_max"].as<JsonArray>();

    JsonArray sunrise =
        dailyData["sunrise"].as<JsonArray>();

    JsonArray sunset =
        dailyData["sunset"].as<JsonArray>();

    JsonArray weatherCodes =
        dailyData["weather_code"].as<JsonArray>();

    for (int i = 0; i < FORECAST_DAYS; i++)
    {
        updatedDaily[i].date =
            dates[i] | "";

        updatedDaily[i].temperatureMax =
            tempMax[i] | 0.0;

        updatedDaily[i].temperatureMin =
            tempMin[i] | 0.0;

        updatedDaily[i].precipitationSum =
            precipitationSum[i] | 0.0;

        updatedDaily[i].precipitationProbability =
            precipitationProbability[i] | 0;

        updatedDaily[i].windSpeedMax =
            windSpeedMax[i] | 0.0;

        updatedDaily[i].sunrise =
            sunrise[i] | "";

        updatedDaily[i].sunset =
            sunset[i] | "";

        updatedDaily[i].weatherCode =
            weatherCodes[i] | -1;
    }

    xSemaphoreTake(dataMutex, portMAX_DELAY);
    data = updatedData;
    for (int i = 0; i < FORECAST_DAYS; i++)
        daily[i] = updatedDaily[i];
    xSemaphoreGive(dataMutex);

    // --------------------------------------------------------
    // SERIAL OUTPUT
    // --------------------------------------------------------

    Serial.println("Weather OK");

    Serial.printf(
        "Temperature: %.1f C\n",
        updatedData.temperature
    );

    Serial.printf(
        "Feels like: %.1f C\n",
        updatedData.apparentTemperature
    );

    Serial.printf(
        "Humidity: %.1f %%\n",
        updatedData.humidity
    );

    Serial.printf(
        "Pressure: %.1f hPa\n",
        updatedData.pressure
    );

    Serial.printf(
        "Wind: %.1f km/h\n",
        updatedData.windSpeed
    );

    Serial.printf(
        "Wind direction: %.0f deg\n",
        updatedData.windDirection
    );

    Serial.printf(
        "Wind gusts: %.1f km/h\n",
        updatedData.windGusts
    );

    Serial.printf(
        "Precipitation: %.1f mm\n",
        updatedData.precipitation
    );

    Serial.printf(
        "Rain: %.1f mm\n",
        updatedData.rain
    );

    Serial.printf(
        "Cloud cover: %.0f %%\n",
        updatedData.cloudCover
    );

    Serial.printf(
        "Weather code: %d\n",
        updatedData.weatherCode
    );

    Serial.printf(
        "Day: %s\n",
        updatedData.isDay ? "YES" : "NO"
    );

    Serial.println();
    Serial.println("=== 7 DAY FORECAST ===");

    for (int i = 0; i < FORECAST_DAYS; i++)
    {
        Serial.printf(
            "%s | Min %.1f C | Max %.1f C | Rain %.1f mm | Probability %d %% | Wind %.1f km/h | Code %d\n",
            updatedDaily[i].date.c_str(),
            updatedDaily[i].temperatureMin,
            updatedDaily[i].temperatureMax,
            updatedDaily[i].precipitationSum,
            updatedDaily[i].precipitationProbability,
            updatedDaily[i].windSpeedMax,
            updatedDaily[i].weatherCode
        );

        Serial.printf(
            "Sunrise: %s | Sunset: %s\n",
            updatedDaily[i].sunrise.c_str(),
            updatedDaily[i].sunset.c_str()
        );
    }

    return true;
}

WeatherData WeatherService::getData() const
{
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    WeatherData result = data;
    xSemaphoreGive(dataMutex);
    return result;
}

DailyWeather WeatherService::getDaily(int index) const
{
    if (index < 0)
        index = 0;

    if (index >= FORECAST_DAYS)
        index = FORECAST_DAYS - 1;

    xSemaphoreTake(dataMutex, portMAX_DELAY);
    DailyWeather result = daily[index];
    xSemaphoreGive(dataMutex);
    return result;
}