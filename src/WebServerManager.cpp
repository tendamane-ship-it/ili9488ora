#include "WebServerManager.h"
#include <WiFi.h>
#include <time.h>
#include "Pages.h"

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

    // Project
    json += "\"project\":\"ILI9488 CLOCK\",";

    // Time
    struct tm timeinfo;

    if (getLocalTime(&timeinfo, 100))
    {
        char timeText[12];
        strftime(timeText, sizeof(timeText), "%H:%M:%S", &timeinfo);

        json += "\"time\":\"";
        json += timeText;
        json += "\",";
    }
    else
    {
        json += "\"time\":\"--:--:--\",";
    }

    // WiFi
    json += "\"wifi\":\"";
    json += WiFi.SSID();
    json += "\",";

    // IP
    json += "\"ip\":\"";
    json += WiFi.localIP().toString();
    json += "\",";

    // RSSI
    json += "\"rssi\":";
    json += String(WiFi.RSSI());
    json += ",";

    // CPU
    json += "\"cpu\":";
    json += String(getCpuFrequencyMhz());
    json += ",";

    // Free Heap
    json += "\"heap\":";
    json += String(ESP.getFreeHeap());
    json += ",";

   // Uptime
json += "\"uptime\":";
json += String(millis() / 1000);

// Weather
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