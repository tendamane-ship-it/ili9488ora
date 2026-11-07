#include <Arduino.h>
#include <WiFi.h>
#include "WebServerManager.h"
#include "TuyaService.h"
#include "Pages.h"
#include "WeatherService.h"
#include "Sensors.h"
#include "settings.h"
#include "ScreenSaver.h"
#include "Startup.h"
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "WiFiManager.h"
#include "SerialLog.h"
#include "AboutScreen.h"
#include "ClockDisplay.h"
#include "WeatherDisplay.h"
#define TOUCH_CS   17
#define TOUCH_IRQ  14

XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// ============================================================
// WIFI
// ============================================================
WiFiManager wifiManager;
const char* NTP_SERVER = "pool.ntp.org";
// ============================================================
// TFT
// ============================================================
TFT_eSPI tft = TFT_eSPI();
struct tm timeinfo;
volatile bool timeSynchronized = false;
volatile bool weatherUpdatePending = false;
volatile bool tuyaUpdatePending = false;
volatile bool weatherServiceReady = false;
volatile bool weatherServiceChecked = false;
volatile bool tuyaServiceReady = false;
volatile bool tuyaServiceChecked = false;
volatile bool wifiGotIpPending = false;
volatile bool networkServicesRefreshPending = false;
bool startupActive = true;
unsigned long startupStartedAt = 0;
unsigned long startupResultsAt = 0;
unsigned long ntpStartedAt = 0;
WebServerManager webServer;
TuyaService tuyaService;
unsigned long lastTuyaUpdate = 0;
const unsigned long TUYA_UPDATE_INTERVAL = 60UL * 1000UL;
WeatherService weather;
int pirState = LOW;

bool getSyncedTime(struct tm& result)
{
    time_t now = time(nullptr);
    if (now < 1483228800)
        return false;

    localtime_r(&now, &result);
    return true;
}

// ============================================================
// COLORS
// ============================================================
#define COLOR_BG    TFT_BLACK
#define COLOR_TIME  TFT_CYAN
#define COLOR_DATE  TFT_WHITE
#define COLOR_WIFI  TFT_GREEN
unsigned long lastMotionTime = 0;
const unsigned long WELCOME_DELAY = 30UL * 60UL * 1000UL;
bool screenSaverActive = false;
unsigned long screenSaverStart = 0;
bool screenSaverPIRArmed = false;
unsigned long screenSaverDelay = 15UL * 60UL * 1000UL;
const unsigned long HEART_INTERVAL = 1200UL;
// Night Mode
bool nightModeActive = false;

enum ScreenMode
{
    SCREEN_CLOCK,
    SCREEN_MENU,
    SCREEN_ABOUT,
    SCREEN_ICON_PREVIEW
};

ScreenMode screenMode = SCREEN_CLOCK;

unsigned long aboutStartTime = 0;
const unsigned long ABOUT_TIMEOUT = 15000UL;
unsigned long menuStartTime = 0;
const unsigned long MENU_TIMEOUT = 30000UL;

const int MAIN_CONTENT_CENTER_X = 277;
// ============================================================
// NETWORK INDICATORS
// ============================================================
bool lastWiFiState = false;
bool lastNtpState = false;
bool lastBmeState = false;
bool lastBh1750State = false;
int lastPirIndicatorState = -1;
int lastWeatherIndicatorState = -1;
int lastTuyaIndicatorState = -1;
bool networkIndicatorsDrawn = false;

void drawMainDividers()
{
    tft.drawFastVLine(68, 0, 241, TFT_DARKGREY);
    tft.drawFastHLine(0, 242, 480, TFT_DARKGREY);
}

void drawWiFiIcon(uint16_t color, int verticalOffset = 0)
{
    tft.drawLine(2, 63 + verticalOffset, 5, 60 + verticalOffset, color);
    tft.drawLine(5, 60 + verticalOffset, 9, 59 + verticalOffset, color);
    tft.drawLine(9, 59 + verticalOffset, 13, 60 + verticalOffset, color);
    tft.drawLine(13, 60 + verticalOffset, 16, 63 + verticalOffset, color);
    tft.drawLine(5, 64 + verticalOffset, 7, 62 + verticalOffset, color);
    tft.drawLine(7, 62 + verticalOffset, 9, 61 + verticalOffset, color);
    tft.drawLine(9, 61 + verticalOffset, 11, 62 + verticalOffset, color);
    tft.drawLine(11, 62 + verticalOffset, 13, 64 + verticalOffset, color);
    tft.fillCircle(9, 67 + verticalOffset, 2, color);
}

