#include "AboutScreen.h"

void drawAboutScreen(TFT_eSPI& tft, const AboutScreenData& data)
{
    const uint16_t backgroundColor = tft.color565(5, 10, 16);
    const uint16_t headerColor = tft.color565(10, 20, 29);
    const uint16_t panelColor = tft.color565(14, 24, 34);
    const uint16_t borderColor = tft.color565(35, 57, 70);
    const uint16_t mutedColor = tft.color565(137, 158, 171);
    const uint16_t accentColor = tft.color565(0, 220, 245);

    tft.fillScreen(backgroundColor);

    tft.fillRect(0, 0, 480, 47, headerColor);
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(4);
    tft.setTextSize(1);
    tft.setTextColor(accentColor, headerColor);
    tft.drawString("ORA E LAPIT", 14, 10);
    tft.setTextFont(1);
    tft.setTextColor(mutedColor, headerColor);
    tft.drawRightString("VERSION 1.1.7", 466, 19, 1);
    tft.drawFastHLine(12, 47, 456, borderColor);

    auto drawCard = [&](int x, int y, int w, int h, const char* title)
    {
        tft.fillRoundRect(x, y, w, h, 9, panelColor);
        tft.drawRoundRect(x, y, w, h, 9, borderColor);
        tft.setTextFont(1);
        tft.setTextColor(accentColor, panelColor);
        tft.drawString(title, x + 10, y + 9);
    };

    auto drawRow = [&](int x, int y, int rightX, const char* label,
                       const String& value, uint16_t color)
    {
        tft.setTextFont(1);
        tft.setTextColor(mutedColor, panelColor);
        tft.drawString(label, x, y);
        tft.setTextColor(color, panelColor);
        tft.drawRightString(value, rightX, y, 1);
    };

    drawCard(10, 55, 224, 105, "NETWORK");
    const String wifiSignal = data.wifiConnected
        ? String(data.wifiRssi) + " dBm"
        : "--";
    drawRow(20, 79, 224, "Wi-Fi", data.wifiConnected ? "CONNECTED" : "OFFLINE",
            data.wifiConnected ? TFT_GREEN : TFT_RED);
    drawRow(20, 96, 224, "IP ADDRESS", data.ipAddress, TFT_WHITE);
    drawRow(20, 113, 224, "SIGNAL", wifiSignal,
            data.wifiConnected ? TFT_WHITE : mutedColor);
    drawRow(20, 140, 224, "NTP CLOCK",
            data.timeSynchronized ? "SYNCHRONIZED" : "WAITING",
            data.timeSynchronized ? TFT_GREEN : TFT_YELLOW);

    drawCard(246, 55, 224, 105, "SENSORS");
    const String bmeValue = data.bmeAvailable
        ? String(data.bmeTemperature, 1) + " C  " + String(data.bmeHumidity, 0) + "%"
        : (data.bmeFound ? "NO READING" : "NOT FOUND");
    const String lightValue = data.lightSensorAvailable
        ? String(data.lightLevel, 0) + " lx"
        : (data.lightSensorFound ? "NO READING" : "NOT FOUND");
    drawRow(256, 79, 458, "BME680", bmeValue,
            data.bmeAvailable ? TFT_GREEN : TFT_RED);
    drawRow(256, 96, 458, "BH1750", lightValue,
            data.lightSensorAvailable ? TFT_GREEN : TFT_RED);
    drawRow(256, 113, 458, "PIR", data.pirEnabled ? "ENABLED" : "DISABLED",
            data.pirEnabled ? TFT_GREEN : mutedColor);
    drawRow(256, 140, 458, "LOCAL DISPLAY",
            data.localDisplayEnabled ? "ON" : "OFF",
            data.localDisplayEnabled ? TFT_GREEN : mutedColor);

    drawCard(10, 168, 460, 92, "INTERNET SERVICES");
    String weatherValue = data.weatherReady ? "CONNECTED" : "UNAVAILABLE";
    if (data.weatherValid)
        weatherValue += "  " + String(data.weatherTemperature, 1) + " C";
    drawRow(20, 192, 458, "OPEN-METEO", weatherValue,
            data.weatherReady ? TFT_GREEN : TFT_RED);

    const String tuyaValue = data.tuyaOnline
        ? String(data.tuyaTemperature, 1) + " C  " +
              String(data.tuyaHumidity, 0) + "%  " +
              String(data.tuyaBattery) + "% BATTERY"
        : "OFFLINE";
    drawRow(20, 210, 458, "TUYA CLOUD", tuyaValue,
            data.tuyaOnline ? TFT_GREEN : TFT_RED);

    const String displaySettings = String("WEATHER ") +
        (data.weatherDisplayEnabled ? "ON" : "OFF") +
        "   LOCAL " + (data.localDisplayEnabled ? "ON" : "OFF") +
        "   TUYA " + (data.tuyaDisplayEnabled ? "ON" : "OFF");
    drawRow(20, 238, 458, "LOWER PANEL", displaySettings, TFT_WHITE);

    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(1);
    tft.setTextColor(mutedColor, backgroundColor);
    tft.drawString("ESP32-S3  |  FREE RAM " + String(data.freeHeapKb) + " KB",
                   240, 270);
    const uint16_t buttonColor = tft.color565(0, 72, 87);
    tft.drawRoundRect(180, 275, 120, 32, 7, accentColor);
    tft.fillRoundRect(181, 276, 118, 30, 6, buttonColor);
    tft.setTextColor(TFT_WHITE, buttonColor);
    tft.drawString("BACK TO CLOCK", 240, 291);
    tft.setTextDatum(TL_DATUM);
}
