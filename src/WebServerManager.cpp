#include "WebServerManager.h"
#include <WiFi.h>
#include <time.h>
#include "Pages.h"
#include <Preferences.h>
#include "TuyaService.h"

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
extern bool settingScreenSaver;
extern unsigned long screenSaverDelay;
extern bool settingNightMode;
extern float nightSleepLux;
extern float nightWakeLux;

extern float bmeTemp;
extern float bmeHumidity;
extern float bmeGas;
extern bool bme680OK;
extern TuyaService tuyaService;
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
    server.on("/tuya", [this]() { handleTuya(); });
    server.on("/weather", [this]()
{
    handleWeather();
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
   html += ".nav{display:flex;gap:8px;margin-bottom:20px;flex-wrap:wrap;}";
html += ".nav a{color:#9aa7b3;background:#1b1b1b;border:1px solid #444;padding:9px 15px;border-radius:8px;text-decoration:none;font-size:14px;}";
html += ".nav a:hover{color:#00eaff;border-color:#00eaff;}";
html += ".nav a.active{color:#00eaff;border-color:#00eaff;}";
    html += "pre{background:#1b1b1b;padding:20px;border-radius:10px;overflow:auto;margin-top:20px;}";
    html += "</style>";

    html += "</head><body>";

   html += "<div class='nav'>";
html += "<a href='/'>Dashboard</a>";
html += "<a href='/weather'>Weather</a>";
html += "<a href='/tuya'>Tuya</a>";
html += "<a href='/settings'>Settings</a>";
html += "<a href='/api' class='active'>API Status</a>";
html += "</div>";

html += "<h1>API Status</h1>";

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
void WebServerManager::handleTuya()
{
    String html;

    html += "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<meta http-equiv='refresh' content='60'>";
    html += "<title>ORA - Tuya TH01</title>";

    html += "<style>";
    html += "body{font-family:Arial,sans-serif;background:#111;color:#eee;margin:0;padding:20px;}";
    html += ".container{max-width:600px;margin:auto;}";
    html += "h1{color:#00ffff;}";
    html += ".card{background:#1d1d1d;border-radius:12px;padding:20px;margin-top:20px;}";
    html += ".value{font-size:32px;color:#00ffff;margin:10px 0;}";
    html += ".label{color:#aaa;font-size:14px;}";
    html += ".status{color:#00ff66;font-weight:bold;}";
    html += "a{color:#00ffff;text-decoration:none;}";
    html += "</style>";

    html += "</head>";
    html += "<body>";

    html += "<div class='container'>";

    html += "<h1>ORA — Tuya TH01</h1>";

    html += "<div class='card'>";

    if (tuyaService.isOnline())
    {
        html += "<div class='status'>● ONLINE</div>";

        html += "<div class='label'>Temperature</div>";
        html += "<div class='value'>";
        html += String(tuyaService.getTemperature(), 1);
        html += " &deg;C</div>";

        html += "<div class='label'>Humidity</div>";
        html += "<div class='value'>";
        html += String(tuyaService.getHumidity(), 1);
        html += " %</div>";

        html += "<div class='label'>Battery</div>";
        html += "<div class='value'>";
        html += String(tuyaService.getBattery());
        html += " %</div>";
    }
    else
    {
        html += "<div style='color:#ff4444;font-weight:bold;'>● OFFLINE</div>";
        html += "<p>Tuya TH01 nuk është aktualisht i disponueshëm.</p>";
    }

    html += "</div>";

    html += "<p style='margin-top:25px;'>";
    html += "<a href='/'>← ORA Dashboard</a>";
    html += "</p>";

    html += "</div>";

    html += "</body>";
    html += "</html>";

    server.send(200, "text/html; charset=UTF-8", html);
}
void WebServerManager::handleWeather()
{
    String html;

    html += "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<meta http-equiv='refresh' content='300'>";
    html += "<title>ORA - Weather</title>";

    html += "<style>";
    html += "body{font-family:Arial,sans-serif;background:#111;color:#eee;margin:0;padding:20px;}";
    html += ".container{max-width:900px;margin:auto;}";
    html += "h1{color:#00ffff;}";
    html += "h2{color:#00ffff;margin-top:30px;}";
    html += ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:12px;}";
    html += ".card{background:#1d1d1d;border-radius:12px;padding:18px;}";
    html += ".label{color:#aaa;font-size:14px;}";
    html += ".value{font-size:26px;color:#00ffff;margin-top:6px;}";
    html += ".forecast{overflow-x:auto;}";
    html += "table{width:100%;border-collapse:collapse;background:#1d1d1d;border-radius:12px;overflow:hidden;}";
    html += "th,td{padding:12px;text-align:center;border-bottom:1px solid #333;}";
    html += "th{color:#00ffff;}";
    html += "a{color:#00ffff;text-decoration:none;margin-right:18px;}";
    html += ".nav{margin-bottom:20px;line-height:2;}";
    html += "</style>";

    html += "</head>";
    html += "<body>";

    html += "<div class='container'>";

    // Navigation
    html += "<div class='nav'>";
    html += "<a href='/'>Dashboard</a>";
    html += "<a href='/weather'>Weather</a>";
    html += "<a href='/tuya'>Tuya</a>";
    html += "<a href='/settings'>Settings</a>";
    html += "<a href='/api'>API</a>";
    html += "</div>";

    html += "<h1>ORA — Weather</h1>";

    if (weather == nullptr)
    {
        html += "<div class='card'>";
        html += "Weather service unavailable.";
        html += "</div>";
        html += "</div></body></html>";

        server.send(200, "text/html; charset=UTF-8", html);
        return;
    }

    const WeatherData& data = weather->getData();

    if (!data.valid)
    {
        html += "<div class='card'>";
        html += "Weather data is not available.";
        html += "</div>";
        html += "</div></body></html>";

        server.send(200, "text/html; charset=UTF-8", html);
        return;
    }

    // --------------------------------------------------------
    // CURRENT WEATHER
    // --------------------------------------------------------

    html += "<h2>Current Weather</h2>";

    html += "<div class='grid'>";

    html += "<div class='card'>";
    html += "<div class='label'>Temperature</div>";
    html += "<div class='value'>";
    html += String(data.temperature, 1);
    html += " &deg;C</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<div class='label'>Feels like</div>";
    html += "<div class='value'>";
    html += String(data.apparentTemperature, 1);
    html += " &deg;C</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<div class='label'>Humidity</div>";
    html += "<div class='value'>";
    html += String(data.humidity, 0);
    html += " %</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<div class='label'>Pressure</div>";
    html += "<div class='value'>";
    html += String(data.pressure, 1);
    html += " hPa</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<div class='label'>Wind</div>";
    html += "<div class='value'>";
    html += String(data.windSpeed, 1);
    html += " km/h</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<div class='label'>Wind direction</div>";
    html += "<div class='value'>";
    html += String(data.windDirection, 0);
    html += " &deg;</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<div class='label'>Wind gusts</div>";
    html += "<div class='value'>";
    html += String(data.windGusts, 1);
    html += " km/h</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<div class='label'>Cloud cover</div>";
    html += "<div class='value'>";
    html += String(data.cloudCover, 0);
    html += " %</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<div class='label'>Precipitation</div>";
    html += "<div class='value'>";
    html += String(data.precipitation, 1);
    html += " mm</div>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<div class='label'>Rain</div>";
    html += "<div class='value'>";
    html += String(data.rain, 1);
    html += " mm</div>";
    html += "</div>";

    html += "</div>";

    // --------------------------------------------------------
    // 7 DAY FORECAST
    // --------------------------------------------------------

    html += "<h2>7 Day Forecast</h2>";

    html += "<div class='forecast'>";

    html += "<table>";

    html += "<tr>";
    html += "<th>Date</th>";
    html += "<th>Min</th>";
    html += "<th>Max</th>";
    html += "<th>Rain</th>";
    html += "<th>Chance</th>";
    html += "<th>Wind</th>";
    html += "<th>Code</th>";
    html += "</tr>";

    for (int i = 0; i < WeatherService::FORECAST_DAYS; i++)
    {
        const DailyWeather& day = weather->getDaily(i);

        html += "<tr>";

        html += "<td>";
        html += day.date;
        html += "</td>";

        html += "<td>";
        html += String(day.temperatureMin, 1);
        html += " &deg;C</td>";

        html += "<td>";
        html += String(day.temperatureMax, 1);
        html += " &deg;C</td>";

        html += "<td>";
        html += String(day.precipitationSum, 1);
        html += " mm</td>";

        html += "<td>";
        html += String(day.precipitationProbability);
        html += " %</td>";

        html += "<td>";
        html += String(day.windSpeedMax, 1);
        html += " km/h</td>";

        html += "<td>";
        html += String(day.weatherCode);
        html += "</td>";

        html += "</tr>";
    }

    html += "</table>";

    html += "</div>";

    // --------------------------------------------------------
    // FOOTER
    // --------------------------------------------------------

    html += "<p style='margin-top:25px;color:#777;'>";
    html += "Data provided by Open-Meteo";
    html += "</p>";

    html += "</div>";

    html += "</body>";
    html += "</html>";

    server.send(200, "text/html; charset=UTF-8", html);
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
if (server.hasArg("screensaver"))
{
    settingScreenSaver = server.arg("screensaver") == "1";
    preferences.putBool("screensaver", settingScreenSaver);
}

if (server.hasArg("screenSaverMin"))
{
    unsigned long minutes = server.arg("screenSaverMin").toInt();

    if (minutes < 1)
        minutes = 1;

    if (minutes > 60)
        minutes = 60;

    screenSaverDelay = minutes * 60UL * 1000UL;
    preferences.putULong("screenSaverMin", minutes);
}
if (server.hasArg("nightMode"))
{
    settingNightMode = server.arg("nightMode") == "1";
    preferences.putBool("nightMode", settingNightMode);
}

if (server.hasArg("nightSleepLux"))
{
    float lux = server.arg("nightSleepLux").toFloat();

    if (lux < 0.0f)
        lux = 0.0f;

    if (lux > 1000.0f)
        lux = 1000.0f;

    nightSleepLux = lux;
    preferences.putFloat("nightSleepLux", nightSleepLux);
}

if (server.hasArg("nightWakeLux"))
{
    float lux = server.arg("nightWakeLux").toFloat();

    if (lux < 0.0f)
        lux = 0.0f;

    if (lux > 1000.0f)
        lux = 1000.0f;

    nightWakeLux = lux;

    if (nightWakeLux <= nightSleepLux)
        nightWakeLux = nightSleepLux + 1.0f;

    preferences.putFloat("nightWakeLux", nightWakeLux);
}
    server.sendHeader("Location", "/settings");
    server.send(303);
}