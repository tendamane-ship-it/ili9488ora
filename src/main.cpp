#include <Arduino.h>
#include <WiFi.h>
#include "WebServerManager.h"
#include "Pages.h"
#include "WeatherService.h"
#include <TFT_eSPI.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_BME680.h>
#include <BH1750.h>
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
Adafruit_BME680 bme;
bool bme680OK = false;
BH1750 lightMeter;
bool bh1750OK = false;
float bmeTemp = 0.0;
float bmeHumidity = 0.0;
float bmePressure = 0.0;
float bmeGas = 0.0;
float bh1750Lux = 0.0;
float airBaseline = 0.0;
bool airBaselineReady = false;

float airBaselineSum = 0.0;
int airBaselineCount = 0;

const int AIR_BASELINE_SAMPLES = 30;
unsigned long lastSensorRead = 0;
const unsigned long SENSOR_READ_INTERVAL = 5000;

// ============================================================
// COLORS
// ============================================================

#define COLOR_BG    TFT_BLACK
#define COLOR_TIME  TFT_CYAN
#define COLOR_DATE  TFT_WHITE
#define COLOR_WIFI  TFT_GREEN

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
void updateAirQuality();
void updateLocalSensors()
{
    if (bme680OK)
    {
        if (bme.performReading())
        {
            bmeTemp = bme.temperature;
            bmeHumidity = bme.humidity;
            bmePressure = bme.pressure / 100.0;
            bmeGas = bme.gas_resistance / 1000.0;
        }
    }

    if (bh1750OK)
    {
        bh1750Lux = lightMeter.readLightLevel();
    }
    updateAirQuality();
}
void updateAirQuality()
{
    if (!bme680OK || bmeGas <= 0)
        return;

    // Mbledhim 30 matje për baseline
    if (!airBaselineReady)
    {
        airBaselineSum += bmeGas;
        airBaselineCount++;

        if (airBaselineCount >= AIR_BASELINE_SAMPLES)
        {
            airBaseline = airBaselineSum / AIR_BASELINE_SAMPLES;
            airBaselineReady = true;

            Serial.print("Air baseline: ");
            Serial.print(airBaseline, 2);
            Serial.println(" kOhm");
        }
    }
}
// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);
   

    // --------------------------------------------------------
// BME680
// --------------------------------------------------------

Wire.begin(8, 9);

if (bme.begin(0x76))
{
    bme680OK = true;
    Serial.println("BME680 found at 0x76");
}
else if (bme.begin(0x77))
{
    bme680OK = true;
    Serial.println("BME680 found at 0x77");
}
else
{
    Serial.println("BME680 NOT FOUND");
}
// --------------------------------------------------------
// BH1750
// --------------------------------------------------------

if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE))
{
    bh1750OK = true;
    Serial.println("BH1750 found at 0x23");
}
else
{
    Serial.println("BH1750 NOT FOUND");
}


if (bme680OK)
{
    if (bme.performReading())
    {
        bmeTemp = bme.temperature;
        bmeHumidity = bme.humidity;
        bmePressure = bme.pressure / 100.0;
        bmeGas = bme.gas_resistance / 1000.0;

        Serial.print("BME680 Temperature: ");
        Serial.print(bmeTemp);
        Serial.println(" C");

        Serial.print("BME680 Humidity: ");
        Serial.print(bmeHumidity);
        Serial.println(" %");

        Serial.print("BME680 Pressure: ");
        Serial.print(bmePressure);
        Serial.println(" hPa");

        Serial.print("BME680 Gas: ");
        Serial.print(bmeGas);
        Serial.println(" kOhm");
    }
    else
    {
        Serial.println("BME680 reading FAILED");
    }
}

if (bh1750OK)
{
    bh1750Lux = lightMeter.readLightLevel();

    Serial.print("BH1750 Lux: ");
    Serial.print(bh1750Lux);
    Serial.println(" lx");
}

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

    updateWiFiIndicator();

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
        if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL)
{
    lastSensorRead = millis();

    updateLocalSensors();

    if (showingLocalSensors)
    {
        updateLocalSensorsDisplay();
    }
}
    }

    // --------------------------------------------------------
    // WIFI INDICATOR
    // --------------------------------------------------------

    updateWiFiIndicator();
    if (millis() - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL)
{
    lastWeatherUpdate = millis();

    if (weather.update())
    {
        updateWeatherDisplay();
    }
}
    // --------------------------------------------------------
// LOWER PANEL SLIDE
// --------------------------------------------------------

updateLowerPanel();
   
    // --------------------------------------------------------
    // WEB SERVER
    // --------------------------------------------------------

    webServer.handleClient();
}
