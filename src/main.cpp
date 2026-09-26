#include <Arduino.h>
#include <WiFi.h>
#include "WebServerManager.h"
#include "TuyaService.h"
#include "Pages.h"
#include "WeatherService.h"
#include "Sensors.h"
#include "Settings.h"
#include "ScreenSaver.h"
#include <TFT_eSPI.h>
#include <time.h>

// ============================================================
// WIFI
// ============================================================
const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";
const char* NTP_SERVER = "pool.ntp.org";
// ============================================================
// TFT
// ============================================================
TFT_eSPI tft = TFT_eSPI();
struct tm timeinfo;
WebServerManager webServer;
TuyaService tuyaService;
unsigned long lastTuyaUpdate = 0;
const unsigned long TUYA_UPDATE_INTERVAL = 60UL * 1000UL;
WeatherService weather;
int pirState = LOW;

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

// Emrat e diteve ne shqip
const char* days[] = {
    "E diel",
    "E hene",
    "E marte",
    "E merkure",
    "E enjte",
    "E premte",
    "E shtune"
};
// Emrat e muajve ne shqip
const char* months[] = {
    "Janar",
    "Shkurt",
    "Mars",
    "Prill",
    "Maj",
    "Qershor",
    "Korrik",
    "Gusht",
    "Shtator",
    "Tetor",
    "Nentor",
    "Dhjetor"
};
// ============================================================
// CLOCK GEOMETRY
// ============================================================
#define DIGIT_W       84
#define DIGIT_H       130
#define SEG_THICK     13
#define DIGIT_GAP     10
#define CLOCK_Y       25
const int COLON_X = 232;
// ============================================================
// 7-SEGMENT DIGIT MAP
// ============================================================
const bool digitSegments[10][7] =
{
    { true,  true,  true,  true,  true,  true,  false }, // 0
    { false, true,  true,  false, false, false, false }, // 1
    { true,  true,  false, true,  true,  false, true  }, // 2
    { true,  true,  true,  true,  false, false, true  }, // 3
    { false, true,  true,  false, false, true,  true  }, // 4
    { true,  false, true,  true,  false, true,  true  }, // 5
    { true,  false, true,  true,  true,  true,  true  }, // 6
    { true,  true,  true,  false, false, false, false }, // 7
    { true,  true,  true,  true,  true,  true,  true  }, // 8
    { true,  true,  true,  true,  false, true,  true  }  // 9
};
// ============================================================
// DIGIT POSITION
// ============================================================
int getDigitX(int index)
{
    const int startX = 43;
    if (index == 0)
        return startX;
    if (index == 1)
        return startX + DIGIT_W + DIGIT_GAP;
    if (index == 2)
        return startX + 2 * DIGIT_W + DIGIT_GAP + 32;
    if (index == 3)
        return startX + 3 * DIGIT_W + 2 * DIGIT_GAP + 32;
    return startX;
}
// ============================================================
// SEGMENTS - BEVELED / POINTED ENDS
// ============================================================
void drawHorizontalSegment(
    int x,
    int y,
    int w,
    int h,
    uint16_t color
)
{
    const int tip = 5;
    tft.fillRect(
        x + tip,
        y,
        w - 2 * tip,
        h,
        color
    );
    tft.fillTriangle(
        x,
        y + h / 2,
        x + tip,
        y,
        x + tip,
        y + h,
        color
    );
    tft.fillTriangle(
        x + w,
        y + h / 2,
        x + w - tip,
        y,
        x + w - tip,
        y + h,
        color
    );
}
void drawVerticalSegment(
    int x,
    int y,
    int w,
    int h,
    uint16_t color
)
{
    const int tip = 5;
    tft.fillRect(
        x,
        y + tip,
        w,
        h - 2 * tip,
        color
    );
    tft.fillTriangle(
        x + w / 2,
        y,
        x,
        y + tip,
        x + w,
        y + tip,
        color
    );
    tft.fillTriangle(
        x,
        y + h - tip,
        x + w,
        y + h - tip,
        x + w / 2,
        y + h,
        color
    );
}
// ============================================================
// DRAW DIGIT
// ============================================================
void drawDigit(int x, int y, int number)
{
    tft.fillRect(
        x - 2,
        y - 2,
        DIGIT_W + 4,
        DIGIT_H + 4,
        COLOR_BG
    );
    if (number < 0 || number > 9)
        return;
    const int verticalH = 52;
    const int topY     = y;
    const int middleY  = y + 59;
    const int bottomY  = y + DIGIT_H - SEG_THICK;
    const int leftX  = x;
    const int rightX = x + DIGIT_W - SEG_THICK;
    if (digitSegments[number][0])
        drawHorizontalSegment(
            x + 6,
            topY,
            DIGIT_W - 12,
            SEG_THICK,
            COLOR_TIME
        );
    if (digitSegments[number][1])
        drawVerticalSegment(
            rightX,
            y + 6,
            SEG_THICK,
            verticalH,
            COLOR_TIME
        );
    if (digitSegments[number][2])
        drawVerticalSegment(
            rightX,
            y + 72,
            SEG_THICK,
            verticalH,
            COLOR_TIME
        );
    if (digitSegments[number][3])
        drawHorizontalSegment(
            x + 6,
            bottomY,
            DIGIT_W - 12,
            SEG_THICK,
            COLOR_TIME
        );
    if (digitSegments[number][4])
        drawVerticalSegment(
            leftX,
            y + 72,
            SEG_THICK,
            verticalH,
            COLOR_TIME
        );
    if (digitSegments[number][5])
        drawVerticalSegment(
            leftX,
            y + 6,
            SEG_THICK,
            verticalH,
            COLOR_TIME
        );
    if (digitSegments[number][6])
        drawHorizontalSegment(
            x + 6,
            middleY,
            DIGIT_W - 12,
            SEG_THICK,
            COLOR_TIME
        );
}
// ============================================================
// COLON
// ============================================================
void drawColon(bool visible)
{
    tft.fillRect(
        COLON_X - 5,
        CLOCK_Y,
        30,
        DIGIT_H,
        COLOR_BG
    );
    if (!visible)
        return;
    tft.fillCircle(
        COLON_X + 5,
        CLOCK_Y + 43,
        7,
        COLOR_TIME
    );
    tft.fillCircle(
        COLON_X + 5,
        CLOCK_Y + 88,
        7,
        COLOR_TIME
    );
}


