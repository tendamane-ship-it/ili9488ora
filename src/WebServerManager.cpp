#include "WebServerManager.h"
#include <WiFi.h>
#include <time.h>
#include "Pages.h"
#include <Preferences.h>

extern Preferences preferences;

extern bool setting24Hour;
extern bool settingDate;
extern bool settingWeather;
extern bool settingLightSensor;
extern bool settingPIR;
extern unsigned long localSensorsDisplayTime;
extern unsigned long weatherDisplayTime;
extern float bh1750Lux;
extern bool bh1750OK;
extern int pirState;

extern float bmeTemp;
extern float bmeHumidity;
extern float bmeGas;
extern bool bme680OK;
WebServerManager::WebServerManager()
    : server(80),
      weather(nullptr)
{
}
void WebServerManager::setWeatherService(WeatherService* service)
{
    weather = service;
}
void WebServerManager::begin()
{
    server.on("/", [this]()
    {
        handleRoot();
    });

    server.on("/settings", [this]()
    {
        handleSettings();
    });

    server.on("/settings/save", HTTP_POST, [this]()
    {
        handleSettingsSave();
    });

    server.on("/restart", HTTP_POST, [this]()
    {
        server.send(200, "text/plain", "ESP32 restarting...");
        delay(300);
        ESP.restart();
    });

    server.on("/api/status", [this]()
    {
        handleApiStatus();
    });

    server.on("/api", [this]()
    {
        handleApiPage();
    });

    server.begin();

    Serial.println("WebServer started");
}
void WebServerManager::handleClient()
{
    server.handleClient();
}

