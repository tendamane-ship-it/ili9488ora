#include "WeatherService.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "SerialLog.h"
#include <time.h>

#define WEATHER_LAT  39.8750
#define WEATHER_LON  20.0050

namespace
{
constexpr time_t MINIMUM_VALID_EPOCH = 1577836800;

bool isKnownWeatherCode(int code)
{
    return (code >= 0 && code <= 3) || code == 45 || code == 48 ||
        (code >= 51 && code <= 57) || (code >= 61 && code <= 67) ||
        (code >= 71 && code <= 77) || (code >= 80 && code <= 82) ||
        (code >= 85 && code <= 86) || (code >= 95 && code <= 99);
}

bool parseIsDay(JsonVariantConst value, bool& isDay)
{
    if (value.is<bool>())
    {
        isDay = value.as<bool>();
        return true;
    }

    if (value.is<int>())
    {
        const int valueAsInt = value.as<int>();
        if (valueAsInt == 0 || valueAsInt == 1)
        {
            isDay = valueAsInt == 1;
            return true;
        }
    }

    return false;
}

bool parseLocalMinutes(const String& isoTime, int& minutes)
{
    const int separator = isoTime.indexOf('T');
    if (separator < 0 || isoTime.length() <= separator + 5)
        return false;

    int hour = -1;
    int minute = -1;
    if (sscanf(isoTime.c_str() + separator + 1, "%2d:%2d", &hour, &minute) != 2 ||
        hour < 0 || hour > 23 || minute < 0 || minute > 59)
        return false;

    minutes = hour * 60 + minute;
    return true;
}

bool localDayFromSunriseSunset(
    const String& responseDate,
    const String& sunrise,
    const String& sunset,
    bool& isDay
)
{
    const time_t now = time(nullptr);
    if (now < MINIMUM_VALID_EPOCH)
        return false;

    struct tm localTime{};
    if (localtime_r(&now, &localTime) == nullptr)
        return false;

    char localDate[11];
    if (strftime(localDate, sizeof(localDate), "%Y-%m-%d", &localTime) == 0 ||
        responseDate != localDate)
        return false;

    int sunriseMinutes = 0;
    int sunsetMinutes = 0;
    if (!parseLocalMinutes(sunrise, sunriseMinutes) ||
        !parseLocalMinutes(sunset, sunsetMinutes))
        return false;

    const int currentMinutes = localTime.tm_hour * 60 + localTime.tm_min;
    isDay = currentMinutes >= sunriseMinutes && currentMinutes < sunsetMinutes;
    return true;
}
}

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

    const JsonVariantConst weatherCodeField = current["weather_code"];
    if (!weatherCodeField.is<int>() ||
        !isKnownWeatherCode(weatherCodeField.as<int>()))
    {
        Serial.println("Weather: current weather_code is missing or invalid");
        return false;
    }
    updatedData.weatherCode = weatherCodeField.as<int>();

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

    bool apiIsDay = false;
    const bool apiIsDayValid = parseIsDay(current["is_day"], apiIsDay);
    bool localIsDay = false;
    const String today = dates[0] | "";
    const String sunriseToday = sunrise[0] | "";
    const String sunsetToday = sunset[0] | "";
    if (localDayFromSunriseSunset(today, sunriseToday, sunsetToday, localIsDay))
    {
        updatedData.isDay = localIsDay;
        Serial.println("Weather day/night: local time compared with sunrise/sunset");
    }
    else if (apiIsDayValid)
    {
        updatedData.isDay = apiIsDay;
        Serial.println("Weather day/night: Open-Meteo is_day fallback");
    }
    else
    {
        Serial.println("Weather: is_day and local sunrise/sunset are unavailable");
        return false;
    }
    updatedData.valid = true;

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