void updateNetworkIndicators(bool force = false)
{
    bool wifiConnected = WiFi.status() == WL_CONNECTED;
    bool ntpSynchronized = timeSynchronized;
    bool bmeAvailable = bme680OK && bmeReadingOK;
    bool bh1750Available = bh1750OK && bh1750ReadingOK;
    int pirIndicatorState = !settingPIR
        ? 0
        : (pirState == HIGH ? 2 : 1);
    int weatherIndicatorState = weatherServiceReady
        ? 2
        : (weatherServiceChecked ? 0 : 1);
    int tuyaIndicatorState = tuyaServiceReady
        ? 2
        : (tuyaServiceChecked ? 0 : 1);
    if (!force &&
        networkIndicatorsDrawn &&
        wifiConnected == lastWiFiState &&
        ntpSynchronized == lastNtpState &&
        bmeAvailable == lastBmeState &&
        bh1750Available == lastBh1750State &&
        pirIndicatorState == lastPirIndicatorState &&
        weatherIndicatorState == lastWeatherIndicatorState &&
        tuyaIndicatorState == lastTuyaIndicatorState)
        return;

    lastWiFiState = wifiConnected;
    lastNtpState = ntpSynchronized;
    lastBmeState = bmeAvailable;
    lastBh1750State = bh1750Available;
    lastPirIndicatorState = pirIndicatorState;
    lastWeatherIndicatorState = weatherIndicatorState;
    lastTuyaIndicatorState = tuyaIndicatorState;
    networkIndicatorsDrawn = true;

    uint16_t wifiColor = wifiConnected ? COLOR_WIFI : TFT_RED;
    uint16_t ntpColor = ntpSynchronized
        ? TFT_GREEN
        : (wifiConnected ? TFT_YELLOW : TFT_RED);
    uint16_t bmeColor = bmeAvailable ? TFT_GREEN : TFT_RED;
    uint16_t bh1750Color = bh1750Available ? TFT_GREEN : TFT_RED;
    uint16_t pirColor = pirIndicatorState == 0
        ? TFT_DARKGREY
        : (pirIndicatorState == 2 ? TFT_YELLOW : TFT_GREEN);
    uint16_t weatherColor = weatherIndicatorState == 2
        ? TFT_GREEN
        : (weatherIndicatorState == 1 ? TFT_YELLOW : TFT_RED);
    uint16_t tuyaColor = tuyaIndicatorState == 2
        ? TFT_GREEN
        : (tuyaIndicatorState == 1 ? TFT_YELLOW : TFT_RED);

    constexpr int indicatorOffset = 28;
    tft.fillRect(0, 78, 70, 142, COLOR_BG);
    drawWiFiIcon(wifiColor, indicatorOffset);
    tft.drawCircle(9, 79 + indicatorOffset, 5, ntpColor);
    tft.drawLine(
        9, 79 + indicatorOffset,
        9, 76 + indicatorOffset,
        ntpColor
    );
    tft.drawLine(
        9, 79 + indicatorOffset,
        12, 81 + indicatorOffset,
        ntpColor
    );
    tft.fillCircle(9, 97 + indicatorOffset, 4, bmeColor);
    tft.fillCircle(9, 115 + indicatorOffset, 4, bh1750Color);
    tft.fillCircle(9, 133 + indicatorOffset, 4, pirColor);
    tft.fillCircle(9, 151 + indicatorOffset, 4, weatherColor);
    tft.fillCircle(9, 169 + indicatorOffset, 4, tuyaColor);

    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextColor(wifiColor, COLOR_BG);
    tft.setCursor(19, 57 + indicatorOffset);
    tft.print("WiFi");
    tft.setTextColor(ntpColor, COLOR_BG);
    tft.setCursor(19, 75 + indicatorOffset);
    tft.print("NTP");
    tft.setTextColor(bmeColor, COLOR_BG);
    tft.setCursor(19, 93 + indicatorOffset);
    tft.print("BME680");
    tft.setTextColor(bh1750Color, COLOR_BG);
    tft.setCursor(19, 111 + indicatorOffset);
    tft.print("BH1750");
    tft.setTextColor(pirColor, COLOR_BG);
    tft.setCursor(19, 129 + indicatorOffset);
    tft.print("PIR");
    tft.setTextColor(weatherColor, COLOR_BG);
    tft.setCursor(19, 147 + indicatorOffset);
    tft.print("MOTI");
    tft.setTextColor(tuyaColor, COLOR_BG);
    tft.setCursor(19, 165 + indicatorOffset);
    tft.print("TUYA");

    drawMainDividers();
}