// CLOCK UPDATE
// ============================================================
int lastDigits[4] = { -1, -1, -1, -1 };
void updateClock(struct tm& timeinfo)
{
    int digits[4];
    int hour = timeinfo.tm_hour;
    if (!setting24Hour)
    {
        hour = hour % 12;
        if (hour == 0)
            hour = 12;
    }
    digits[0] = hour / 10;
    digits[1] = hour % 10;
    digits[2] = timeinfo.tm_min / 10;
    digits[3] = timeinfo.tm_min % 10;
    for (int i = 0; i < 4; i++)
    {
        if (digits[i] != lastDigits[i])
        {
            drawDigit(
                getDigitX(i),
                CLOCK_Y,
                digits[i]
            );
            lastDigits[i] = digits[i];
        }
    }
}

// ============================================================
// DATE
// ============================================================
int lastDay = -1;
void updateDate(struct tm& timeinfo)
{
    if (timeinfo.tm_mday == lastDay)
        return;
    lastDay = timeinfo.tm_mday;
    tft.fillRect(
        0,
        185,
        480,
        55,
        COLOR_BG
    );
    char dateText[50];
    snprintf(
        dateText,
        sizeof(dateText),
        "%s, %02d %s %d",
        days[timeinfo.tm_wday],
        timeinfo.tm_mday,
        months[timeinfo.tm_mon],
        timeinfo.tm_year + 1900
    );
    tft.setTextColor(
        COLOR_TIME,
        COLOR_BG
    );
    tft.setTextFont(4);
    int textWidth = tft.textWidth(dateText);
    tft.setCursor(
        (480 - textWidth) / 2,
        187
    );
    tft.print(dateText);
}

