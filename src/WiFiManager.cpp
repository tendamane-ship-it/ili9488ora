
#include "WiFiManager.h"

static const char* AP_SSID = "oraPas88888888";
static const char* AP_PASSWORD = "88888888";

static const byte DNS_PORT = 53;

WiFiManager::WiFiManager()
    : portalServer(80),
      portalActive(false),
      serverStarted(false) {
}

bool WiFiManager::begin(
    const char* defaultSSID,
    const char* defaultPassword,
    uint32_t timeoutMs
) {
    preferences.begin("wifi", false);

    wifiSSID = preferences.getString("ssid", defaultSSID);
    wifiPassword = preferences.getString("password", defaultPassword);
    Serial.print("Saved SSID: ");
Serial.println(wifiSSID);
Serial.print("Saved password length: ");
Serial.println(wifiPassword.length());

    preferences.end();
    WiFi.mode(WIFI_STA);

    if (connectToWiFi(timeoutMs)) {
        Serial.println("WiFi connected successfully.");
        return true;
    }

    Serial.println("WiFi connection failed. Starting setup portal.");

    startPortal();
    return false;
}
bool WiFiManager::connectToWiFi(uint32_t timeoutMs) {
        WiFi.mode(WIFI_STA);
    delay(300);

    if (wifiSSID.length() == 0) {
        return false;
    }

    Serial.println("Connecting to WiFi...");
   
    WiFi.begin(wifiSSID.c_str(),wifiPassword.c_str()
    );

    unsigned long startAttempt = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startAttempt < timeoutMs
    ) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }

    Serial.println("WiFi connection failed");
    return false;
}
void WiFiManager::startPortal() {
   WiFi.mode(WIFI_OFF);
delay(500);

WiFi.mode(WIFI_AP);
delay(500);
    

    IPAddress apIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);

    bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);

Serial.print("AP start result: ");
Serial.println(apStarted ? "SUCCESS" : "FAILED");

delay(500);

Serial.print("AP IP: ");
Serial.println(WiFi.softAPIP());

Serial.print("AP clients: ");
Serial.println(WiFi.softAPgetStationNum());
    dnsServer.start(DNS_PORT, "*", apIP);

    portalServer.on("/", HTTP_GET, [this]() {
        handleRoot();
    });

    portalServer.on("/save", HTTP_POST, [this]() {
        handleSave();
    });

    portalServer.onNotFound([this]() {
        handleNotFound();
    });

    portalServer.begin();

    portalActive = true;
    serverStarted = true;

    Serial.println("WiFi setup portal started.");
    Serial.print("AP SSID: ");
    Serial.println(AP_SSID);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void WiFiManager::handleRoot() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta charset="UTF-8">
<title>ORA WiFi Setup</title>
<style>
body {
    font-family: Arial;
    background: #101820;
    color: white;
    text-align: center;
    padding: 25px;
}
.container {
    max-width: 400px;
    margin: auto;
}
input {
    width: 90%;
    padding: 13px;
    margin: 10px 0;
    border: 0;
    border-radius: 6px;
    font-size: 16px;
}
button {
    width: 96%;
    padding: 14px;
    background: #00bcd4;
    color: #101820;
    border: 0;
    border-radius: 6px;
    font-size: 17px;
    font-weight: bold;
}
</style>
</head>
<body>
<div class="container">
<h1>ORA WiFi Setup</h1>
<p>Vendos emrin dhe fjalekalimin e rrjetit WiFi.</p>
<form action="/save" method="POST">
<input type="text" name="ssid" placeholder="WiFi SSID" required>
<input type="password" name="password" placeholder="WiFi Password">
<button type="submit">Ruaj dhe lidhu</button>
</form>
</div>
</body>
</html>
)rawliteral";

    portalServer.send(200, "text/html; charset=UTF-8", html);
}

void WiFiManager::handleSave() {
    if (!portalServer.hasArg("ssid")) {
        portalServer.send(400, "text/plain", "SSID mungon.");
        return;
    }

    String newSSID = portalServer.arg("ssid");
    String newPassword = portalServer.arg("password");

    newSSID.trim();

    if (newSSID.isEmpty()) {
        portalServer.send(400, "text/plain", "SSID nuk mund te jete bosh.");
        return;
    }

    saveCredentials(newSSID, newPassword);

    wifiSSID = newSSID;
    wifiPassword = newPassword;

    portalServer.send(
        200,
        "text/html; charset=UTF-8",
        "<html><meta charset='UTF-8'><body>"
        "<h2>Te dhenat u ruajten!</h2>"
        "<p>ORA po provon te lidhet me routerin. Prisni pak.</p>"
        "</body></html>"
    );

    delay(500);

   stopPortal();

if (connectToWiFi(20000)) {
    Serial.println("New WiFi connection successful.");
    Serial.println("Restarting ORA...");
    delay(1000);
    ESP.restart();
} else {
    Serial.println("New WiFi connection failed. Restarting setup portal.");
    startPortal();
}
}

void WiFiManager::saveCredentials(
    const String& ssid,
    const String& password
) {
    preferences.begin("wifi", false);

    preferences.putString("ssid", ssid);
    preferences.putString("password", password);

    preferences.end();

    Serial.println("WiFi credentials saved.");
}

void WiFiManager::stopPortal() {
    if (!portalActive) {
        return;
    }

    dnsServer.stop();
    portalServer.stop();

    WiFi.softAPdisconnect(true);

    portalActive = false;
    serverStarted = false;
}

void WiFiManager::handleNotFound() {
    portalServer.sendHeader("Location", "http://192.168.4.1/", true);
    portalServer.send(302, "text/plain", "");
}

void WiFiManager::handleClient() {
    if (portalActive) {
        dnsServer.processNextRequest();
        portalServer.handleClient();
    }
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiManager::isPortalActive() const {
    return portalActive;
}
