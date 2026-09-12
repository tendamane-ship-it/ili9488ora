#include <Arduino.h>
#include <WiFi.h>
#include "WebServerManager.h"
#include "Pages.h"
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


// ============================================================
// COLORS
// ============================================================

#define COLOR_BG    TFT_BLACK
#define COLOR_TIME  TFT_CYAN
#define COLOR_DATE  TFT_WHITE
#define COLOR_WIFI  TFT_GREEN

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

// ============================================================
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

    char dateText[40];

    strftime(
        dateText,
        sizeof(dateText),
        "%A, %d %B %Y",
        &timeinfo
    );

    tft.setTextColor(
        COLOR_DATE,
        COLOR_BG
    );

    tft.setTextFont(4);

    int textWidth = tft.textWidth(dateText);

    tft.setCursor(
        (480 - textWidth) / 2,
        200
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

// ============================================================
// SUN
// ============================================================

void drawSun(int x, int y)
{
    // Center of sun
    tft.fillCircle(
        x,
        y,
        18,
        TFT_YELLOW
    );

    // Rays
    tft.drawLine(x, y - 30, x, y - 23, TFT_YELLOW);
    tft.drawLine(x, y + 23, x, y + 30, TFT_YELLOW);

    tft.drawLine(x - 30, y, x - 23, y, TFT_YELLOW);
    tft.drawLine(x + 23, y, x + 30, y, TFT_YELLOW);

    tft.drawLine(
        x - 21,
        y - 21,
        x - 16,
        y - 16,
        TFT_YELLOW
    );

    tft.drawLine(
        x + 16,
        y - 16,
        x + 21,
        y - 21,
        TFT_YELLOW
    );

    tft.drawLine(
        x - 21,
        y + 21,
        x - 16,
        y + 16,
        TFT_YELLOW
    );

    tft.drawLine(
        x + 16,
        y + 16,
        x + 21,
        y + 21,
        TFT_YELLOW
    );
}


// ============================================================
// WEATHER DISPLAY
// ============================================================

void drawWeather()
{
    // Clear only the weather area
    tft.fillRect(
        0,
        WEATHER_Y,
        480,
        75,
        COLOR_BG
    );

    // Sun
    drawSun(
        145,
        WEATHER_Y + 35
    );
}
   // ============================================================
// WEB SERVER
// ============================================================

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

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

    updateWiFiIndicator();

    

    if (getLocalTime(&timeinfo, 2000))
    {
        updateClock(timeinfo);
        updateDate(timeinfo);
        drawColon(true);
    }

    drawWeather();
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // CLOCK
    // --------------------------------------------------------

   

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

    // --------------------------------------------------------
    // WIFI INDICATOR
    // --------------------------------------------------------

    updateWiFiIndicator();
    webServer.handleClient();
    }

    