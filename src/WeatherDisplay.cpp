#include "WeatherDisplay.h"

namespace
{
constexpr int WEATHER_Y = 245;
constexpr uint16_t COLOR_BG = TFT_BLACK;
unsigned long lastRainAnimation = 0;
int rainFrame = 0;
unsigned long lastSunAnimation = 0;
int sunFrame = 0;
unsigned long lastLightningAnimation = 0;
int lightningFrame = 0;
unsigned long lastCloudAnimation = 0;
int cloudFrame = 0;
}

// ============================================================
// WEATHER DISPLAY
// ============================================================
static void drawCloud(TFT_eSPI& display)
{
    const int cx = 40;
    const int cy = WEATHER_Y + 37;

    // Trupi i rrumbullakosur i resë
    display.fillCircle(cx - 9, cy + 4, 9, TFT_LIGHTGREY);
    display.fillCircle(cx + 3, cy - 2, 12, TFT_LIGHTGREY);
    display.fillCircle(cx + 14, cy + 5, 8, TFT_LIGHTGREY);

    // Mbushja e pjesës së poshtme
    display.fillRect(
        cx - 9,
        cy + 4,
        23,
        9,
        TFT_LIGHTGREY
    );

    // Rrumbullakosim skajet e poshtme
    display.fillCircle(cx - 9, cy + 9, 5, TFT_LIGHTGREY);
    display.fillCircle(cx + 14, cy + 9, 5, TFT_LIGHTGREY);
}
// ============================================================
// WEATHER ICON
// ============================================================