void WebServerManager::handleRoot()
{
    server.send(200, "text/html", getDashboardPage());
}
void WebServerManager::handleSettings()
{
    server.send(200, "text/html", getSettingsPage());
}
void WebServerManager::handleApiStatus()
{
    String json;

    json += "{";

    // ============================================================
    // PROJECT
    // ============================================================

    json += "\"project\":\"ILI9488 CLOCK\",";

    // ============================================================
    // TIME
    // ============================================================

    struct tm timeinfo;

    if (getLocalTime(&timeinfo, 100))
    {
        char timeText[12];
        char dateText[32];

        strftime(timeText, sizeof(timeText), "%H:%M:%S", &timeinfo);
        strftime(dateText, sizeof(dateText), "%A, %d %B %Y", &timeinfo);

        json += "\"time\":\"";
        json += timeText;
        json += "\",";

        json += "\"date\":\"";
        json += dateText;
        json += "\",";
    }
    else
    {
        json += "\"time\":\"--:--:--\",";
        json += "\"date\":\"--\",";
    }

    // ============================================================
    // WIFI
    // ============================================================

    json += "\"wifi\":\"";
    json += WiFi.SSID();
    json += "\",";

    json += "\"wifiConnected\":";
    json += (WiFi.status() == WL_CONNECTED ? "true" : "false");
    json += ",";

    json += "\"ip\":\"";
    json += WiFi.localIP().toString();
    json += "\",";

    json += "\"rssi\":";
    json += String(WiFi.RSSI());
    json += ",";

    // ============================================================
    // SYSTEM
    // ============================================================

    json += "\"cpu\":";
    json += String(getCpuFrequencyMhz());
    json += ",";

    json += "\"heap\":";
    json += String(ESP.getFreeHeap());
    json += ",";

    json += "\"flash\":";
    json += String(ESP.getFlashChipSize());
    json += ",";

    json += "\"uptime\":";
    json += String(millis() / 1000);
    json += ",";

    json += "\"webServer\":true,";

    // ============================================================
    // ORA SETTINGS
    // ============================================================

    json += "\"setting24Hour\":";
    json += (setting24Hour ? "true" : "false");
    json += ",";

    json += "\"settingDate\":";
    json += (settingDate ? "true" : "false");
    json += ",";

    json += "\"settingWeather\":";
    json += (settingWeather ? "true" : "false");
    json += ",";

    json += "\"settingLightSensor\":";
    json += (settingLightSensor ? "true" : "false");
    json += ",";

    json += "\"settingPIR\":";
    json += (settingPIR ? "true" : "false");
    json += ",";

    json += "\"weatherDisplayTime\":";
    json += String(weatherDisplayTime / 1000);
    json += ",";

    json += "\"localSensorsDisplayTime\":";
    json += String(localSensorsDisplayTime / 1000);
// ============================================================
// LOCAL SENSORS
// ============================================================

json += ",";

// BME680 Temperature
json += "\"bmeTemperature\":";
if (bme680OK)
    json += String(bmeTemp, 1);
else
    json += "null";

json += ",";

// BME680 Humidity
json += "\"bmeHumidity\":";
if (bme680OK)
    json += String(bmeHumidity, 1);
else
    json += "null";

json += ",";

// BME680 Gas
json += "\"bmeGas\":";
if (bme680OK)
    json += String(bmeGas, 0);
else
    json += "null";

json += ",";

// BME680 status
json += "\"bme680OK\":";
json += bme680OK ? "true" : "false";

json += ",";

// Lux
json += "\"lux\":";
if (bh1750OK)
    json += String(bh1750Lux, 0);
else
    json += "null";

json += ",";

// Lux status
json += "\"luxValid\":";
json += bh1750OK ? "true" : "false";

json += ",";

// PIR
json += "\"pirState\":";
json += String(pirState);

json += ",";

// PIR motion
json += "\"pirMotion\":";
json += (pirState == HIGH ? "true" : "false");

    // ============================================================
    // WEATHER
    // ============================================================

    if (weather != nullptr)
    {
        const WeatherData& data = weather->getData();

        json += ",";

        json += "\"temperature\":";
        json += String(data.temperature, 1);
        json += ",";

        json += "\"humidity\":";
        json += String(data.humidity, 1);
        json += ",";

        json += "\"pressure\":";
        json += String(data.pressure, 1);
        json += ",";

        json += "\"windSpeed\":";
        json += String(data.windSpeed, 1);
        json += ",";

        json += "\"weatherCode\":";
        json += String(data.weatherCode);
        json += ",";

        json += "\"weatherValid\":";
        json += data.valid ? "true" : "false";
    }

    json += "}";

    server.send(200, "application/json", json);
}
void WebServerManager::handleApiPage()
{
    String html;

    html += "<!DOCTYPE html>";
    html += "<html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>API Status</title>";

    html += "<style>";
    html += "body{font-family:Arial,sans-serif;background:#111;color:#eee;padding:20px;}";
    html += "h1{margin-bottom:10px;}";
    html += "button{background:#333;color:#fff;border:1px solid #555;padding:10px 16px;border-radius:8px;cursor:pointer;margin-right:8px;}";
    html += "pre{background:#1b1b1b;padding:20px;border-radius:10px;overflow:auto;margin-top:20px;}";
    html += "</style>";

    html += "</head><body>";

    html += "<h1>API Status</h1>";

    html += "<button onclick=\"location.href='/'\">← Dashboard</button>";
    html += "<button onclick='loadAPI()'>Refresh</button>";

    html += "<pre id='json'>Loading...</pre>";

    html += "<script>";
    html += "async function loadAPI(){";
    html += "try{";
    html += "const response=await fetch('/api/status');";
    html += "const data=await response.json();";
    html += "document.getElementById('json').textContent=JSON.stringify(data,null,2);";
    html += "}catch(e){";
    html += "document.getElementById('json').textContent='API error: '+e;";
    html += "}";
    html += "}";
    html += "loadAPI();";
    html += "setInterval(loadAPI,3000);";
    html += "</script>";

    html += "</body></html>";

    server.send(200, "text/html", html);
}
void WebServerManager::handleSettingsSave()
{
    if (server.hasArg("24hour"))
    {
        setting24Hour = server.arg("24hour") == "1";
        preferences.putBool("24hour", setting24Hour);
    }

    if (server.hasArg("date"))
    {
        settingDate = server.arg("date") == "1";
        preferences.putBool("date", settingDate);
    }

    if (server.hasArg("pir"))
    {
        settingPIR = server.arg("pir") == "1";
        preferences.putBool("pir", settingPIR);
    }

    if (server.hasArg("weatherTime"))
    {
        unsigned long seconds = server.arg("weatherTime").toInt();

        if (seconds < 1)
            seconds = 1;

        if (seconds > 300)
            seconds = 300;

        weatherDisplayTime = seconds * 1000UL;
        preferences.putULong("weatherTime", weatherDisplayTime);
    }

    if (server.hasArg("localTime"))
    {
        unsigned long seconds = server.arg("localTime").toInt();

        if (seconds < 1)
            seconds = 1;

        if (seconds > 300)
            seconds = 300;

        localSensorsDisplayTime = seconds * 1000UL;
        preferences.putULong("localTime", localSensorsDisplayTime);
    }

    server.sendHeader("Location", "/settings");
    server.send(303);
}