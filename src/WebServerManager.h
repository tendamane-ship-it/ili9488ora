#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <Arduino.h>
#include <WebServer.h>

class WebServerManager
{
public:
    WebServerManager();

    void begin();
    void handleClient();

private:
    WebServer server;

    void handleRoot();
    void handleSettings();
    void handleApiStatus();
};

#endif