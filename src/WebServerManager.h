#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <Arduino.h>
#include <WebServer.h>
#include "WeatherService.h"

class WebServerManager
{
public:
    WebServerManager();

    void begin();
    void handleClient();
    void setWeatherService(WeatherService* service);

private:
    WebServer server;
    WeatherService* weather;

    void handleRoot();
    void handleSettings();
    void handleApiStatus();
    void handleApiPage();
    void handleSettingsSave();
};

#endif