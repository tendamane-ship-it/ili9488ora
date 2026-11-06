#include "WebServerManager.h"
#include <WiFi.h>
#include <time.h>
#include <ArduinoJson.h>
#include "Pages.h"
#include <Preferences.h>
#include "TuyaService.h"
#include "SerialLog.h"

extern Preferences preferences;

extern bool setting24Hour;
extern bool settingDate;
extern bool settingWeather;
extern bool settingLightSensor;
extern bool settingTuyaDisplay;
extern bool settingPIR;
extern unsigned long localSensorsDisplayTime;
extern unsigned long weatherDisplayTime;
extern unsigned long tuyaDisplayTime;
extern float bh1750Lux;
extern bool bh1750OK;
extern bool bh1750ReadingOK;
extern unsigned long lastLuxReading;
extern int pirState;
extern bool settingScreenSaver;
extern unsigned long screenSaverDelay;
extern bool settingGreeting;
extern bool settingNightMode;
extern float nightSleepLux;
extern float nightWakeLux;

extern float bmeTemp;
extern float bmeHumidity;
extern float bmeGas;
extern bool bme680OK;
extern bool bmeReadingOK;
extern unsigned long lastBmeReading;
extern TuyaService tuyaService;

static void appendPageFooter(String& html, bool includeWeatherAttribution = false)
{
    html += "<footer style='margin:28px 0 8px;text-align:center;color:#71838e;font-size:11px;line-height:1.6'>";
    if (includeWeatherAttribution)
    {
        html += "<div>Të dhënat e motit: <a style='color:inherit' href='https://open-meteo.com/'>Open-Meteo</a> — me ndihmën e Murat Mane.</div>";
    }
    html += "<div style='margin-top:5px'>Mundësuar nga Murat Mane</div>";
    html += "</footer>";
}

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

    server.on("/api/logs", [this]()
    {
        handleApiLogs();
    });

    server.on("/logs", [this]()
    {
        handleLogsPage();
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
    Serial.printf("Free heap BEFORE HTML: %u\n", ESP.getFreeHeap());

    String page = getDashboardPage();

    Serial.printf("HTML size: %u bytes\n", page.length());
    Serial.printf("Free heap AFTER HTML: %u\n", ESP.getFreeHeap());

    server.send(200, "text/html", page);
}
void WebServerManager::handleSettings()
{
    server.send(200, "text/html", getSettingsPage());
}
void WebServerManager::handleApiStatus()
{
    Serial.println("API STATUS REQUEST RECEIVED");
    JsonDocument doc;
    doc["project"] = "ILI9488 CLOCK";

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 100))
    {
        char timeText[12];
        char dateText[32];
        strftime(timeText, sizeof(timeText), "%H:%M:%S", &timeinfo);
        strftime(dateText, sizeof(dateText), "%A, %d %B %Y", &timeinfo);
        doc["time"] = timeText;
        doc["date"] = dateText;
    }
    else
    {
        doc["time"] = "--:--:--";
        doc["date"] = "--";
    }

    doc["wifi"] = WiFi.SSID();
    doc["wifiConnected"] = WiFi.status() == WL_CONNECTED;
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
    doc["cpu"] = getCpuFrequencyMhz();
    doc["heap"] = ESP.getFreeHeap();
    doc["flash"] = ESP.getFlashChipSize();
    doc["uptime"] = millis() / 1000;
    doc["webServer"] = true;

    doc["setting24Hour"] = setting24Hour;
    doc["settingDate"] = settingDate;
    doc["settingWeather"] = settingWeather;
    doc["settingLightSensor"] = settingLightSensor;
    doc["settingTuyaDisplay"] = settingTuyaDisplay;
    doc["settingPIR"] = settingPIR;
    doc["weatherDisplayTime"] = weatherDisplayTime / 1000;
    doc["localSensorsDisplayTime"] = localSensorsDisplayTime / 1000;
    doc["tuyaDisplayTime"] = tuyaDisplayTime / 1000;

    if (bmeReadingOK)
    {
        doc["bmeTemperature"] = bmeTemp;
        doc["bmeHumidity"] = bmeHumidity;
        doc["bmeGas"] = bmeGas;
    }
    else
    {
        doc["bmeTemperature"] = nullptr;
        doc["bmeHumidity"] = nullptr;
        doc["bmeGas"] = nullptr;
    }
    doc["bme680OK"] = bme680OK;
    doc["bme680ReadingOK"] = bmeReadingOK;
    if (lastBmeReading != 0)
        doc["bmeReadingAgeMs"] = millis() - lastBmeReading;
    else
        doc["bmeReadingAgeMs"] = nullptr;

    if (bh1750ReadingOK)
        doc["lux"] = bh1750Lux;
    else
        doc["lux"] = nullptr;
    doc["luxValid"] = bh1750ReadingOK;
    if (lastLuxReading != 0)
        doc["luxReadingAgeMs"] = millis() - lastLuxReading;
    else
        doc["luxReadingAgeMs"] = nullptr;

    doc["pirState"] = pirState;
    doc["pirMotion"] = pirState == HIGH;

    if (weather != nullptr)
    {
        const WeatherData& data = weather->getData();
        doc["temperature"] = data.temperature;
        doc["humidity"] = data.humidity;
        doc["pressure"] = data.pressure;
        doc["windSpeed"] = data.windSpeed;
        doc["weatherCode"] = data.weatherCode;
        doc["weatherValid"] = data.valid;
    }

    String json;
    serializeJson(doc, json);
    Serial.printf("API JSON size: %u bytes\n", json.length());
    Serial.println("API: sending response...");
    server.send(200, "application/json", json);
    Serial.println("API: response sent");
}