// ============================================================
// WEATHER AREA
// ============================================================
#define WEATHER_Y 245
#define WEATHER_UPDATE_INTERVAL 300000UL
// ============================================================
// LOWER PANEL SLIDE
// ============================================================
#define LOWER_PANEL_INTERVAL 10000UL
bool showingLocalSensors = false;
bool showingTuyaData = false;
bool lowerPanelHasContent = false;
unsigned long lastLowerPanelChange = 0;
unsigned long lastWeatherUpdate = 0;
// ============================================================
// MENU ICON
// ============================================================
void drawMenuIcon()
{
    tft.fillRect(
        0,
        24,
        43,
        26,
        COLOR_BG
    );

    tft.drawLine(5, 28, 35, 28, TFT_WHITE);
    tft.drawLine(5, 36, 35, 36, TFT_WHITE);
    tft.drawLine(5, 44, 35, 44, TFT_WHITE);

    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, COLOR_BG);
    tft.setCursor(5, 50);
    tft.print("Menu");
}
void drawMainMenu()
{
    
    tft.fillScreen(COLOR_BG);
    tft.setTextDatum(MC_DATUM);

    // Titulli
    tft.setTextColor(TFT_CYAN, COLOR_BG);
    tft.setTextFont(4);
    tft.setTextSize(1);
    tft.drawString("MENU", 240, 25);

    // IP address
    String ip = WiFi.localIP().toString();

    tft.setTextColor(TFT_WHITE, COLOR_BG);
    tft.setTextFont(2);
    tft.drawString(
        "IP: " + ip,
        240,
        60
    );

    // 12H / 24H
    tft.drawRect(80, 95, 320, 45, TFT_WHITE);

    tft.setTextColor(TFT_CYAN, COLOR_BG);
    tft.setTextFont(2);

    if (setting24Hour)
        tft.drawString("24H", 240, 117);
    else
        tft.drawString("12H", 240, 117);

    // DATA ON / OFF
    tft.drawRect(80, 150, 320, 45, TFT_WHITE);

    if (settingDate)
        tft.setTextColor(TFT_GREEN, COLOR_BG);
    else
        tft.setTextColor(TFT_RED, COLOR_BG);

    tft.drawString(
        settingDate ? "DATA ON" : "DATA OFF",
        240,
        172
    );

    // ABOUT
    tft.drawRect(80, 205, 320, 45, TFT_WHITE);

    tft.setTextColor(TFT_WHITE, COLOR_BG);
    tft.drawString("ABOUT", 240, 227);

    tft.drawRect(80, 260, 320, 45, TFT_WHITE);
    tft.setTextColor(TFT_YELLOW, COLOR_BG);
    tft.drawString("WEATHER ICONS", 240, 282);

    tft.setTextDatum(TL_DATUM);
}
void drawAbout()
{
    const WeatherData weatherData = weather.getData();
    const bool wifiConnected = WiFi.status() == WL_CONNECTED;
    const bool tuyaOnline = tuyaService.isOnline();
    AboutScreenData data{};
    data.wifiConnected = wifiConnected;
    data.ipAddress = wifiConnected ? WiFi.localIP().toString() : "DISCONNECTED";
    data.wifiRssi = wifiConnected ? WiFi.RSSI() : 0;
    data.timeSynchronized = timeSynchronized;
    data.bmeFound = bme680OK;
    data.bmeAvailable = bme680OK && bmeReadingOK;
    data.bmeTemperature = bmeTemp;
    data.bmeHumidity = bmeHumidity;
    data.lightSensorFound = bh1750OK;
    data.lightSensorAvailable = bh1750OK && bh1750ReadingOK;
    data.lightLevel = bh1750Lux;
    data.pirEnabled = settingPIR;
    data.localDisplayEnabled = settingLightSensor;
    data.weatherReady = weatherServiceReady;
    data.weatherValid = weatherData.valid;
    data.weatherTemperature = weatherData.temperature;
    data.tuyaOnline = tuyaOnline;
    data.tuyaTemperature = tuyaOnline ? tuyaService.getTemperature() : 0;
    data.tuyaHumidity = tuyaOnline ? tuyaService.getHumidity() : 0;
    data.tuyaBattery = tuyaOnline ? tuyaService.getBattery() : 0;
    data.weatherDisplayEnabled = settingWeather;
    data.tuyaDisplayEnabled = settingTuyaDisplay;
    data.freeHeapKb = ESP.getFreeHeap() / 1024;
    drawAboutScreen(tft, data);
}
void openMenu()
{
    screenMode = SCREEN_MENU;
    menuStartTime = millis();
    drawMainMenu();
}

void openAbout()
{
    screenMode = SCREEN_ABOUT;
    aboutStartTime = millis();
    drawAbout();
}

void openWeatherIconPreview()
{
    screenMode = SCREEN_ICON_PREVIEW;
    drawWeatherIconGallery(tft);
}

