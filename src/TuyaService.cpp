#include "TuyaService.h"
#include "secrets.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <mbedtls/md.h>
#include <time.h>
#include "SerialLog.h"

// ============================================================
// TUYA SIGNATURE HELPERS
// ============================================================

String sha256Hex(const String& data)
{
    byte hash[32];

    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    mbedtls_md_setup(
        &ctx,
        mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
        0
    );

    mbedtls_md_starts(&ctx);

    mbedtls_md_update(
        &ctx,
        (const unsigned char*)data.c_str(),
        data.length()
    );

    mbedtls_md_finish(&ctx, hash);
    mbedtls_md_free(&ctx);

    String result = "";

    for (int i = 0; i < 32; i++)
    {
        if (hash[i] < 16)
            result += "0";

        result += String(hash[i], HEX);
    }

    result.toLowerCase();

    return result;
}

String hmacSha256Hex(
    const String& message,
    const char* secret
)
{
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
        (const unsigned char*)secret,
        strlen(secret)
    );

    mbedtls_md_hmac_update(
        &ctx,
        (const unsigned char*)message.c_str(),
        message.length()
    );

    mbedtls_md_hmac_finish(&ctx, hash);
    mbedtls_md_free(&ctx);

    String result = "";

    for (int i = 0; i < 32; i++)
    {
        if (hash[i] < 16)
            result += "0";

        result += String(hash[i], HEX);
    }

    result.toUpperCase();

    return result;
}

// ============================================================
// CONSTRUCTOR
// ============================================================

TuyaService::TuyaService()
    : dataMutex(xSemaphoreCreateMutex())
{
    configASSERT(dataMutex != nullptr);
    temperature = 0.0;
    humidity = 0.0;
    battery = 0;
    online = false;
}

// ============================================================
// BEGIN
// ============================================================

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

// ============================================================
// GET ACCESS TOKEN
// ============================================================

String TuyaService::getAccessToken()
{
    HTTPClient http;

    const char* tokenPath =
        "/v1.0/token?grant_type=1";

    String url =
        String("https://") +
        TUYA_API_HOST +
        tokenPath;

    // --------------------------------------------------------
    // Timestamp
    // --------------------------------------------------------

    uint64_t timestampValue =
        (uint64_t)time(nullptr) * 1000ULL;

    String timestamp =
        String(timestampValue);

    // --------------------------------------------------------
    // SHA256 of empty GET body
    // --------------------------------------------------------

    const String contentSha256 =
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

    // --------------------------------------------------------
    // Tuya stringToSign
    // --------------------------------------------------------

    String stringToSign =
        String("GET\n") +
        contentSha256 +
        "\n\n" +
        tokenPath;

    // --------------------------------------------------------
    // Signature
    // client_id + timestamp + stringToSign
    // --------------------------------------------------------

    String signMessage =
        String(TUYA_CLIENT_ID) +
        timestamp +
        stringToSign;

    String sign =
        hmacSha256Hex(
            signMessage,
            TUYA_CLIENT_SECRET
        );

    // --------------------------------------------------------
    // HTTP
    // --------------------------------------------------------

    http.begin(url);

    http.addHeader(
        "client_id",
        TUYA_CLIENT_ID
    );

    http.addHeader(
        "sign",
        sign
    );

    http.addHeader(
        "t",
        timestamp
    );

    http.addHeader(
        "sign_method",
        "HMAC-SHA256"
    );

    Serial.println(
        "Tuya: getting cloud token..."
    );

    int httpCode = http.GET();

    String response =
        http.getString();

    Serial.print(
        "Tuya token HTTP: "
    );
    Serial.println(httpCode);

    Serial.println("Tuya token response: [redacted]");

    http.end();

    if (httpCode != 200)
        return "";

    JsonDocument doc;

    DeserializationError error =
        deserializeJson(
            doc,
            response
        );

    if (error)
    {
        Serial.print(
            "Tuya JSON error: "
        );
        Serial.println(
            error.c_str()
        );

        return "";
    }

    if (!doc["success"])
        return "";

    return doc["result"]["access_token"].as<String>();
}

// ============================================================
// UPDATE DEVICE
// ============================================================

