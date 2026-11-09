#include "WeatherDisplay.h"
#include "DisplayText.h"

namespace
{
constexpr int WEATHER_Y = 245;
constexpr uint16_t COLOR_BG = TFT_BLACK;
unsigned long lastRainAnimation = 0;
int rainFrame = 0;
unsigned long lastSnowAnimation = 0;
int snowFrame = 0;
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
static void drawCloud(
    TFT_eSPI& display,
    int centerX,
    int centerY,
    int verticalOffset = 0,
    int horizontalOffset = 0
)
{
    const int cx = centerX + horizontalOffset;
    const int cy = centerY + verticalOffset;

    display.fillCircle(cx - 10, cy + 4, 10, TFT_DARKGREY);
    display.fillCircle(cx, cy - 2, 14, TFT_DARKGREY);
    display.fillCircle(cx + 14, cy + 4, 9, TFT_DARKGREY);
    display.fillRect(cx - 10, cy + 4, 24, 10, TFT_DARKGREY);

    display.fillCircle(cx - 10, cy + 3, 8, TFT_LIGHTGREY);
    display.fillCircle(cx, cy - 2, 12, TFT_LIGHTGREY);
    display.fillCircle(cx + 14, cy + 3, 7, TFT_LIGHTGREY);
    display.fillRect(cx - 10, cy + 3, 24, 10, TFT_LIGHTGREY);
}

static void drawSmallCloud(TFT_eSPI& display, int cx, int cy)
{
    display.fillCircle(cx - 5, cy + 2, 6, TFT_DARKGREY);
    display.fillCircle(cx + 1, cy - 2, 8, TFT_DARKGREY);
    display.fillCircle(cx + 8, cy + 2, 5, TFT_DARKGREY);
    display.fillRect(cx - 5, cy + 2, 13, 6, TFT_DARKGREY);

    display.fillCircle(cx - 5, cy + 1, 5, TFT_LIGHTGREY);
    display.fillCircle(cx + 1, cy - 2, 6, TFT_LIGHTGREY);
    display.fillCircle(cx + 8, cy + 1, 4, TFT_LIGHTGREY);
    display.fillRect(cx - 5, cy + 1, 13, 6, TFT_LIGHTGREY);
}

static void drawCloudGroup(
    TFT_eSPI& display,
    int cx,
    int cy,
    int verticalOffset = 0
)
{
    drawCloud(display, cx, cy, verticalOffset - 4, -3);
    drawSmallCloud(display, cx + 10, cy + verticalOffset + 4);
}

static void drawSun(TFT_eSPI& display, int cx, int cy, int rayLength = 17)
{
    display.fillCircle(cx, cy, 10, TFT_YELLOW);

    display.drawLine(cx, cy - 11, cx, cy - rayLength, TFT_YELLOW);
    display.drawLine(cx, cy + 11, cx, cy + rayLength, TFT_YELLOW);
    display.drawLine(cx - 11, cy, cx - rayLength, cy, TFT_YELLOW);
    display.drawLine(cx + 11, cy, cx + rayLength, cy, TFT_YELLOW);
    display.drawLine(cx - 8, cy - 8, cx - rayLength + 5, cy - rayLength + 5, TFT_YELLOW);
    display.drawLine(cx + 8, cy - 8, cx + rayLength - 5, cy - rayLength + 5, TFT_YELLOW);
    display.drawLine(cx - 8, cy + 8, cx - rayLength + 5, cy + rayLength - 5, TFT_YELLOW);
    display.drawLine(cx + 8, cy + 8, cx + rayLength - 5, cy + rayLength - 5, TFT_YELLOW);
}

static void drawMoon(TFT_eSPI& display, int cx, int cy)
{
    display.fillCircle(cx, cy, 9, TFT_YELLOW);
    display.fillCircle(cx + 5, cy - 4, 9, COLOR_BG);
    display.drawPixel(cx - 12, cy - 10, TFT_WHITE);
    display.drawPixel(cx - 15, cy - 3, TFT_WHITE);
}

static void drawSnowflake(TFT_eSPI& display, int cx, int cy)
{
    display.drawLine(cx - 3, cy, cx + 3, cy, TFT_WHITE);
    display.drawLine(cx, cy - 3, cx, cy + 3, TFT_WHITE);
    display.drawLine(cx - 2, cy - 2, cx + 2, cy + 2, TFT_WHITE);
    display.drawLine(cx + 2, cy - 2, cx - 2, cy + 2, TFT_WHITE);
}

// ============================================================
// WEATHER ICON
// ============================================================

static void drawWeatherIcon(
    TFT_eSPI& display,
    const WeatherData& data,
    int cx,
    int cy,
    int cloudVerticalOffset = 0
)
{
    int code = data.weatherCode;

    if (code == 0)
    {
        if (data.isDay)
            drawSun(display, cx, cy, 17 + sunFrame);
        else
            drawMoon(display, cx, cy);
    }
    else if (code == 1 || code == 2)
    {
        if (data.isDay)
            drawSun(display, cx, cy - 11, 15);
        else
            drawMoon(display, cx, cy - 11);

        drawCloudGroup(display, cx, cy, cloudVerticalOffset + 4);
    }
    else if (code == 3)
    {
        drawCloudGroup(display, cx, cy, cloudVerticalOffset);
    }

    // =========================
    // FOG
    // =========================
    else if (code >= 45 && code <= 48)
    {
        drawCloudGroup(display, cx, cy);
        display.drawLine(
            cx - 20, cy + 6,
            cx + 20, cy + 6,
            TFT_LIGHTGREY
        );

        display.drawLine(
            cx - 17, cy + 11,
            cx + 17, cy + 11,
            TFT_LIGHTGREY
        );

        display.drawLine(
            cx - 20, cy + 16,
            cx + 20, cy + 16,
            TFT_LIGHTGREY
        );
    }

    // =========================
    // RAIN
    // =========================
    else if (code >= 51 && code <= 67)
    {
        drawCloudGroup(display, cx, cy);

        const uint16_t rainColor = code <= 57 ? TFT_SKYBLUE : TFT_CYAN;
        display.drawLine(cx - 10, cy + 15, cx - 13, cy + 22, rainColor);
        display.drawLine(cx, cy + 15, cx - 3, cy + 22, rainColor);
        display.drawLine(cx + 10, cy + 15, cx + 7, cy + 22, rainColor);
    }

    // =========================
    // SNOW
    // =========================
    else if (code >= 71 && code <= 77)
    {
        drawCloudGroup(display, cx, cy);
        drawSnowflake(display, cx - 9, cy + 19);
        drawSnowflake(display, cx + 2, cy + 23);
        drawSnowflake(display, cx + 12, cy + 18);
    }

    // =========================
    // SHOWERS
    // =========================
    else if (code >= 80 && code <= 82)
    {
        if (data.isDay)
            drawSun(display, cx, cy - 11, 15);
        else
            drawMoon(display, cx, cy - 11);

        drawCloudGroup(display, cx, cy);
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
        if (data.isDay)
            drawSun(display, cx, cy - 11, 15);
        else
            drawMoon(display, cx, cy - 11);

        drawCloudGroup(display, cx, cy);
        drawSnowflake(display, cx - 8, cy + 19);
        drawSnowflake(display, cx + 3, cy + 23);
        drawSnowflake(display, cx + 13, cy + 18);
    }

    // =========================
    // THUNDERSTORM
    // =========================
    else if (code >= 95 && code <= 99)
    {
        drawCloudGroup(display, cx, cy);

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

void drawWeatherIconGallery(TFT_eSPI& display)
{
    static const int codes[] = {0, 0, 2, 2, 3, 45, 51, 61, 71, 80, 85, 95};
    static const bool dayFlags[] = {
        true, false, true, false, true, true,
        true, true, true, true, true, true
    };
    static const char* const labels[] = {
        "Sunny", "Clear night", "Partly day", "Partly night",
        "Cloudy", "Fog", "Drizzle", "Rain", "Snow", "Showers",
        "Snow shower", "Storm"
    };

    display.fillScreen(COLOR_BG);
    display.setTextDatum(MC_DATUM);
    display.setTextFont(2);
    display.setTextColor(TFT_CYAN, COLOR_BG);
    display.drawString("WEATHER ICON PREVIEW", 240, 18);

    for (int i = 0; i < 12; ++i)
    {
        const int column = i % 2;
        const int row = i / 2;
        const int iconX = column == 0 ? 44 : 274;
        const int iconY = 54 + row * 44;
        WeatherData sample{};
        sample.valid = true;
        sample.weatherCode = codes[i];
        sample.isDay = dayFlags[i];

        drawWeatherIcon(display, sample, iconX, iconY);

        display.setTextColor(TFT_WHITE, COLOR_BG);
        display.drawString(labels[i], column == 0 ? 112 : 350, iconY);
    }

    display.setTextFont(1);
    display.setTextColor(TFT_DARKGREY, COLOR_BG);
    display.drawString("Touch screen to return", 240, 309);
    display.setTextDatum(TL_DATUM);
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
    if (code != 0 || !data.isDay)
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
    if (data.weatherCode < 1 || data.weatherCode > 3)
        return;

    const int cx = 40;
    const int cy = WEATHER_Y + 37;
    display.fillRect(cx - 38, cy - 34, 76, 65, COLOR_BG);

    cloudFrame = (cloudFrame + 1) % 12;
    int verticalOffset = cloudFrame <= 3
        ? -cloudFrame
        : (cloudFrame <= 9 ? cloudFrame - 6 : 12 - cloudFrame);

    drawWeatherIcon(display, data, cx, cy, verticalOffset);
}

static void animateSnowflakes(TFT_eSPI& display, const WeatherData& data)
{
    const int code = data.weatherCode;
    if (!((code >= 71 && code <= 77) || (code >= 85 && code <= 86)))
        return;

    const int cx = 40;
    const int cy = WEATHER_Y + 37;
    display.fillRect(cx - 18, cy + 15, 40, 21, COLOR_BG);
    drawSnowflake(display, cx - 9, cy + 16 + (snowFrame % 5));
    drawSnowflake(display, cx + 2, cy + 16 + ((snowFrame + 2) % 5));
    drawSnowflake(display, cx + 12, cy + 16 + ((snowFrame + 4) % 5));
    snowFrame = (snowFrame + 1) % 5;
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

    constexpr uint8_t titleTextSize = 2;
    const char* title = "Moti në Sarandë";
    display.setTextFont(1);
    display.setTextSize(titleTextSize);
    display.setCursor(
        277 - albanianTextWidth(title, titleTextSize) / 2,
        WEATHER_Y
    );
    drawAlbanianText(display, title, titleTextSize);
    display.setTextSize(1);

    // Weather icon
    drawWeatherIcon(display, data, 40, WEATHER_Y + 37);

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
    if (data.weatherCode == 0 && data.isDay &&
        now - lastSunAnimation >= 180)
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
    if (((data.weatherCode >= 71 && data.weatherCode <= 77) ||
         (data.weatherCode >= 85 && data.weatherCode <= 86)) &&
        now - lastSnowAnimation >= 300)
    {
        lastSnowAnimation = now;
        animateSnowflakes(display, data);
    }

    now = millis();
    if (data.weatherCode >= 1 && data.weatherCode <= 3 &&
        now - lastCloudAnimation >= 500)
    {
        lastCloudAnimation = now;
        animateCloud(display, data);
    }
}