void WebServerManager::handleApiLogs()
{
    server.send(
        200,
        "text/plain; charset=UTF-8",
        serialLog.getRecentLogs()
    );
}

void WebServerManager::handleLogsPage()
{
    String html;
    html.reserve(1800);
    html += "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<meta name='theme-color' content='#080b10'>";
    html += "<title>ORA - Serial Logs</title>";
    html += "<style>";
    html += "*{box-sizing:border-box}";
    html += "body{margin:0;padding:16px;background:#080b10;color:#e8faff;font-family:Arial,Helvetica,sans-serif}";
    html += ".container{max-width:1000px;margin:auto}";
    html += ".nav{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:18px}";
    html += ".nav a,button{padding:10px 14px;border:1px solid #202b36;border-radius:8px;background:#111720;color:#9aa7b3;text-decoration:none;font-size:14px}";
    html += ".nav a:hover,button:hover{color:#00eaff;border-color:#00eaff}";
    html += ".nav a.active{color:#00eaff;border-color:#00eaff}";
    html += "@media(max-width:520px){body{padding:10px}.nav{gap:6px}.nav a{flex:1 1 105px;text-align:center;padding:11px 8px}h1{font-size:18px}.heading{align-items:flex-start}button{min-height:42px}pre{height:68vh;min-height:200px;padding:10px;font-size:11px}}";
    html += ".heading{display:flex;align-items:center;justify-content:space-between;gap:12px;margin:12px 0}";
    html += "h1{font-size:20px;color:#00eaff}";
    html += "pre{background:#10161e;border:1px solid #202b36;border-radius:10px;color:#b9d1da;font:12px/1.55 monospace;white-space:pre-wrap;overflow-wrap:anywhere;overflow:auto;height:70vh;min-height:220px;padding:14px;margin:0}";
    html += ".note{color:#72838e;font-size:12px;margin:10px 2px}";
    html += "</style></head><body><main class='container'>";
    html += "<nav class='nav'><a href='/'>Dashboard</a><a href='/weather'>Weather</a><a href='/tuya'>Tuya</a><a href='/settings'>Settings</a><a class='active' href='/logs'>Logs</a><a href='/api'>API Status</a></nav>";
    html += "<div class='heading'><h1>Serial Terminal</h1><button id='scrollButton' type='button'>Poshtë</button></div>";
    html += "<pre id='logs'>Duke pritur logjet...</pre>";
    html += "<div class='note'>Shfaqen mesazhet e fundit të ruajtura nga ora. Rifreskohet çdo 2 sekonda.</div>";
    appendPageFooter(html);
    html += "</main><script>";
    html += "const panel=document.getElementById('logs');";
    html += "async function updateLogs(){const atBottom=panel.scrollHeight-panel.scrollTop-panel.clientHeight<24;try{const r=await fetch('/api/logs',{cache:'no-store'});if(!r.ok)throw new Error('HTTP '+r.status);panel.textContent=(await r.text())||'Nuk ka ende mesazhe.';if(atBottom)panel.scrollTop=panel.scrollHeight;}catch(e){panel.textContent='Nuk u lexuan dot logjet: '+e.message;}}";
    html += "document.getElementById('scrollButton').addEventListener('click',()=>panel.scrollTop=panel.scrollHeight);";
    html += "updateLogs();setInterval(updateLogs,2000);";
    html += "</script></body></html>";
    server.send(200, "text/html; charset=UTF-8", html);
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
    html += "pre{background:#1b1b1b;padding:20px;border-radius:10px;overflow:auto;overflow-wrap:anywhere;white-space:pre-wrap;margin-top:20px;max-height:70vh;}";
    html += "@media(max-width:520px){body{padding:10px}.nav{gap:6px}.nav a{flex:1 1 105px;text-align:center;padding:11px 8px}h1{font-size:20px}pre{padding:12px;font-size:12px;max-height:72vh}button{min-height:42px}}";
    html += "</style>";

    html += "</head><body>";

   html += "<div class='nav'>";
html += "<a href='/'>Dashboard</a>";
html += "<a href='/weather'>Weather</a>";
html += "<a href='/tuya'>Tuya</a>";
html += "<a href='/settings'>Settings</a>";
html += "<a href='/logs'>Logs</a>";
html += "<a href='/api' class='active'>API Status</a>";
html += "</div>";

html += "<h1>API Status</h1>";

html += "<button onclick='loadAPI()'>Refresh</button>";

    html += "<pre id='json'>Loading...</pre>";

        appendPageFooter(html);

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
    html += ".nav{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:20px;}";
    html += ".nav a{display:inline-block;background:#1b1b1b;border:1px solid #444;padding:9px 15px;border-radius:8px;font-size:14px;margin:0;color:#9aa7b3;}";
    html += ".nav a:hover,.nav a.active{color:#00eaff;border-color:#00eaff;}";
    html += "@media(max-width:520px){body{padding:10px}.container{width:100%}.nav{gap:6px}.nav a{flex:1 1 105px;text-align:center;padding:11px 8px}.card{padding:16px;margin-top:14px}.value{font-size:clamp(26px,8vw,32px)}h1{font-size:22px}}";
    html += "</style>";

    html += "</head>";
    html += "<body>";

    html += "<div class='container'>";

    html += "<div class='nav'>";
    html += "<a href='/'>Dashboard</a>";
    html += "<a href='/weather'>Weather</a>";
    html += "<a class='active' href='/tuya'>Tuya</a>";
    html += "<a href='/settings'>Settings</a>";
    html += "<a href='/logs'>Logs</a>";
    html += "<a href='/api'>API Status</a>";
    html += "</div>";

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

    appendPageFooter(html);

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
    html += ".nav{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:20px;}";
    html += ".nav a{display:inline-block;color:#9aa7b3;background:#1b1b1b;border:1px solid #444;padding:9px 15px;border-radius:8px;text-decoration:none;font-size:14px;}";
    html += ".nav a:hover,.nav a.active{color:#00eaff;border-color:#00eaff;}";
    html += "@media(max-width:600px){body{padding:12px}.container{width:100%}.nav{gap:6px}.nav a{flex:1 1 105px;text-align:center;padding:11px 8px}.grid{grid-template-columns:repeat(2,minmax(0,1fr));gap:8px}.card{padding:12px;min-width:0}.value{font-size:clamp(18px,5vw,24px);overflow-wrap:anywhere}h1{font-size:22px}h2{font-size:18px;margin-top:24px}.forecast{margin-right:-12px}table{min-width:650px}th,td{padding:9px 7px;font-size:13px}}";
    html += "</style>";

    html += "</head>";
    html += "<body>";

    html += "<div class='container'>";

    // Navigation
    html += "<div class='nav'>";
    html += "<a href='/'>Dashboard</a>";
    html += "<a class='active' href='/weather'>Weather</a>";
    html += "<a href='/tuya'>Tuya</a>";
    html += "<a href='/settings'>Settings</a>";
    html += "<a href='/logs'>Logs</a>";
    html += "<a href='/api'>API Status</a>";
    html += "</div>";

    html += "<h1>ORA — Weather</h1>";

    if (weather == nullptr)
    {
        html += "<div class='card'>";
        html += "Weather service unavailable.";
        html += "</div>";
        appendPageFooter(html, true);
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
        appendPageFooter(html, true);
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

    appendPageFooter(html, true);

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

    if (server.hasArg("tuyaDisplay"))
    {
        settingTuyaDisplay = server.arg("tuyaDisplay") == "1";
        preferences.putBool("tuyaDisplay", settingTuyaDisplay);
    }

    if (server.hasArg("weather"))
    {
        settingWeather = server.arg("weather") == "1";
        preferences.putBool("weather", settingWeather);
    }

    if (server.hasArg("light"))
    {
        settingLightSensor = server.arg("light") == "1";
        preferences.putBool("light", settingLightSensor);
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

    if (server.hasArg("tuyaTime"))
    {
        unsigned long seconds = server.arg("tuyaTime").toInt();
        if (seconds < 1)
            seconds = 1;
        if (seconds > 300)
            seconds = 300;

        tuyaDisplayTime = seconds * 1000UL;
        preferences.putULong("tuyaTime", tuyaDisplayTime);
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


if (server.hasArg("greeting"))
{
    settingGreeting = server.arg("greeting") == "1";
    preferences.putBool("greeting", settingGreeting);
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