bool TuyaService::update()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        setOffline();
        return false;
    }

    // --------------------------------------------------------
    // Get access token
    // --------------------------------------------------------

    String accessToken =
        getAccessToken();

    if (accessToken.length() == 0)
    {
        Serial.println(
            "Tuya: failed to get access token"
        );

        setOffline();
        return false;
    }

    // --------------------------------------------------------
    // Device status endpoint
    // --------------------------------------------------------

    String statusPath =
        String("/v1.0/iot-03/devices/") +
        TUYA_DEVICE_ID +
        "/status";

    String url =
        String("https://") +
        TUYA_API_HOST +
        statusPath;

    // --------------------------------------------------------
    // Timestamp
    // --------------------------------------------------------

    uint64_t timestampValue =
        (uint64_t)time(nullptr) * 1000ULL;

    String timestamp =
        String(timestampValue);

    // --------------------------------------------------------
    // SHA256 of empty GET body
    // --------------------------------------------------------

    const String contentSha256 =
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

    // --------------------------------------------------------
    // Tuya stringToSign
    // --------------------------------------------------------

    String stringToSign =
        String("GET\n") +
        contentSha256 +
        "\n\n" +
        statusPath;

    // --------------------------------------------------------
    // Signature
    // client_id + access_token + timestamp + stringToSign
    // --------------------------------------------------------

    String signMessage =
        String(TUYA_CLIENT_ID) +
        accessToken +
        timestamp +
        stringToSign;

    String sign =
        hmacSha256Hex(
            signMessage,
            TUYA_CLIENT_SECRET
        );

    // --------------------------------------------------------
    // HTTP
    // --------------------------------------------------------

    HTTPClient http;

    http.begin(url);

    http.addHeader(
        "client_id",
        TUYA_CLIENT_ID
    );

    http.addHeader(
        "access_token",
        accessToken
    );

    http.addHeader(
        "sign",
        sign
    );

    http.addHeader(
        "t",
        timestamp
    );

    http.addHeader(
        "sign_method",
        "HMAC-SHA256"
    );

    Serial.println(
        "Tuya: getting device status..."
    );

    int httpCode =
        http.GET();

    String response =
        http.getString();

    Serial.print(
        "Tuya status HTTP: "
    );
    Serial.println(httpCode);

    Serial.println(
        "Tuya status response:"
    );
    Serial.println(response);

    http.end();

    if (httpCode != 200)
    {
        setOffline();
        return false;
    }

    JsonDocument doc;

    DeserializationError error =
        deserializeJson(
            doc,
            response
        );

    if (error)
    {
        Serial.print(
            "Tuya JSON error: "
        );
        Serial.println(
            error.c_str()
        );

        setOffline();
        return false;
    }

    if (!doc["success"])
    {
        setOffline();
        return false;
    }

    // --------------------------------------------------------
    // Device status
    // --------------------------------------------------------

    xSemaphoreTake(dataMutex, portMAX_DELAY);
    float updatedTemperature = temperature;
    float updatedHumidity = humidity;
    int updatedBattery = battery;
    xSemaphoreGive(dataMutex);

    JsonArray result =
        doc["result"].as<JsonArray>();
for (JsonObject item : result)
{
    const char* code =
        item["code"];

    if (!code)
        continue;

    if (
        strcmp(
            code,
            "temp_current"
        ) == 0
    )
    {
        updatedTemperature =
            item["value"].as<float>() /
            10.0;
    }
    else if (
        strcmp(
            code,
            "humidity_value"
        ) == 0
    )
    {
        updatedHumidity =
            item["value"].as<float>();
    }
    else if (
        strcmp(
            code,
            "battery_state"
        ) == 0
    )
    {
        const char* batteryState =
            item["value"];

        if (batteryState)
        {
            if (
                strcmp(
                    batteryState,
                    "high"
                ) == 0
            )
            {
                updatedBattery = 100;
            }
            else if (
                strcmp(
                    batteryState,
                    "middle"
                ) == 0
            )
            {
                updatedBattery = 50;
            }
            else if (
                strcmp(
                    batteryState,
                    "low"
                ) == 0
            )
            {
                updatedBattery = 20;
            }
        }
    }
}

    xSemaphoreTake(dataMutex, portMAX_DELAY);
    temperature = updatedTemperature;
    humidity = updatedHumidity;
    battery = updatedBattery;
    online = true;
    xSemaphoreGive(dataMutex);

    Serial.println(
        "Tuya: device status OK"
    );

    Serial.print(
        "Tuya temperature: "
    );
    Serial.print(
        updatedTemperature
    );
    Serial.println(" C");

    Serial.print(
        "Tuya humidity: "
    );
    Serial.print(
        updatedHumidity
    );
    Serial.println(" %");

    Serial.print(
        "Tuya battery: "
    );
    Serial.print(
        updatedBattery
    );
    Serial.println(" %");

    return true;
}

// ============================================================
// GETTERS
// ============================================================

float TuyaService::getTemperature()
{
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    float value = temperature;
    xSemaphoreGive(dataMutex);
    return value;
}

float TuyaService::getHumidity()
{
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    float value = humidity;
    xSemaphoreGive(dataMutex);
    return value;
}

int TuyaService::getBattery()
{
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    int value = battery;
    xSemaphoreGive(dataMutex);
    return value;
}

bool TuyaService::isOnline()
{
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    bool value = online;
    xSemaphoreGive(dataMutex);
    return value;
}

void TuyaService::setOffline()
{
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    online = false;
    xSemaphoreGive(dataMutex);
}