// ============================================================
// WIFI INDICATOR
// ============================================================
bool lastWiFiState = false;
void updateWiFiIndicator()
{
    bool connected = WiFi.status() == WL_CONNECTED;
    if (connected == lastWiFiState)
        return;
    lastWiFiState = connected;
    tft.fillCircle(
        15,
        15,
        7,
        COLOR_BG
    );
    if (connected)
    {
        tft.fillCircle(
            15,
            15,
            5,
            COLOR_WIFI
        );
    }
    else
    {
        tft.drawCircle(
            15,
            15,
            5,
            TFT_RED
        );
    }
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
unsigned long lastLowerPanelChange = 0;
unsigned long lastWeatherUpdate = 0;
unsigned long lastRainAnimation = 0;
int rainFrame = 0;
unsigned long lastSunAnimation = 0;
int sunFrame = 0;
unsigned long lastLightningAnimation = 0;
int lightningFrame = 0;
unsigned long lastCloudAnimation = 0;
int cloudFrame = 0;
// ============================================================
// MENU ICON
// ============================================================
void drawMenuIcon()
{
    tft.fillRect(
        8,
        35,
        55,
        38,
        COLOR_BG
    );

tft.drawLine(5, 41, 35, 41, TFT_WHITE);
tft.drawLine(5, 49, 35, 49, TFT_WHITE);
tft.drawLine(5, 57, 35, 57, TFT_WHITE);
}
// ============================================================
// WEATHER DISPLAY
// ============================================================
void drawWeather()
{
    // Clear weather area
    tft.fillRect(
        0,
        WEATHER_Y,
        480,
        75,
        COLOR_BG
    );
    // Menu icon
    drawMenuIcon();
}


// ============================================================
// WEATHER ICON
// ============================================================

void drawWeatherIcon()
{
    int cx = 40;
    int cy = WEATHER_Y + 37;
    int code = weather.getData().weatherCode;

    // =========================
    // CLEAR SKY
    // =========================
    if (code == 0)
    {
        tft.fillCircle(
            cx,
            cy,
            10,
            TFT_YELLOW
        );

        // Rrezet
        tft.drawLine(
            cx, cy - 17,
            cx, cy - 11,
            TFT_YELLOW
        );

        tft.drawLine(
            cx, cy + 11,
            cx, cy + 17,
            TFT_YELLOW
        );

        tft.drawLine(
            cx - 17, cy,
            cx - 11, cy,
            TFT_YELLOW
        );

        tft.drawLine(
            cx + 11, cy,
            cx + 17, cy,
            TFT_YELLOW
        );

        tft.drawLine(
            cx - 12, cy - 12,
            cx - 8, cy - 8,
            TFT_YELLOW
        );

        tft.drawLine(
            cx + 8, cy - 8,
            cx + 12, cy - 12,
            TFT_YELLOW
        );

        tft.drawLine(
            cx - 12, cy + 12,
            cx - 8, cy + 8,
            TFT_YELLOW
        );

        tft.drawLine(
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
        tft.fillCircle(
            cx - 10,
            cy + 4,
            9,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx + 2,
            cy - 2,
            12,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx + 13,
            cy + 5,
            8,
            TFT_LIGHTGREY
        );

        tft.fillRect(
            cx - 18,
            cy + 4,
            40,
            12,
            TFT_LIGHTGREY
        );
    }

    // =========================
    // FOG
    // =========================
    else if (code >= 45 && code <= 48)
    {
        tft.drawLine(
            cx - 18, cy - 8,
            cx + 18, cy - 8,
            TFT_LIGHTGREY
        );

        tft.drawLine(
            cx - 22, cy,
            cx + 14, cy,
            TFT_LIGHTGREY
        );

        tft.drawLine(
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
        // Re
        tft.fillCircle(
            cx - 9,
            cy - 1,
            9,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx + 3,
            cy - 6,
            12,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx + 14,
            cy,
            8,
            TFT_LIGHTGREY
        );

        tft.fillRect(
            cx - 17,
            cy,
            39,
            11,
            TFT_LIGHTGREY
        );

        // Pikat e shiut
        tft.drawLine(
            cx - 10, cy + 15,
            cx - 13, cy + 23,
            TFT_CYAN
        );

        tft.drawLine(
            cx, cy + 15,
            cx - 3, cy + 23,
            TFT_CYAN
        );

        tft.drawLine(
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
        tft.fillCircle(
            cx - 9,
            cy - 1,
            9,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx + 3,
            cy - 6,
            12,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx + 14,
            cy,
            8,
            TFT_LIGHTGREY
        );

        tft.fillRect(
            cx - 17,
            cy,
            39,
            11,
            TFT_LIGHTGREY
        );

        // Floket
        tft.fillCircle(
            cx - 9,
            cy + 19,
            3,
            TFT_WHITE
        );

        tft.fillCircle(
            cx + 2,
            cy + 23,
            3,
            TFT_WHITE
        );

        tft.fillCircle(
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
        // Re
        tft.fillCircle(
            cx - 9,
            cy - 1,
            9,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx + 3,
            cy - 6,
            12,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx + 14,
            cy,
            8,
            TFT_LIGHTGREY
        );

        tft.fillRect(
            cx - 17,
            cy,
            39,
            11,
            TFT_LIGHTGREY
        );

        // Shi
        tft.drawLine(
            cx - 9, cy + 15,
            cx - 12, cy + 23,
            TFT_CYAN
        );

        tft.drawLine(
            cx + 1, cy + 15,
            cx - 2, cy + 23,
            TFT_CYAN
        );

        tft.drawLine(
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
        tft.fillCircle(
            cx - 9,
            cy - 1,
            9,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx + 3,
            cy - 6,
            12,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx + 14,
            cy,
            8,
            TFT_LIGHTGREY
        );

        tft.fillRect(
            cx - 17,
            cy,
            39,
            11,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx - 8,
            cy + 19,
            3,
            TFT_WHITE
        );

        tft.fillCircle(
            cx + 3,
            cy + 23,
            3,
            TFT_WHITE
        );

        tft.fillCircle(
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
    {
        // Re
        tft.fillCircle(
            cx - 9,
            cy - 1,
            9,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx + 3,
            cy - 6,
            12,
            TFT_LIGHTGREY
        );

        tft.fillCircle(
            cx + 14,
            cy,
            8,
            TFT_LIGHTGREY
        );

        tft.fillRect(
            cx - 17,
            cy,
            39,
            11,
            TFT_LIGHTGREY
        );

        // Rrufe
        tft.fillTriangle(
            cx + 1, cy + 10,
            cx - 6, cy + 22,
            cx + 1, cy + 19,
            TFT_YELLOW
        );

        tft.fillTriangle(
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
        tft.drawCircle(
            cx,
            cy,
            12,
            TFT_WHITE
        );

        tft.drawLine(
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
void animateRainDrops()
{
    int code = weather.getData().weatherCode;

    // Vetëm shi dhe showers
    if (!((code >= 51 && code <= 67) ||
          (code >= 80 && code <= 82)))
    {
        return;
    }

    const int cx = 40;
    const int cy = WEATHER_Y + 37;

    // Pastro vetëm zonën e pikave të shiut
    tft.fillRect(
        cx - 18,
        cy + 15,
        40,
        21,
        COLOR_BG
    );

    int drop1 = rainFrame;
    int drop2 = (rainFrame + 4) % 12;
    int drop3 = (rainFrame + 8) % 12;

    tft.drawLine(
        cx - 9,
        cy + 15 + drop1,
        cx - 12,
        cy + 21 + drop1,
        TFT_CYAN
    );

    tft.drawLine(
        cx + 1,
        cy + 15 + drop2,
        cx - 2,
        cy + 21 + drop2,
        TFT_CYAN
    );

    tft.drawLine(
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
void animateSun()
{
    int code = weather.getData().weatherCode;

    // Vetëm clear sky
    if (code != 0)
        return;

    const int cx = 40;
    const int cy = WEATHER_Y + 37;

    // Gjatësia e rrezes së vjetër
    int oldOuter = 17 + sunFrame;

    // Fshi vetëm rrezet e vjetra
    tft.drawLine(cx, cy - 12, cx, cy - oldOuter, COLOR_BG);
    tft.drawLine(cx, cy + 12, cx, cy + oldOuter, COLOR_BG);

    tft.drawLine(cx - 12, cy, cx - oldOuter, cy, COLOR_BG);
    tft.drawLine(cx + 12, cy, cx + oldOuter, cy, COLOR_BG);

    tft.drawLine(cx - 8, cy - 8,
                 cx - oldOuter + 5, cy - oldOuter + 5, COLOR_BG);

    tft.drawLine(cx + 8, cy - 8,
                 cx + oldOuter - 5, cy - oldOuter + 5, COLOR_BG);

    tft.drawLine(cx - 8, cy + 8,
                 cx - oldOuter + 5, cy + oldOuter - 5, COLOR_BG);

    tft.drawLine(cx + 8, cy + 8,
                 cx + oldOuter - 5, cy + oldOuter - 5, COLOR_BG);

    // Kalo në frame tjetër
    sunFrame++;

    if (sunFrame > 2)
        sunFrame = 0;

    int newOuter = 17 + sunFrame;

    // Vizato vetëm rrezet e reja
    tft.drawLine(cx, cy - 12, cx, cy - newOuter, TFT_YELLOW);
    tft.drawLine(cx, cy + 12, cx, cy + newOuter, TFT_YELLOW);

    tft.drawLine(cx - 12, cy, cx - newOuter, cy, TFT_YELLOW);
    tft.drawLine(cx + 12, cy, cx + newOuter, cy, TFT_YELLOW);

    tft.drawLine(cx - 8, cy - 8,
                 cx - newOuter + 5, cy - newOuter + 5, TFT_YELLOW);

    tft.drawLine(cx + 8, cy - 8,
                 cx + newOuter - 5, cy - newOuter + 5, TFT_YELLOW);

    tft.drawLine(cx - 8, cy + 8,
                 cx - newOuter + 5, cy + newOuter - 5, TFT_YELLOW);

    tft.drawLine(cx + 8, cy + 8,
                 cx + newOuter - 5, cy + newOuter - 5, TFT_YELLOW);
}
void animateLightning()
{
    int code = weather.getData().weatherCode;

    // Vetëm thunderstorm
    if (code < 95 || code > 99)
        return;

    const int cx = 40;
    const int cy = WEATHER_Y + 37;

    // Rrufeja pulson: një frame fikur, një frame ndezur
    if (lightningFrame == 0)
    {
        tft.fillTriangle(
            cx + 1, cy + 10,
            cx - 6, cy + 22,
            cx + 1, cy + 19,
            COLOR_BG
        );

        tft.fillTriangle(
            cx + 1, cy + 19,
            cx + 8, cy + 19,
            cx + 1, cy + 29,
            COLOR_BG
        );
    }
    else
    {
        tft.fillTriangle(
            cx + 1, cy + 10,
            cx - 6, cy + 22,
            cx + 1, cy + 19,
            TFT_YELLOW
        );

        tft.fillTriangle(
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
void animateCloud()
{
    int code = weather.getData().weatherCode;

    // Vetëm ikonat me re
    if (!((code >= 1 && code <= 3) ||
          (code >= 51 && code <= 67) ||
          (code >= 71 && code <= 77) ||
          (code >= 80 && code <= 82) ||
          (code >= 85 && code <= 86) ||
          (code >= 95 && code <= 99)))
    {
        return;
    }

    const int cx = 40;
    const int cy = WEATHER_Y + 37;

    // Lëvizje shumë e vogël majtas-djathtas
    int offset = (cloudFrame == 0) ? 0 : 1;

    // Fshi vetëm pjesën e poshtme të resë
    tft.fillRect(
        cx - 18,
        cy - 1,
        40,
        12,
        COLOR_BG
    );

    // Rivizato trupin e resë me zhvendosje minimale
    tft.fillCircle(cx - 10 + offset, cy + 4, 9, TFT_LIGHTGREY);
    tft.fillCircle(cx + 2 + offset, cy - 2, 12, TFT_LIGHTGREY);
    tft.fillCircle(cx + 13 + offset, cy + 5, 8, TFT_LIGHTGREY);
    tft.fillRect(cx - 18 + offset, cy + 4, 40, 12, TFT_LIGHTGREY);

    cloudFrame++;

    if (cloudFrame >= 2)
        cloudFrame = 0;
}
void updateWeatherDisplay()
{
    if (!weather.getData().valid)
        return;

    const WeatherData& data = weather.getData();

    // Clear weather data area
    tft.fillRect(
        75,
        WEATHER_Y,
        405,
        75,
        COLOR_BG
    );

    // Titulli
    tft.setTextColor(
        TFT_WHITE,
        COLOR_BG
    );

    tft.setTextFont(2);
    tft.setTextDatum(MC_DATUM);

    tft.drawString(
        "Moti ne Sarande",
        277,
        WEATHER_Y + 7
    );

    tft.setTextDatum(TL_DATUM);

    // Weather icon
    drawWeatherIcon();

    tft.setTextColor(
        TFT_WHITE,
        COLOR_BG
    );

    tft.setTextFont(4);

    // --------------------------------------------------------
    // TEMPERATURE
    // --------------------------------------------------------

    char tempText[20];

    snprintf(
        tempText,
        sizeof(tempText),
        "Temp: %.1f C",
        data.temperature
    );

    tft.setTextColor(
        TFT_ORANGE,
        COLOR_BG
    );

    tft.setCursor(
        85,
        WEATHER_Y + 25
    );

    tft.print(tempText);

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

    tft.setTextColor(
        TFT_CYAN,
        COLOR_BG
    );

    tft.setCursor(
        85,
        WEATHER_Y + 52
    );

    tft.print(humidityText);

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

    tft.setTextColor(
        TFT_GREEN,
        COLOR_BG
    );

    tft.setCursor(
        285,
        WEATHER_Y + 25
    );

    tft.print(windText);

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

    tft.setTextColor(
        TFT_MAGENTA,
        COLOR_BG
    );

    tft.setCursor(
        285,
        WEATHER_Y + 52
    );

    tft.print(pressureText);
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
        "Sensoret lokale",
        277,
        WEATHER_Y + 7
    );
    tft.setTextDatum(TL_DATUM);

    // Madhesia e te dhenave
    tft.setTextFont(4);

    // Temperature
    tft.setTextColor(TFT_ORANGE, COLOR_BG);
    tft.setCursor(85, WEATHER_Y + 25);

    if (bme680OK)
    {
        tft.print("Temp: ");
        tft.print(bmeTemp, 1);
        tft.print(" C");
    }
    else
    {
        tft.print("TEMP: --.- C");
    }

    // Humidity
    tft.setTextColor(TFT_CYAN, COLOR_BG);
    tft.setCursor(285, WEATHER_Y + 25);

    if (bme680OK)
    {
        tft.print("Lag: ");
        tft.print(bmeHumidity, 0);
        tft.print(" %");
    }
    else
    {
        tft.print("Lag: -- %");
    }

    // Lux
    tft.setTextColor(TFT_YELLOW, COLOR_BG);
    tft.setCursor(85, WEATHER_Y + 52);

    if (bh1750OK)
    {
        tft.print("LUX: ");
        tft.print(bh1750Lux, 0);
    }
    else
    {
        tft.print("LUX: ----");
    }

    // Air quality
    tft.setTextColor(TFT_GREEN, COLOR_BG);
    tft.setCursor(285, WEATHER_Y + 52);

    if (bme680OK)
    {
        if (!airBaselineReady)
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
    else
    {
        tft.print("Ajer: --");
    }
}
// ============================================================
// SETUP
// ============================================================
void setup()
{
    Serial.begin(115200);
    delay(1000);
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
initSensors();
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
    tft.fillScreen(COLOR_BG);
    initScreenSaver(&tft);
    // --------------------------------------------------------
    // STARTUP MESSAGE
    // --------------------------------------------------------
    tft.setTextColor(
        TFT_WHITE,
        COLOR_BG
    );
    tft.setTextFont(4);
    const char* startupText = "LIDHJA ME WI-FI...";
    int startupWidth = tft.textWidth(startupText);
    tft.setCursor(
        (480 - startupWidth) / 2,
        140
    );
    tft.print(startupText);
    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------
    Serial.println("Connecting to WiFi...");
    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );
    unsigned long startAttempt = millis();
    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startAttempt < 20000
    )
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi connected");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        webServer.setWeatherService(&weather);
        webServer.begin();
        
        // ----------------------------------------------------
        // NTP
        // ----------------------------------------------------
        configTime(
            7200,
            0,
            NTP_SERVER,
            "time.nist.gov",
            "time.google.com"
        );
        Serial.println("Waiting for NTP...");
        bool timeSynced = false;
        for (int i = 0; i < 30; i++)
        {
            if (getLocalTime(&timeinfo, 1000))
            {
                Serial.println("Time synchronized");
             lastTuyaUpdate = millis();   
                Serial.printf(
                    "Current time: %02d:%02d:%02d\n",
                    timeinfo.tm_hour,
                    timeinfo.tm_min,
                    timeinfo.tm_sec
                );
                timeSynced = true;
                break;
            }
            Serial.print(".");
        }
        Serial.println();
        if (!timeSynced)
        {
            Serial.println("NTP failed");
        }
        weather.update();
        lastWeatherUpdate = millis();
    }
    else
    {
        Serial.println("WiFi connection failed");
    }
    // --------------------------------------------------------
    // INITIAL DISPLAY
    // --------------------------------------------------------
    tft.fillScreen(COLOR_BG);
    lastWiFiState = false;
    if (!screenSaverActive)
{
    updateWiFiIndicator();
}
    if (getLocalTime(&timeinfo, 2000))
    {
        updateClock(timeinfo);
if (settingDate)
{
    updateDate(timeinfo);
}
drawColon(true);
    }
    drawWeather();
    updateWeatherDisplay();
    lastLowerPanelChange = millis();
}
// ============================================================
// LOWER PANEL UPDATE
// ============================================================
void updateLowerPanel()
{
    if (settingWeather && settingLightSensor)
    {
        unsigned long displayTime =
            showingLocalSensors
                ? localSensorsDisplayTime
                : weatherDisplayTime;

        if (millis() - lastLowerPanelChange >= displayTime)
        {
            lastLowerPanelChange = millis();

            showingLocalSensors = !showingLocalSensors;

            if (showingLocalSensors)
            {
                updateLocalSensorsDisplay();
            }
            else
            {
                updateWeatherDisplay();
            }
        }
    }
    else if (settingWeather)
    {
        if (showingLocalSensors)
        {
            showingLocalSensors = false;
            updateWeatherDisplay();
        }
    }
    else if (settingLightSensor)
    {
        if (!showingLocalSensors)
        {
            showingLocalSensors = true;
            updateLocalSensorsDisplay();
        }
    }
}
// ============================================================
// LOOP
// ============================================================
void loop()
{
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
// SCREEN SAVER WAKE
// --------------------------------------------------------

if (screenSaverActive)
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

        // Injoro PIR per 2 sekonda pas wake
        pirIgnoreUntil = millis() + 2000;

        // Pastro ekranin
        tft.fillScreen(COLOR_BG);

        // Detyro ridizenjimin e shifrave
        lastDigits[0] = -1;
        lastDigits[1] = -1;
        lastDigits[2] = -1;
        lastDigits[3] = -1;

       // Force date redraw after screen saver
          lastDay = -1;

         if (getLocalTime(&timeinfo, 1000))
        {
            updateClock(timeinfo);

            if (settingDate)
            {
                updateDate(timeinfo);
            }

            bool visible = (timeinfo.tm_sec % 2 == 0);
            drawColon(visible);
        }

        // Rifresko elementet e ekranit
        lastWiFiState = false;
        updateWiFiIndicator();
        updateLowerPanel();
        drawMenuIcon();

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
    // SCREEN SAVER ACTIVE
    // --------------------------------------------------------
       if (screenSaverActive)
       updateScreenSaver(true);
       else
    {
        // ----------------------------------------------------
        // CLOCK
        // ----------------------------------------------------
        if (getLocalTime(&timeinfo, 1000))
        {
            updateClock(timeinfo);
            if (settingDate)
        {
         updateDate(timeinfo);
        }
            static int lastSecond = -1;
            if (timeinfo.tm_sec != lastSecond)
            {
                lastSecond = timeinfo.tm_sec;
                bool visible =
                    (timeinfo.tm_sec % 2 == 0);
                drawColon(visible);
            }
        }
        // ----------------------------------------------------
        // LOCAL SENSORS
        // ----------------------------------------------------
        if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL)
        {
            lastSensorRead = millis();
            updateLocalSensors();
            if (showingLocalSensors)
            {
                updateLocalSensorsDisplay();
            }
        }
        // ----------------------------------------------------
        // WIFI   INDICATOR
        // ----------------------------------------------------
        updateWiFiIndicator();
        // ----------------------------------------------------
        // WEATHER
        // --------------------------------------------------------
        if (millis() - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL)
        {
            lastWeatherUpdate = millis();
            if (weather.update())
            {
                updateWeatherDisplay();
            }
        }
        // ----------------------------------------------------
        // LOWER PANEL
        // ----------------------------------------------------
        updateLowerPanel();
       // ----------------------------------------------------
// RAIN ANIMATION
// ----------------------------------------------------
if (!showingLocalSensors &&
    weather.getData().valid &&
    millis() - lastRainAnimation >= 180)
{
    lastRainAnimation = millis();
    animateRainDrops();
}
if (!showingLocalSensors &&
    weather.getData().valid &&
    weather.getData().weatherCode == 0 &&
    millis() - lastSunAnimation >= 180)
{
    lastSunAnimation = millis();
    animateSun();
}
if (!showingLocalSensors &&
    weather.getData().valid &&
    weather.getData().weatherCode >= 95 &&
    weather.getData().weatherCode <= 99 &&
    millis() - lastLightningAnimation >= 300)
{
    lastLightningAnimation = millis();
    animateLightning();
}
if (!showingLocalSensors &&
    weather.getData().valid &&
    weather.getData().weatherCode >= 1 &&
    weather.getData().weatherCode <= 3 &&
    millis() - lastCloudAnimation >= 500)
{
    lastCloudAnimation = millis();
    animateCloud();
}
    }
     // --------------------------------------------------------
     // TUYA TH01
     // --------------------------------------------------------
if (millis() - lastTuyaUpdate >= TUYA_UPDATE_INTERVAL)
{
    lastTuyaUpdate = millis();
    tuyaService.update();
}

    // WEB SERVER
    // --------------------------------------------------------
    webServer.handleClient();
}