static void drawWeatherIcon(TFT_eSPI& display, const WeatherData& data)
{
    int cx = 40;
    int cy = WEATHER_Y + 37;
    int code = data.weatherCode;

    // =========================
    // CLEAR SKY
    // =========================
    if (code == 0)
    {
        display.fillCircle(
            cx,
            cy,
            10,
            TFT_YELLOW
        );

        // Rrezet
        display.drawLine(
            cx, cy - 17,
            cx, cy - 11,
            TFT_YELLOW
        );

        display.drawLine(
            cx, cy + 11,
            cx, cy + 17,
            TFT_YELLOW
        );

        display.drawLine(
            cx - 17, cy,
            cx - 11, cy,
            TFT_YELLOW
        );

        display.drawLine(
            cx + 11, cy,
            cx + 17, cy,
            TFT_YELLOW
        );

        display.drawLine(
            cx - 12, cy - 12,
            cx - 8, cy - 8,
            TFT_YELLOW
        );

        display.drawLine(
            cx + 8, cy - 8,
            cx + 12, cy - 12,
            TFT_YELLOW
        );

        display.drawLine(
            cx - 12, cy + 12,
            cx - 8, cy + 8,
            TFT_YELLOW
        );

        display.drawLine(
            cx + 8, cy + 8,
            cx + 12, cy + 12,
            TFT_YELLOW
        );
    }

    // =========================
    // CLOUDY
    // =========================
    else if (code >= 1 && code <= 3)
    {
       drawCloud(display);
    }

    // =========================
    // FOG
    // =========================
    else if (code >= 45 && code <= 48)
    {
        display.drawLine(
            cx - 18, cy - 8,
            cx + 18, cy - 8,
            TFT_LIGHTGREY
        );

        display.drawLine(
            cx - 22, cy,
            cx + 14, cy,
            TFT_LIGHTGREY
        );

        display.drawLine(
            cx - 18, cy + 8,
            cx + 18, cy + 8,
            TFT_LIGHTGREY
        );
    }

    // =========================
    // RAIN
    // =========================
    else if (code >= 51 && code <= 67)
{
    drawCloud(display);

        // Pikat e shiut
        display.drawLine(
            cx - 10, cy + 15,
            cx - 13, cy + 23,
            TFT_CYAN
        );

        display.drawLine(
            cx, cy + 15,
            cx - 3, cy + 23,
            TFT_CYAN
        );

        display.drawLine(
            cx + 10, cy + 15,
            cx + 7, cy + 23,
            TFT_CYAN
        );
    }

    // =========================
    // SNOW
    // =========================
    else if (code >= 71 && code <= 77)
    {
        // Re
        display.fillCircle(
            cx - 9,
            cy - 1,
            9,
            TFT_LIGHTGREY
        );

        display.fillCircle(
            cx + 3,
            cy - 6,
            12,
            TFT_LIGHTGREY
        );

        display.fillCircle(
            cx + 14,
            cy,
            8,
            TFT_LIGHTGREY
        );

        display.fillRect(
            cx - 17,
            cy,
            39,
            11,
            TFT_LIGHTGREY
        );

        // Floket
        display.fillCircle(
            cx - 9,
            cy + 19,
            3,
            TFT_WHITE
        );

        display.fillCircle(
            cx + 2,
            cy + 23,
            3,
            TFT_WHITE
        );

        display.fillCircle(
            cx + 12,
            cy + 18,
            3,
            TFT_WHITE
        );
    }

    // =========================
    // SHOWERS
    // =========================
    else if (code >= 80 && code <= 82)
    {
       drawCloud(display);

        // Shi
        display.drawLine(
            cx - 9, cy + 15,
            cx - 12, cy + 23,
            TFT_CYAN
        );

        display.drawLine(
            cx + 1, cy + 15,
            cx - 2, cy + 23,
            TFT_CYAN
        );

        display.drawLine(
            cx + 11, cy + 15,
            cx + 8, cy + 23,
            TFT_CYAN
        );
    }

    // =========================
    // SNOW SHOWERS
    // =========================
    else if (code >= 85 && code <= 86)
    {
        display.fillCircle(
            cx - 9,
            cy - 1,
            9,
            TFT_LIGHTGREY
        );

        display.fillCircle(
            cx + 3,
            cy - 6,
            12,
            TFT_LIGHTGREY
        );

        display.fillCircle(
            cx + 14,
            cy,
            8,
            TFT_LIGHTGREY
        );

        display.fillRect(
            cx - 17,
            cy,
            39,
            11,
            TFT_LIGHTGREY
        );

        display.fillCircle(
            cx - 8,
            cy + 19,
            3,
            TFT_WHITE
        );

        display.fillCircle(
            cx + 3,
            cy + 23,
            3,
            TFT_WHITE
        );

        display.fillCircle(
            cx + 13,
            cy + 18,
            3,
            TFT_WHITE
        );
    }

    // =========================
    // THUNDERSTORM
    // =========================
    else if (code >= 95 && code <= 99)
    {drawCloud(display);

        // Rrufe
        display.fillTriangle(
            cx + 1, cy + 10,
            cx - 6, cy + 22,
            cx + 1, cy + 19,
            TFT_YELLOW
        );

        display.fillTriangle(
            cx + 1, cy + 19,
            cx + 8, cy + 19,
            cx + 1, cy + 29,
            TFT_YELLOW
        );
    }

    // =========================
    // UNKNOWN
    // =========================
    else
    {
        display.drawCircle(
            cx,
            cy,
            12,
            TFT_WHITE
        );

        display.drawLine(
            cx - 6,
            cy,
            cx + 6,
            cy,
            TFT_WHITE
        );
    }
}