void updateLowerPanel();
void closeMenu()
{
    screenMode = SCREEN_CLOCK;

    tft.fillScreen(COLOR_BG);

    resetClockDisplay();

    if (getSyncedTime(timeinfo))
    {
        updateClockDisplay(tft, timeinfo, setting24Hour);

        if (settingDate)
            updateDateDisplay(tft, timeinfo);

        bool visible = (timeinfo.tm_sec % 2 == 0);
        drawClockColon(tft, visible);
    }

    updateLowerPanel();
    drawMenuIcon();
    updateNetworkIndicators(true);
}
// ============================================================
// LOCAL SENSORS DISPLAY
// ============================================================
void updateLocalSensorsDisplay()
{
    tft.fillRect(
        75,
        WEATHER_Y,
        405,
        75,
        COLOR_BG
    );

    // Titulli
    tft.setTextColor(TFT_WHITE, COLOR_BG);
    tft.setTextFont(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(
        "Sensoret e dhomes",
        277,
        WEATHER_Y + 7
    );
    tft.setTextDatum(TL_DATUM);

    // Madhesia e te dhenave
    tft.setTextFont(4);

    // Temperature
    tft.setTextColor(TFT_ORANGE, COLOR_BG);
    tft.setCursor(85, WEATHER_Y + 25);

    if (bmeReadingOK)
    {
        tft.print("Temp: ");
        tft.print(bmeTemp, 1);
        tft.print(" C");
    }
    else if (bme680OK)
    {
        tft.print("Temp: ERR");
    }
    else
    {
        tft.print("TEMP: --.- °C");
    }

    // Humidity
    tft.setTextColor(TFT_CYAN, COLOR_BG);
    tft.setCursor(285, WEATHER_Y + 25);

    if (bmeReadingOK)
    {
        tft.print("Lag: ");
        tft.print(bmeHumidity, 0);
        tft.print(" %");
    }
    else if (bme680OK)
    {
        tft.print("Lag: ERR");
    }
    else
    {
        tft.print("Lag: -- %");
    }

    // Lux
    tft.setTextColor(TFT_YELLOW, COLOR_BG);
    tft.setCursor(85, WEATHER_Y + 52);

    if (bh1750ReadingOK)
    {
        tft.print("LUX: ");
        tft.print(bh1750Lux, 0);
    }
    else if (bh1750OK)
    {
        tft.print("LUX: ERR");
    }
    else
    {
        tft.print("LUX: ----");
    }

    // Air quality
    tft.setTextColor(TFT_GREEN, COLOR_BG);
    tft.setCursor(285, WEATHER_Y + 52);

    if (!bme680OK)
    {
        tft.print("Ajer: --");
    }
    else if (!bmeReadingOK)
    {
        tft.print("AIR: ERR");
    }
    else if (!airBaselineReady)
    {
        tft.print("AIR: CAL...");
    }
    else
    {
        float ratio = bmeGas / airBaseline;

        if (ratio >= 0.80)
            tft.print("AIR: GOOD");
        else if (ratio >= 0.50)
            tft.print("AIR: FAIR");
        else
            tft.print("AIR: POOR");
    }
}
void updateTuyaDisplay()
{
    tft.fillRect(
        75,
        WEATHER_Y,
        405,
        75,
        COLOR_BG
    );

    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, COLOR_BG);
    tft.drawString("TUYA", 277, WEATHER_Y + 8);

    tft.setTextFont(4);
    if (tuyaService.isOnline())
    {
        char temperatureText[32];
        char statusText[40];
        snprintf(
            temperatureText,
            sizeof(temperatureText),
            "Temp: %.1f C",
            tuyaService.getTemperature()
        );
        snprintf(
            statusText,
            sizeof(statusText),
            "Lag: %.0f%%  Bat: %d%%",
            tuyaService.getHumidity(),
            tuyaService.getBattery()
        );
        tft.setTextColor(TFT_CYAN, COLOR_BG);
        tft.drawString(temperatureText, 277, WEATHER_Y + 31);
        tft.drawString(statusText, 277, WEATHER_Y + 59);
    }
    else
    {
        tft.setTextColor(TFT_RED, COLOR_BG);
        tft.drawString("Offline", 277, WEATHER_Y + 47);
    }

    tft.setTextDatum(TL_DATUM);
}

void renderLowerPanel()
{
    tft.fillRect(75, WEATHER_Y, 405, 75, COLOR_BG);
    if (showingTuyaData)
        updateTuyaDisplay();
    else if (showingLocalSensors)
        updateLocalSensorsDisplay();
    else if (settingWeather)
        drawWeatherDisplay(tft, weather.getData());
    lowerPanelHasContent = true;
    lastLowerPanelChange = millis();
}

void updateLowerPanel();
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
        Serial.print("WiFi disconnect reason: ");
        Serial.println(info.wifi_sta_disconnected.reason);
    } else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
        wifiGotIpPending = true;
        Serial.print("WiFi got IP: ");
        Serial.println(WiFi.localIP());
    }
}

