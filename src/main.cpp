#include <Arduino.h>

#include <WiFi.h>

#include "WebServerManager.h"

#include "Pages.h"

#include "WeatherService.h"

#include "Sensors.h"

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
WeatherService weather;
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

const unsigned long SCREEN_SAVER_DELAY = 20UL * 1000UL;
const unsigned long HEART_INTERVAL = 1200UL;
TFT_eSprite heartSprite = TFT_eSprite(&tft);

bool heartSpriteReady = false;

unsigned long lastHeartFrame = 0;
float heartPhase = 0.0;
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

#define DIGIT_W       84
#define DIGIT_H       130
#define SEG_THICK     13
#define DIGIT_GAP     10
#define CLOCK_Y       25

const int COLON_X = 232;

// ============================================================
// CLOCK GEOMETRY
// ============================================================

#define DIGIT_W       84
#define DIGIT_H       130
#define SEG_THICK     13
#define DIGIT_GAP     10
#define CLOCK_Y       25



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
void updateScreenSaver()
{
    if (!screenSaverActive)
        return;

    // --------------------------------------------------------
    // INITIALIZO SPRITE
    // --------------------------------------------------------

    if (!heartSpriteReady)
    {
        Serial.println("SCREEN SAVER: creating heart sprite...");

        heartSprite.setColorDepth(8);

        if (heartSprite.createSprite(300, 200))
        {
            heartSpriteReady = true;

            Serial.println("SCREEN SAVER: heart sprite CREATED");

            heartPhase = 0.0;
            lastHeartFrame = millis();
        }
        else
        {
            Serial.println("SCREEN SAVER: heart sprite FAILED");
            return;
        }
    }

    // --------------------------------------------------------
    // ~30 FPS
    // --------------------------------------------------------

    if (millis() - lastHeartFrame < 33)
        return;

    lastHeartFrame = millis();

    // --------------------------------------------------------
    // HEARTBEAT
    // --------------------------------------------------------

    heartPhase += 0.12;

    float cycle = fmod(heartPhase, TWO_PI);

    float pulse = 1.0;

    // RRAHJA E PARE
    if (cycle < 0.55)
    {
        float p = sin((cycle / 0.55) * PI);
        pulse = 1.0 + 0.14 * p;
    }

    // RRAHJA E DYTE
    else if (cycle >= 0.75 && cycle < 1.15)
    {
        float p = sin(
            ((cycle - 0.75) / 0.40) * PI
        );

        pulse = 1.0 + 0.075 * p;
    }

    // --------------------------------------------------------
    // PASTRO VETEM SPRITE-IN
    // --------------------------------------------------------

    heartSprite.fillSprite(TFT_BLACK);

    // --------------------------------------------------------
    // PARAMETRIC HEART
    // --------------------------------------------------------

    const int cx = 150;
    const int cy = 92;

    float scale = 5.8 * pulse;

    bool firstPoint = true;

    int oldX = 0;
    int oldY = 0;

    for (float t = 0; t <= TWO_PI; t += 0.015)
    {
        float x =
            16.0 *
            sin(t) *
            sin(t) *
            sin(t);

        float y =
            13.0 * cos(t)
            - 5.0 * cos(2.0 * t)
            - 2.0 * cos(3.0 * t)
            - cos(4.0 * t);

        int px = cx + (int)(x * scale);
        int py = cy - (int)(y * scale);

        if (!firstPoint)
        {
          // --------------------------------------------------------
// ZEMER - VIJE 7 PX
// --------------------------------------------------------

heartSprite.drawLine(
    oldX, oldY,
    px, py,
    TFT_CYAN
);

heartSprite.drawLine(
    oldX + 1, oldY,
    px + 1, py,
    TFT_CYAN
);

heartSprite.drawLine(
    oldX - 1, oldY,
    px - 1, py,
    TFT_CYAN
);

heartSprite.drawLine(
    oldX, oldY + 1,
    px, py + 1,
    TFT_CYAN
);

heartSprite.drawLine(
    oldX, oldY - 1,
    px, py - 1,
    TFT_CYAN
);

heartSprite.drawLine(
    oldX + 2, oldY + 1,
    px + 2, py + 1,
    TFT_CYAN
);

heartSprite.drawLine(
    oldX - 2, oldY - 1,
    px - 2, py - 1,
    TFT_CYAN
);
        }

        oldX = px;
        oldY = py;

        firstPoint = false;
    }

    // --------------------------------------------------------
    // MANE
    // --------------------------------------------------------

    heartSprite.setTextDatum(MC_DATUM);

    heartSprite.setTextFont(4);

    heartSprite.setTextColor(
        TFT_WHITE,
        TFT_BLACK
    );

    heartSprite.drawString(
        "Mane",
        150,
        178
    );

    heartSprite.setTextDatum(TL_DATUM);

    // --------------------------------------------------------
    // SHFAQ SPRITE SI NJE BllOK TE VETEM
    // --------------------------------------------------------

    heartSprite.pushSprite(
        90,
        60
    );
}

