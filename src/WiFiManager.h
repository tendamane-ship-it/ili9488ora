
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

class WiFiManager {
public:
    struct NetworkConfig {
        bool useDhcp = false;
        String ipAddress = "192.168.1.55";
        String gateway = "192.168.1.1";
        String subnetMask = "255.255.255.0";
        String dns1 = "192.168.1.1";
        String dns2 = "1.1.1.1";
    };

    WiFiManager();
    NetworkConfig getNetworkConfig() const;
    bool saveNetworkConfig(const NetworkConfig& config, String& error);

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
    NetworkConfig networkConfig;

    bool portalActive;
    bool serverStarted;
    bool wasConnected;
    uint8_t reconnectAttempts;
    unsigned long nextReconnectAt;

    void startPortal();
    void stopPortal();

    void handleRoot();
    void handleSave();
    void handleNotFound();

    bool connectToWiFi(uint32_t timeoutMs);
    void saveCredentials(const String& ssid, const String& password);
};

#endif