// ============================================================
// WEATHER DATA DISPLAY
// ============================================================
static void animateRainDrops(TFT_eSPI& display, const WeatherData& data)
{
    int code = data.weatherCode;

    // Vetëm shi dhe showers
    if (!((code >= 51 && code <= 67) ||
          (code >= 80 && code <= 82)))
    {
        return;
    }

    const int cx = 40;
    const int cy = WEATHER_Y + 37;

    // Pastro vetëm zonën e pikave të shiut
    display.fillRect(
        cx - 18,
        cy + 15,
        40,
        21,
        COLOR_BG
    );

    int drop1 = rainFrame;
    int drop2 = (rainFrame + 4) % 12;
    int drop3 = (rainFrame + 8) % 12;

    display.drawLine(
        cx - 9,
        cy + 15 + drop1,
        cx - 12,
        cy + 21 + drop1,
        TFT_CYAN
    );

    display.drawLine(
        cx + 1,
        cy + 15 + drop2,
        cx - 2,
        cy + 21 + drop2,
        TFT_CYAN
    );

    display.drawLine(
        cx + 11,
        cy + 15 + drop3,
        cx + 8,
        cy + 21 + drop3,
        TFT_CYAN
    );

    rainFrame++;

    if (rainFrame >= 12)
        rainFrame = 0;
}
static void animateSun(TFT_eSPI& display, const WeatherData& data)
{
    int code = data.weatherCode;

    // Vetëm clear sky
    if (code != 0)
        return;

    const int cx = 40;
    const int cy = WEATHER_Y + 37;

    // Gjatësia e rrezes së vjetër
    int oldOuter = 17 + sunFrame;

    // Fshi vetëm rrezet e vjetra
    display.drawLine(cx, cy - 12, cx, cy - oldOuter, COLOR_BG);
    display.drawLine(cx, cy + 12, cx, cy + oldOuter, COLOR_BG);

    display.drawLine(cx - 12, cy, cx - oldOuter, cy, COLOR_BG);
    display.drawLine(cx + 12, cy, cx + oldOuter, cy, COLOR_BG);

    display.drawLine(cx - 8, cy - 8,
                 cx - oldOuter + 5, cy - oldOuter + 5, COLOR_BG);

    display.drawLine(cx + 8, cy - 8,
                 cx + oldOuter - 5, cy - oldOuter + 5, COLOR_BG);

    display.drawLine(cx - 8, cy + 8,
                 cx - oldOuter + 5, cy + oldOuter - 5, COLOR_BG);

    display.drawLine(cx + 8, cy + 8,
                 cx + oldOuter - 5, cy + oldOuter - 5, COLOR_BG);

    // Kalo në frame tjetër
    sunFrame++;

    if (sunFrame > 2)
        sunFrame = 0;

    int newOuter = 17 + sunFrame;

    // Vizato vetëm rrezet e reja
    display.drawLine(cx, cy - 12, cx, cy - newOuter, TFT_YELLOW);
    display.drawLine(cx, cy + 12, cx, cy + newOuter, TFT_YELLOW);

    display.drawLine(cx - 12, cy, cx - newOuter, cy, TFT_YELLOW);
    display.drawLine(cx + 12, cy, cx + newOuter, cy, TFT_YELLOW);

    display.drawLine(cx - 8, cy - 8,
                 cx - newOuter + 5, cy - newOuter + 5, TFT_YELLOW);

    display.drawLine(cx + 8, cy - 8,
                 cx + newOuter - 5, cy - newOuter + 5, TFT_YELLOW);

    display.drawLine(cx - 8, cy + 8,
                 cx - newOuter + 5, cy + newOuter - 5, TFT_YELLOW);

    display.drawLine(cx + 8, cy + 8,
                 cx + newOuter - 5, cy + newOuter - 5, TFT_YELLOW);
}
static void animateLightning(TFT_eSPI& display, const WeatherData& data)
{
    int code = data.weatherCode;

    // Vetëm thunderstorm
    if (code < 95 || code > 99)
        return;

    const int cx = 40;
    const int cy = WEATHER_Y + 37;

    // Rrufeja pulson: një frame fikur, një frame ndezur
    if (lightningFrame == 0)
    {
        display.fillTriangle(
            cx + 1, cy + 10,
            cx - 6, cy + 22,
            cx + 1, cy + 19,
            COLOR_BG
        );

        display.fillTriangle(
            cx + 1, cy + 19,
            cx + 8, cy + 19,
            cx + 1, cy + 29,
            COLOR_BG
        );
    }
    else
    {
        display.fillTriangle(
            cx + 1, cy + 10,
            cx - 6, cy + 22,
            cx + 1, cy + 19,
            TFT_YELLOW
        );

        display.fillTriangle(
            cx + 1, cy + 19,
            cx + 8, cy + 19,
            cx + 1, cy + 29,
            TFT_YELLOW
        );
    }

    lightningFrame++;

    if (lightningFrame >= 2)
        lightningFrame = 0;
}
static void animateCloud(TFT_eSPI& display, const WeatherData& data)
{
    // Reja është statike.
    return;
}
void drawWeatherDisplay(TFT_eSPI& display, const WeatherData& data)
{
    if (!data.valid)
        return;

    // Clear weather data area
    display.fillRect(
        75,
        WEATHER_Y,
        405,
        75,
        COLOR_BG
    );

    // Titulli
    display.setTextColor(
        TFT_WHITE,
        COLOR_BG
    );

    display.setTextFont(2);
    display.setTextDatum(MC_DATUM);

    display.drawString(
        "Moti ne Sarande",
        277,
        WEATHER_Y + 7
    );

    display.setTextDatum(TL_DATUM);

    // Weather icon
    drawWeatherIcon(display, data);

    display.setTextColor(
        TFT_WHITE,
        COLOR_BG
    );

    display.setTextFont(4);

    // --------------------------------------------------------
    // TEMPERATURE
    // --------------------------------------------------------

    char tempText[20];

    snprintf(
        tempText,
        sizeof(tempText),
        "Temp: %.1f °C",
        data.temperature
    );

    display.setTextColor(
        TFT_ORANGE,
        COLOR_BG
    );

    display.setCursor(
        85,
        WEATHER_Y + 25
    );

    display.print(tempText);

    // --------------------------------------------------------
    // HUMIDITY
    // --------------------------------------------------------

    char humidityText[20];

    snprintf(
        humidityText,
        sizeof(humidityText),
        "Lag: %.0f%%",
        data.humidity
    );

    display.setTextColor(
        TFT_CYAN,
        COLOR_BG
    );

    display.setCursor(
        85,
        WEATHER_Y + 52
    );

    display.print(humidityText);

    // --------------------------------------------------------
    // WIND
    // --------------------------------------------------------

    char windText[20];

    snprintf(
        windText,
        sizeof(windText),
        "Era: %.1f km/h",
        data.windSpeed
    );

    display.setTextColor(
        TFT_GREEN,
        COLOR_BG
    );

    display.setCursor(
        285,
        WEATHER_Y + 25
    );

    display.print(windText);

    // --------------------------------------------------------
    // PRESSURE
    // --------------------------------------------------------

    char pressureText[20];

    snprintf(
        pressureText,
        sizeof(pressureText),
        "Pres: %.0fhPa",
        data.pressure
    );

    display.setTextColor(
        TFT_MAGENTA,
        COLOR_BG
    );

    display.setCursor(
        285,
        WEATHER_Y + 52
    );

    display.print(pressureText);
}

void updateWeatherAnimations(
    TFT_eSPI& display,
    const WeatherData& data,
    bool weatherPanelVisible
)
{
    if (!weatherPanelVisible || !data.valid)
        return;

    unsigned long now = millis();
    if (now - lastRainAnimation >= 180)
    {
        lastRainAnimation = now;
        animateRainDrops(display, data);
    }

    now = millis();
    if (data.weatherCode == 0 && now - lastSunAnimation >= 180)
    {
        lastSunAnimation = now;
        animateSun(display, data);
    }

    now = millis();
    if (data.weatherCode >= 95 && data.weatherCode <= 99 &&
        now - lastLightningAnimation >= 300)
    {
        lastLightningAnimation = now;
        animateLightning(display, data);
    }

    now = millis();
    if (data.weatherCode >= 1 && data.weatherCode <= 3 &&
        now - lastCloudAnimation >= 500)
    {
        lastCloudAnimation = now;
        animateCloud(display, data);
    }
}
