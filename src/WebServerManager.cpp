#include "WebServerManager.h"
#include <WiFi.h>
#include <time.h>
#include "Pages.h"

WebServerManager::WebServerManager()
    : server(80)
{
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

    json += "\"project\":\"testili9848\",";
    json += "\"wifi\":\"";
    json += WiFi.SSID();
    json += "\",";

    json += "\"ip\":\"";
    json += WiFi.localIP().toString();
    json += "\",";

    json += "\"rssi\":";
    json += String(WiFi.RSSI());
    json += ",";

    json += "\"uptime\":";
    json += String(millis() / 1000);

    json += "}";

    server.send(200, "application/json", json);
}