// CLOCK UPDATE
// ============================================================

int lastDigits[4] = { -1, -1, -1, -1 };

void updateClock(struct tm& timeinfo)
{
    int digits[4];

    digits[0] = timeinfo.tm_hour / 10;
    digits[1] = timeinfo.tm_hour % 10;
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

     #define LOWER_PANEL_INTERVAL 10000UL   // 10 sekonda

     bool showingLocalSensors = false;
     unsigned long lastLowerPanelChange = 0;

unsigned long lastWeatherUpdate = 0;
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

    tft.drawLine(
        18,
        41,
        48,
        41,
        TFT_WHITE
    );

    tft.drawLine(
        18,
        49,
        48,
        49,
        TFT_WHITE
    );

    tft.drawLine(
        18,
        57,
        48,
        57,
        TFT_WHITE
    );
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
        tft.drawLine(cx, cy - 17, cx, cy - 11, TFT_YELLOW);
        tft.drawLine(cx, cy + 11, cx, cy + 17, TFT_YELLOW);

        tft.drawLine(cx - 17, cy, cx - 11, cy, TFT_YELLOW);
        tft.drawLine(cx + 11, cy, cx + 17, cy, TFT_YELLOW);

        tft.drawLine(cx - 12, cy - 12, cx - 8, cy - 8, TFT_YELLOW);
        tft.drawLine(cx + 8, cy - 8, cx + 12, cy - 12, TFT_YELLOW);

        tft.drawLine(cx - 12, cy + 12, cx - 8, cy + 8, TFT_YELLOW);
        tft.drawLine(cx + 8, cy + 8, cx + 12, cy + 12, TFT_YELLOW);
    }

    // =========================
    // CLOUDY
    // =========================
    else if (code >= 1 && code <= 3)
    {
        tft.fillCircle(cx - 10, cy + 4, 9, TFT_LIGHTGREY);
        tft.fillCircle(cx + 2, cy - 2, 12, TFT_LIGHTGREY);
        tft.fillCircle(cx + 13, cy + 5, 8, TFT_LIGHTGREY);

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
        tft.fillCircle(cx - 9, cy - 1, 9, TFT_LIGHTGREY);
        tft.fillCircle(cx + 3, cy - 6, 12, TFT_LIGHTGREY);
        tft.fillCircle(cx + 14, cy, 8, TFT_LIGHTGREY);

        tft.fillRect(
            cx - 17,
            cy,
            39,
            11,
            TFT_LIGHTGREY
        );

        // Pikat e shiut
        tft.drawLine(cx - 10, cy + 15, cx - 13, cy + 23, TFT_CYAN);
        tft.drawLine(cx, cy + 15, cx - 3, cy + 23, TFT_CYAN);
        tft.drawLine(cx + 10, cy + 15, cx + 7, cy + 23, TFT_CYAN);
    }

    // =========================
    // SNOW
    // =========================
    else if (code >= 71 && code <= 77)
    {
        // Re
        tft.fillCircle(cx - 9, cy - 1, 9, TFT_LIGHTGREY);
        tft.fillCircle(cx + 3, cy - 6, 12, TFT_LIGHTGREY);
        tft.fillCircle(cx + 14, cy, 8, TFT_LIGHTGREY);

        tft.fillRect(
            cx - 17,
            cy,
            39,
            11,
            TFT_LIGHTGREY
        );

        // Floket
        tft.fillCircle(cx - 9, cy + 19, 3, TFT_WHITE);
        tft.fillCircle(cx + 2, cy + 23, 3, TFT_WHITE);
        tft.fillCircle(cx + 12, cy + 18, 3, TFT_WHITE);
    }

    // =========================
    // SHOWERS
    // =========================
    else if (code >= 80 && code <= 82)
    {
        // Re
        tft.fillCircle(cx - 9, cy - 1, 9, TFT_LIGHTGREY);
        tft.fillCircle(cx + 3, cy - 6, 12, TFT_LIGHTGREY);
        tft.fillCircle(cx + 14, cy, 8, TFT_LIGHTGREY);

        tft.fillRect(
            cx - 17,
            cy,
            39,
            11,
            TFT_LIGHTGREY
        );

        // Shi
        tft.drawLine(cx - 9, cy + 15, cx - 12, cy + 23, TFT_CYAN);
        tft.drawLine(cx + 1, cy + 15, cx - 2, cy + 23, TFT_CYAN);
        tft.drawLine(cx + 11, cy + 15, cx + 8, cy + 23, TFT_CYAN);
    }

    // =========================
    // SNOW SHOWERS
    // =========================
    else if (code >= 85 && code <= 86)
    {
        tft.fillCircle(cx - 9, cy - 1, 9, TFT_LIGHTGREY);
        tft.fillCircle(cx + 3, cy - 6, 12, TFT_LIGHTGREY);
        tft.fillCircle(cx + 14, cy, 8, TFT_LIGHTGREY);

        tft.fillRect(
            cx - 17,
            cy,
            39,
            11,
            TFT_LIGHTGREY
        );

        tft.fillCircle(cx - 8, cy + 19, 3, TFT_WHITE);
        tft.fillCircle(cx + 3, cy + 23, 3, TFT_WHITE);
        tft.fillCircle(cx + 13, cy + 18, 3, TFT_WHITE);
    }

    // =========================
    // THUNDERSTORM
    // =========================
    else if (code >= 95 && code <= 99)
    {
        // Re
        tft.fillCircle(cx - 9, cy - 1, 9, TFT_LIGHTGREY);
        tft.fillCircle(cx + 3, cy - 6, 12, TFT_LIGHTGREY);
        tft.fillCircle(cx + 14, cy, 8, TFT_LIGHTGREY);

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
        // Ikonë neutrale
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
    tft.setTextColor(TFT_ORANGE, COLOR_BG);
    tft.setCursor(
        85,
        WEATHER_Y + 8
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
    tft.setTextColor(TFT_CYAN, COLOR_BG);

    tft.setCursor(
        85,
        WEATHER_Y + 40
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
    tft.setTextColor(TFT_GREEN, COLOR_BG);

    tft.setCursor(
        285,
        WEATHER_Y + 8
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
     tft.setTextColor(TFT_MAGENTA, COLOR_BG);
    tft.setCursor(
        285,
        WEATHER_Y + 40
    );

    tft.print(pressureText);
}
// ============================================================
// LOCAL SENSORS DISPLAY
// ============================================================
void updateLocalSensorsDisplay()
{
    tft.fillRect(75, WEATHER_Y, 405, 75, COLOR_BG);

    tft.setTextFont(2);

    // Titulli
    tft.setTextColor(TFT_WHITE, COLOR_BG);
    tft.setCursor(75, WEATHER_Y);
    tft.print("Sensoret lokale");

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
            {
                tft.print("AIR: GOOD");
            }
            else if (ratio >= 0.50)
            {
                tft.print("AIR: FAIR");
            }
            else
            {
                tft.print("AIR: POOR");
            }
        }
    }
    else
    {
        tft.print("Ajer: --");
    }
}
// ============================================================
// LOWER PANEL SLIDE
// ============================================================

void updateLowerPanel()
{
    if (millis() - lastLowerPanelChange < LOWER_PANEL_INTERVAL)
        return;

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


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);
   
    initPIR();
lastMotionTime = millis();
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
        updateDate(timeinfo);
        drawColon(true);
    }

    drawWeather();
    updateWeatherDisplay();
    lastLowerPanelChange = millis();
}
// ============================================================
// LOOP
// ============================================================

