#include "TuyaService.h"
#include "secrets.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <mbedtls/md.h>

TuyaService::TuyaService()
{
    temperature = 0.0;
    humidity = 0.0;
    battery = 0;
    online = false;
}

bool TuyaService::begin()
{
    Serial.println();
    Serial.println("=== TUYA SERVICE ===");

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Tuya: WiFi not connected");
        return false;
    }

    Serial.println("Tuya: WiFi connected");

    return update();
}

String TuyaService::getAccessToken()
{
    HTTPClient http;

    String url = String("https://") +
                 TUYA_API_HOST +
                 "/v1.0/token?grant_type=1";

    unsigned long t = millis();

    String timestamp = String((unsigned long)time(nullptr) * 1000ULL);

    String message = String(TUYA_CLIENT_ID) + timestamp;

    byte hash[32];

    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    mbedtls_md_setup(
        &ctx,
        mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
        1
    );

    mbedtls_md_hmac_starts(
        &ctx,
        (const unsigned char*)TUYA_CLIENT_SECRET,
        strlen(TUYA_CLIENT_SECRET)
    );

    mbedtls_md_hmac_update(
        &ctx,
        (const unsigned char*)message.c_str(),
        message.length()
    );

    mbedtls_md_hmac_finish(&ctx, hash);
    mbedtls_md_free(&ctx);

    String sign = "";

    for (int i = 0; i < 32; i++)
    {
        if (hash[i] < 16)
            sign += "0";

        sign += String(hash[i], HEX);
    }

    sign.toUpperCase();

    http.begin(url);

    http.addHeader("client_id", TUYA_CLIENT_ID);
    http.addHeader("sign", sign);
    http.addHeader("t", timestamp);
    http.addHeader("sign_method", "HMAC-SHA256");

    Serial.println("Tuya: getting cloud token...");

    int httpCode = http.GET();

    String response = http.getString();

    Serial.print("Tuya token HTTP: ");
    Serial.println(httpCode);

    Serial.println("Tuya token response:");
    Serial.println(response);

    http.end();

    if (httpCode != 200)
        return "";

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, response);

    if (error)
    {
        Serial.print("Tuya JSON error: ");
        Serial.println(error.c_str());
        return "";
    }

    if (!doc["success"])
        return "";

    return doc["result"]["access_token"].as<String>();
}

bool TuyaService::update()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        online = false;
        return false;
    }

    String accessToken = getAccessToken();

    if (accessToken.length() == 0)
    {
        Serial.println("Tuya: failed to get access token");
        online = false;
        return false;
    }

    HTTPClient http;

    String url = String("https://") +
                 TUYA_API_HOST +
                 "/v1.0/iot-03/devices/" +
                 TUYA_DEVICE_ID +
                 "/status";

    String timestamp = String((unsigned long)time(nullptr) * 1000ULL);

    String message =
        String(TUYA_CLIENT_ID) +
        accessToken +
        timestamp;

    byte hash[32];

    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    mbedtls_md_setup(
        &ctx,
        mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
        1
    );

    mbedtls_md_hmac_starts(
        &ctx,
        (const unsigned char*)TUYA_CLIENT_SECRET,
        strlen(TUYA_CLIENT_SECRET)
    );

    mbedtls_md_hmac_update(
        &ctx,
        (const unsigned char*)message.c_str(),
        message.length()
    );

    mbedtls_md_hmac_finish(&ctx, hash);
    mbedtls_md_free(&ctx);

    String sign = "";

    for (int i = 0; i < 32; i++)
    {
        if (hash[i] < 16)
            sign += "0";

        sign += String(hash[i], HEX);
    }

    sign.toUpperCase();

    http.begin(url);

    http.addHeader("client_id", TUYA_CLIENT_ID);
    http.addHeader("access_token", accessToken);
    http.addHeader("sign", sign);
    http.addHeader("t", timestamp);
    http.addHeader("sign_method", "HMAC-SHA256");

    Serial.println("Tuya: getting device status...");

    int httpCode = http.GET();

    String response = http.getString();

    Serial.print("Tuya status HTTP: ");
    Serial.println(httpCode);

    Serial.println("Tuya status response:");
    Serial.println(response);

    http.end();

    if (httpCode != 200)
    {
        online = false;
        return false;
    }

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, response);

    if (error)
    {
        Serial.print("Tuya JSON error: ");
        Serial.println(error.c_str());
        online = false;
        return false;
    }

    if (!doc["success"])
    {
        online = false;
        return false;
    }

    JsonArray result = doc["result"].as<JsonArray>();

    for (JsonObject item : result)
    {
        const char* code = item["code"];
        
        if (!code)
            continue;

        if (strcmp(code, "va_temperature") == 0)
        {
            temperature = item["value"].as<float>() / 10.0;
        }
        else if (strcmp(code, "va_humidity") == 0)
        {
            humidity = item["value"].as<float>() / 10.0;
        }
        else if (strcmp(code, "battery_percentage") == 0)
        {
            battery = item["value"].as<int>();
        }
    }

    online = true;

    Serial.println("Tuya: device status OK");

    Serial.print("Tuya temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Tuya humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Tuya battery: ");
    Serial.print(battery);
    Serial.println(" %");

    return true;
}

float TuyaService::getTemperature()
{
    return temperature;
}

float TuyaService::getHumidity()
{
    return humidity;
}

int TuyaService::getBattery()
{
    return battery;
}

bool TuyaService::isOnline()
{
    return online;
}