void networkServicesTask(void* parameter)
{
    (void)parameter;
    bool wasConnected = true;

    for (;;)
    {
        bool connected = wifiManager.isConnected();
        if (!connected)
        {
            if (wasConnected)
            {
                weatherServiceReady = false;
                tuyaServiceReady = false;
            }
            wasConnected = false;
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        wasConnected = true;
        if (timeSynchronized && networkServicesRefreshPending)
        {
            networkServicesRefreshPending = false;
            lastWeatherUpdate = millis() - WEATHER_UPDATE_INTERVAL;
            lastTuyaUpdate = millis() - TUYA_UPDATE_INTERVAL;
        }

        if (timeSynchronized)
        {
            unsigned long now = millis();

            if (now - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL)
            {
                lastWeatherUpdate = now;
                setStartupStatus(STARTUP_WEATHER, STARTUP_CHECKING);
                if (weather.update())
                {
                    weatherServiceReady = true;
                    weatherServiceChecked = true;
                    weatherUpdatePending = true;
                    setStartupStatus(STARTUP_WEATHER, STARTUP_OK);
                }
                else
                {
                    weatherServiceReady = false;
                    weatherServiceChecked = true;
                    setStartupStatus(STARTUP_WEATHER, STARTUP_ERROR);
                }
            }

            now = millis();
            if (now - lastTuyaUpdate >= TUYA_UPDATE_INTERVAL)
            {
                lastTuyaUpdate = now;
                setStartupStatus(STARTUP_TUYA, STARTUP_CHECKING);
                tuyaServiceReady = tuyaService.update();
                tuyaServiceChecked = true;
                tuyaUpdatePending = true;
                setStartupStatus(
                    STARTUP_TUYA,
                    tuyaServiceReady ? STARTUP_OK : STARTUP_ERROR
                );
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ============================================================
// SETUP
// ============================================================
void setup()
{
    Serial.begin(115200);
    delay(1000);
    WiFi.onEvent(WiFiEvent);
    loadSettings();
    if (settingPIR)
{
    initPIR();
    lastMotionTime = millis();
}
else
{
    Serial.println("PIR disabled by settings");
}
screenSaverDelay = screenSaverMinutes * 60UL * 1000UL;
// --------------------------------------------------------
// SENSORS
// --------------------------------------------------------

Serial.println("========================");
    Serial.println();
    Serial.println("==============================");
    Serial.println("ILI9488 CLOCK");
    Serial.println("==============================");
    // --------------------------------------------------------
    // TFT
    // --------------------------------------------------------
tft.init();
tft.setRotation(1);

SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, -1);
touch.begin();
touch.setRotation(1);

tft.fillScreen(COLOR_BG);

initScreenSaver(&tft);

runStartup(&tft);
startupStartedAt = millis();

// TOUCH - kontrollo sinjalin PEN
Serial.printf("IRQ GPIO14 = %d\n", digitalRead(TOUCH_IRQ));
if (touch.touched())
{
    TS_Point p = touch.getPoint();

    Serial.printf(
        "PEN=TOUCHED | X=%d | Y=%d | Z=%d\n",
        p.x, p.y, p.z
    );
}
else
{
    static unsigned long lastTouchCheck = 0;

    if (millis() - lastTouchCheck >= 2000)
    {
        lastTouchCheck = millis();
        Serial.println("PEN=NOT TOUCHED");
    }
}
// --------------------------------------------------------
// WIFI MANAGER
// --------------------------------------------------------

Serial.println("Starting WiFi Manager...");
setStartupStatus(STARTUP_WIFI, STARTUP_CHECKING);

bool wifiConnected = wifiManager.begin(
    "",
    "",
    20000
);

if (wifiConnected)
{
    setStartupStatus(STARTUP_WIFI, STARTUP_OK);
    setStartupStatus(STARTUP_NTP, STARTUP_CHECKING);
    setStartupStatus(STARTUP_WEATHER, STARTUP_WAITING);
    setStartupStatus(STARTUP_TUYA, STARTUP_BACKGROUND);

    Serial.println("WiFi Manager: CONNECTED");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
        Serial.print("WiFi RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    webServer.setWeatherService(&weather);
    webServer.begin();

    configTzTime(
        "CET-1CEST,M3.5.0,M10.5.0/3",
        NTP_SERVER,
        "time.nist.gov",
        "time.google.com"
    );

    ntpStartedAt = millis();
    Serial.println("NTP sync running in background");
}
else
{
    setStartupStatus(STARTUP_WIFI, STARTUP_PORTAL);
    setStartupStatus(STARTUP_NTP, STARTUP_SKIPPED);
    setStartupStatus(STARTUP_WEATHER, STARTUP_SKIPPED);
    setStartupStatus(STARTUP_TUYA, STARTUP_SKIPPED);
    Serial.println("WiFi Manager: PORTAL ACTIVE");
}

initSensors();
setStartupStatus(
    STARTUP_BME680,
    bmeReadingOK ? STARTUP_OK : STARTUP_ERROR
);
setStartupStatus(
    STARTUP_BH1750,
    bh1750ReadingOK ? STARTUP_OK : STARTUP_ERROR
);

lastLowerPanelChange = millis();

    BaseType_t taskCreated = xTaskCreate(
        networkServicesTask,
        "NetworkServices",
        12288,
        nullptr,
        1,
        nullptr
    );

    if (taskCreated != pdPASS)
    {
        Serial.println("ERROR: failed to start network services task");
        setStartupStatus(STARTUP_WEATHER, STARTUP_ERROR);
        setStartupStatus(STARTUP_TUYA, STARTUP_ERROR);
    }
}
// ============================================================
// LOWER PANEL UPDATE
// ============================================================
void updateLowerPanel()
{
    int currentPanel = showingTuyaData ? 2 : (showingLocalSensors ? 1 : 0);
    bool enabled[3] = {
        settingWeather,
        settingLightSensor,
        settingTuyaDisplay
    };
    bool anyEnabled = enabled[0] || enabled[1] || enabled[2];

    if (!anyEnabled)
    {
        if (lowerPanelHasContent)
            tft.fillRect(75, WEATHER_Y, 405, 75, COLOR_BG);
        lowerPanelHasContent = false;
        showingLocalSensors = false;
        showingTuyaData = false;
        return;
    }

    if (!enabled[currentPanel])
    {
        int nextPanel = 0;
        while (!enabled[nextPanel])
            ++nextPanel;
        showingLocalSensors = nextPanel == 1;
        showingTuyaData = nextPanel == 2;
        renderLowerPanel();
        return;
    }

    if (!lowerPanelHasContent)
    {
        renderLowerPanel();
        return;
    }

    int enabledCount = (int)enabled[0] + (int)enabled[1] + (int)enabled[2];
    if (enabledCount > 1)
    {
        unsigned long displayTime = currentPanel == 1
            ? localSensorsDisplayTime
            : (currentPanel == 2 ? tuyaDisplayTime : weatherDisplayTime);
        if (millis() - lastLowerPanelChange >= displayTime)
        {
            int nextPanel = currentPanel;
            do
            {
                nextPanel = (nextPanel + 1) % 3;
            } while (!enabled[nextPanel]);

            showingLocalSensors = nextPanel == 1;
            showingTuyaData = nextPanel == 2;
            renderLowerPanel();
        }
    }
}
// ============================================================
// LOOP
// ============================================================
void loop()
{
    wifiManager.handleClient();

if (wifiGotIpPending)
{
    wifiGotIpPending = false;
    if (ntpStartedAt != 0)
    {
        configTzTime(
            "CET-1CEST,M3.5.0,M10.5.0/3",
            NTP_SERVER,
            "time.nist.gov",
            "time.google.com"
        );
        networkServicesRefreshPending = true;
        Serial.println("WiFi restored; NTP and network services refreshed.");
    }
}

if (wifiManager.isConnected())
{
    webServer.handleClient();
}

if (!timeSynchronized && wifiManager.isConnected() &&
    getSyncedTime(timeinfo))
{
    timeSynchronized = true;
    setStartupStatus(STARTUP_NTP, STARTUP_OK);
    setStartupStatus(STARTUP_WEATHER, STARTUP_WAITING);
    setStartupStatus(STARTUP_TUYA, STARTUP_BACKGROUND);
    lastWeatherUpdate = millis() - WEATHER_UPDATE_INTERVAL;
    lastTuyaUpdate = millis() - TUYA_UPDATE_INTERVAL;
    Serial.println("Time synchronized");
    Serial.printf(
        "Current time: %02d:%02d:%02d\n",
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
    );
}

if (startupActive)
{
    if (!timeSynchronized && ntpStartedAt != 0 &&
        millis() - ntpStartedAt >= 30000UL)
    {
        setStartupStatus(STARTUP_NTP, STARTUP_ERROR);
        setStartupStatus(STARTUP_WEATHER, STARTUP_SKIPPED);
        setStartupStatus(STARTUP_TUYA, STARTUP_SKIPPED);
    }

    refreshStartup();

    if (isStartupComplete())
    {
        if (startupResultsAt == 0)
            startupResultsAt = millis();

        if (millis() - startupResultsAt >= 5000UL)
        {
            startupActive = false;
            closeMenu();
            return;
        }
    }
    else
        startupResultsAt = 0;

    if (startupActive)
        return;
}

 // --------------------------------------------------------
 // TOUCH - HAP MENU DHE ABOUT
 // --------------------------------------------------------
static bool touchWasPressed = false;
static unsigned long lastMenuTouch = 0;

bool touchPressed = touch.touched();

if (touchPressed && !touchWasPressed)
{
    TS_Point p = touch.getPoint();

    Serial.printf(
        "TOUCH X=%d Y=%d Z=%d\n",
        p.x, p.y, p.z
    );

    if (p.z > 300 &&
        millis() - lastMenuTouch > 500)
    {
        const ScreenMode touchedScreen = screenMode;

        if (touchedScreen == SCREEN_CLOCK &&
            p.x >= 3300 && p.x <= 4000 &&
            p.y >= 2700 && p.y <= 3500)
        {
            lastMenuTouch = millis();
            Serial.println("TOUCH: OPEN MENU");
            openMenu();
        }
        else if (touchedScreen == SCREEN_MENU &&
                 p.x >= 1800 && p.x <= 2250 &&
                 p.y >= 2300 && p.y <= 2800)
        {
            lastMenuTouch = millis();
            setting24Hour = !setting24Hour;

            preferences.begin("ora", false);
            preferences.putBool("24hour", setting24Hour);
            preferences.end();

            Serial.printf(
                "TIME FORMAT: %s\n",
                setting24Hour ? "24H" : "12H"
            );

            drawMainMenu();
        }
        else if (touchedScreen == SCREEN_MENU &&
                 p.x >= 1800 && p.x <= 2250 &&
                 p.y >= 1600 && p.y <= 2050)
        {
            lastMenuTouch = millis();
            settingDate = !settingDate;

            preferences.begin("ora", false);
            preferences.putBool("date", settingDate);
            preferences.end();

            Serial.printf(
                "DATE: %s\n",
                settingDate ? "ON" : "OFF"
            );

            drawMainMenu();
        }
        else if (touchedScreen == SCREEN_MENU &&
                 p.x >= 1800 && p.x <= 2250 &&
                 p.y >= 1050 && p.y <= 1500)
        {
            lastMenuTouch = millis();
            Serial.println("TOUCH: OPEN ABOUT");
            openAbout();
        }
        else if (touchedScreen == SCREEN_MENU &&
                 p.x >= 1800 && p.x <= 2250 &&
                 p.y >= 500 && p.y <= 950)
        {
            lastMenuTouch = millis();
            Serial.println("TOUCH: WEATHER ICON PREVIEW");
            openWeatherIconPreview();
        }
        else if (touchedScreen == SCREEN_ICON_PREVIEW)
        {
            lastMenuTouch = millis();
            Serial.println("TOUCH: CLOSE WEATHER ICON PREVIEW");
            closeMenu();
        }
        else if (touchedScreen == SCREEN_ABOUT &&
                 p.z > 500 &&
                 p.x >= 1600 && p.x <= 2550 &&
                 p.y >= 500 && p.y <= 800)
        {
            lastMenuTouch = millis();
            Serial.println("TOUCH: BACK TO CLOCK");
            closeMenu();
        }
    }
}

touchWasPressed = touchPressed;
// --------------------------------------------------------
// FUNDI TOUCH
// --------------------------------------------------------
if (screenMode == SCREEN_ICON_PREVIEW)
    return;

// --------------------------------------------------------
// PIR / MOTION
// --------------------------------------------------------
if (settingPIR)
{
    pirState = readPIR();
}
else
{
    pirState = LOW;
}
// --------------------------------------------------------
// SENSOR READ
// --------------------------------------------------------
if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL)
{
    lastSensorRead = millis();
    updateLocalSensors();
}
// --------------------------------------------------------
// SCREEN SAVER WAKE
// --------------------------------------------------------

if (screenSaverActive && !nightModeActive)
{
    // Fillimisht presim që PIR të kthehet LOW.
    // Vetëm pas LOW PIR-i armatoset për wake.
    if (!screenSaverPIRArmed)
    {
        if (pirState == LOW)
        {
            screenSaverPIRArmed = true;
            Serial.println("SCREEN SAVER: PIR ARMED");
        }
    }
    else if (pirState == HIGH && pirLastState == LOW)
    {
        unsigned long now = millis();

        screenSaverActive = false;

        Serial.println("PIR: MOTION DETECTED");
        Serial.println("SCREEN SAVER: WAKE");
        if (settingGreeting)
{
    tft.fillScreen(COLOR_BG);

    tft.setTextColor(TFT_CYAN, COLOR_BG);
    tft.setTextFont(4);
    tft.setTextSize(3);
    tft.setTextDatum(MC_DATUM);

    tft.drawString(
        "MIRE SE VJEN",
        MAIN_CONTENT_CENTER_X,
        160
    );

    delay(2000);
    tft.setTextSize(1);

    tft.setTextDatum(TL_DATUM);

    tft.fillScreen(COLOR_BG);
}

        // Injoro PIR per 2 sekonda pas wake
        pirIgnoreUntil = millis() + 1000;

        // Pastro ekranin
        tft.fillScreen(COLOR_BG);

        // Detyro ridizenjimin e shifrave
        resetClockDisplay();

        if (getSyncedTime(timeinfo))
        {
            updateClockDisplay(tft, timeinfo, setting24Hour);

            if (settingDate)
            {
                updateDateDisplay(tft, timeinfo);
            }

            bool visible = (timeinfo.tm_sec % 2 == 0);
            drawClockColon(tft, visible);
        }

        // Rifresko elementet e ekranit
        updateLowerPanel();
        drawMenuIcon();
        updateNetworkIndicators(true);

        lastMotionTime = now;

        // Ruaj gjendjen aktuale te PIR
        pirLastState = HIGH;

        return;
    }
}
// --------------------------------------------------------
// NORMAL PIR MOTION
// --------------------------------------------------------
if (!screenSaverActive &&
    pirState == HIGH &&
    pirLastState == LOW)
{
    unsigned long now = millis();
    Serial.println("PIR: MOTION DETECTED");
    if (millis() >= pirIgnoreUntil)
    {
        if (now - lastMotionTime >= WELCOME_DELAY)
        {
            Serial.println("PIR: WELCOME EVENT");
        }
        lastMotionTime = now;
    }
}
// Ruaj gjendjen aktuale
pirLastState = pirState;
    // MENU / ABOUT SCREEN
    if (screenMode == SCREEN_ABOUT)
    {
        if (millis() - aboutStartTime >= ABOUT_TIMEOUT)
        {
            screenMode = SCREEN_CLOCK;

            tft.fillScreen(COLOR_BG);

            resetClockDisplay();

            if (getSyncedTime(timeinfo))
            {
                updateClockDisplay(tft, timeinfo, setting24Hour);

                if (settingDate)
                    updateDateDisplay(tft, timeinfo);

                bool visible = (timeinfo.tm_sec % 2 == 0);
                drawClockColon(tft, visible);
            }

            updateLowerPanel();
            drawMenuIcon();
            updateNetworkIndicators(true);
        }
         return;
    }
     if (screenMode == SCREEN_MENU)
{
    if (millis() - menuStartTime >= MENU_TIMEOUT)
    {
        Serial.println("MENU: AUTO CLOSE");
        closeMenu();
    }
return;
}
// NIGHT MODE
if (settingNightMode &&
    bh1750OK &&
    !nightModeActive &&
    bh1750Lux < nightSleepLux)
{
    nightModeActive = true;

    Serial.println("NIGHT MODE: ACTIVE");
    Serial.print("Lux: ");
    Serial.println(bh1750Lux);

    tft.fillScreen(TFT_BLACK);
}
    // --------------------------------------------------------
    // SCREEN SAVER
    // --------------------------------------------------------
   if (!screenSaverActive &&
    settingScreenSaver &&
    millis() - lastMotionTime >= screenSaverDelay)
    {
        screenSaverActive = true;
        screenSaverStart = millis();
        screenSaverPIRArmed = false;
        Serial.println("SCREEN SAVER: ACTIVE");
        tft.fillScreen(COLOR_BG);
    }
    // --------------------------------------------------------
// NIGHT MODE ACTIVE
// --------------------------------------------------------
if (nightModeActive)
{
    if (!settingNightMode ||
        (bh1750OK && bh1750Lux >= nightWakeLux))
    {
        nightModeActive = false;

        Serial.println("NIGHT MODE: WAKE");
        Serial.print("Lux: ");
        Serial.println(bh1750Lux);

        tft.fillScreen(COLOR_BG);

        resetClockDisplay();

        if (getSyncedTime(timeinfo))
        {
            updateClockDisplay(tft, timeinfo, setting24Hour);

            if (settingDate)
            {
                updateDateDisplay(tft, timeinfo);
            }

            bool visible = (timeinfo.tm_sec % 2 == 0);
            drawClockColon(tft, visible);
        }

        updateLowerPanel();
        drawMenuIcon();
        updateNetworkIndicators(true);

        lastMotionTime = millis();
    }
}
else if (screenSaverActive)
{
    updateScreenSaver(true);
}
else
{
        // ----------------------------------------------------
        // CLOCK
        // ----------------------------------------------------
        if (getSyncedTime(timeinfo))
        {
            updateClockDisplay(tft, timeinfo, setting24Hour);
            if (settingDate)
        {
         updateDateDisplay(tft, timeinfo);
        }
            static int lastSecond = -1;
            if (timeinfo.tm_sec != lastSecond)
            {
                lastSecond = timeinfo.tm_sec;
                bool visible =
                    (timeinfo.tm_sec % 2 == 0);
                drawClockColon(tft, visible);
            }
        }
        // ----------------------------------------------------
        // WIFI   INDICATOR
        // ----------------------------------------------------
        updateNetworkIndicators();
        // ----------------------------------------------------
        // WEATHER
        // --------------------------------------------------------
        if (weatherUpdatePending)
        {
            weatherUpdatePending = false;
            if (!showingLocalSensors && !showingTuyaData && settingWeather)
                drawWeatherDisplay(tft, weather.getData());
        }
        if (tuyaUpdatePending)
        {
            tuyaUpdatePending = false;
            if (showingTuyaData && settingTuyaDisplay)
                updateTuyaDisplay();
        }
        // ----------------------------------------------------
        // LOWER PANEL
        // ----------------------------------------------------
        updateLowerPanel();
       // ----------------------------------------------------
updateWeatherAnimations(
    tft,
    weather.getData(),
    screenMode == SCREEN_CLOCK &&
        !showingLocalSensors && !showingTuyaData
);
    }
}