void loop()
{
// --------------------------------------------------------
// PIR / MOTION
// --------------------------------------------------------

int pirState = readPIR();
// --------------------------------------------------------
// SCREEN SAVER WAKE
// --------------------------------------------------------

if (screenSaverActive)
{
    // Fillimisht presim që PIR të kthehet LOW.
    // Kjo shmang wake të menjëhershëm kur screensaver-i aktivizohet
    // ndërkohë që PIR është ende HIGH.
   if (screenSaverActive && pirState == HIGH)
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

        // Rifresko oren dhe daten
        if (getLocalTime(&timeinfo, 1000))
        {
            updateClock(timeinfo);
            updateDate(timeinfo);

            bool visible = (timeinfo.tm_sec % 2 == 0);
            drawColon(visible);
        }

        // Rifresko elementet e ekranit
        updateWiFiIndicator();
        updateLowerPanel();

        lastMotionTime = now;

        // Mos e konsidero kete HIGH si nje levizje te re
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
        millis() - lastMotionTime >= SCREEN_SAVER_DELAY)
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
    {
        updateScreenSaver();
    }
    else
    {
        // ----------------------------------------------------
        // CLOCK
        // ----------------------------------------------------

        if (getLocalTime(&timeinfo, 1000))
        {
            updateClock(timeinfo);
            updateDate(timeinfo);

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
        // WIFI INDICATOR
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
    }


    // --------------------------------------------------------
    // WEB SERVER
    // --------------------------------------------------------

    webServer.handleClient();
}