
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

class WiFiManager {
public:
    WiFiManager();

    // Ngarkon kredencialet dhe provon lidhjen me routerin.
    // Nëse lidhja dështon, hap portalin e konfigurimit.
    bool begin(
        const char* defaultSSID,
        const char* defaultPassword,
        uint32_t timeoutMs = 20000
    );

    // Përpunon kërkesat e portalit gjatë loop().
    void handleClient();

    bool isConnected() const;
    bool isPortalActive() const;

private:
    WebServer portalServer;
    DNSServer dnsServer;
    Preferences preferences;

    String wifiSSID;
    String wifiPassword;

    bool portalActive;
    bool serverStarted;

    void startPortal();
    void stopPortal();

    void handleRoot();
    void handleSave();
    void handleNotFound();

    bool connectToWiFi(uint32_t timeoutMs);
    void saveCredentials(const String& ssid, const String& password);
};

#endif
