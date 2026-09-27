#include "Startup.h"

// ============================================================
// COLORS
// ============================================================

#define STARTUP_BG      TFT_BLACK
#define STARTUP_TEXT    TFT_WHITE
#define STARTUP_ACCENT  TFT_CYAN
#define STARTUP_OK      TFT_GREEN

// ============================================================
// CENTERED TEXT
// ============================================================

void drawCentered(
    TFT_eSPI* tft,
    const char* text,
    int y,
    uint16_t color,
    uint8_t font
)
{
    tft->setTextFont(font);
    tft->setTextSize(1);
    tft->setTextColor(color, STARTUP_BG);

    int width = tft->textWidth(text);

    tft->setCursor(
        (480 - width) / 2,
        y
    );

    tft->print(text);
}

// ============================================================
// BOOT LINE
// ============================================================

void drawBootLine(
    TFT_eSPI* tft,
    const char* text,
    int y
)
{
    tft->setTextFont(2);
    tft->setTextSize(1);
    tft->setTextColor(
        STARTUP_TEXT,
        STARTUP_BG
    );

    tft->setCursor(45, y);
    tft->print(text);

    tft->setTextColor(
        STARTUP_OK,
        STARTUP_BG
    );

    tft->setCursor(385, y);
    tft->print("ONLINE");
}

// ============================================================
// MANE ZOOM
// ============================================================

void maneZoom(TFT_eSPI* tft)
{
    const char* text = "MANE";

    const int startSize = 1;
    const int endSize = 18;

    for (int size = startSize; size <= endSize; size++)
    {
        tft->fillScreen(STARTUP_BG);

        tft->setTextFont(1);
        tft->setTextSize(size);

        tft->setTextColor(
            STARTUP_ACCENT,
            STARTUP_BG
        );

        int width = tft->textWidth(text);
        int height = 8 * size;

        int x = (480 - width) / 2;
        int y = (320 - height) / 2;

        tft->setCursor(x, y);
        tft->print(text);

        delay(25);
    }

    // --------------------------------------------------------
    // FINAL LARGE MANE
    // --------------------------------------------------------

    tft->fillScreen(STARTUP_BG);

    tft->setTextFont(1);
    tft->setTextSize(24);

    tft->setTextColor(
        STARTUP_ACCENT,
        STARTUP_BG
    );

    int width = tft->textWidth(text);
    int height = 8 * 24;

    int x = (480 - width) / 2;
    int y = (320 - height) / 2;

    tft->setCursor(x, y);
    tft->print(text);

    delay(80);

    tft->fillScreen(STARTUP_BG);
    tft->setTextSize(1);
}

// ============================================================
// STARTUP FILM
// ============================================================

void runStartup(TFT_eSPI* tft)
{
    if (tft == nullptr)
        return;

    tft->fillScreen(STARTUP_BG);

    // --------------------------------------------------------
    // TITLE
    // --------------------------------------------------------

    drawCentered(
        tft,
        "ORA SYSTEM",
        45,
        STARTUP_ACCENT,
        4
    );

    drawCentered(
        tft,
        "BOOT SEQUENCE",
        82,
        STARTUP_TEXT,
        2
    );

    delay(250);

    // --------------------------------------------------------
    // SYSTEM LINES
    // --------------------------------------------------------

    drawBootLine(
        tft,
        "WiFi NETWORK",
        125
    );

    delay(500);

    drawBootLine(
        tft,
        "BME680 SENSOR",
        150
    );

    delay(500);

    drawBootLine(
        tft,
        "BH1750 LIGHT",
        175
    );

    delay(500);

    drawBootLine(
        tft,
        "OPEN-METEO ENGINE",
        200
    );

    delay(500);

    drawBootLine(
        tft,
        "TUYA CLOUD",
        225
    );

    delay(220);

    // --------------------------------------------------------
    // READY
    // --------------------------------------------------------

    drawCentered(
        tft,
        "ALL SYSTEMS READY",
        260,
        STARTUP_OK,
        2
    );

    delay(250);

    
    // --------------------------------------------------------
    // FINISHED
    // --------------------------------------------------------

    tft->fillScreen(STARTUP_BG);
}