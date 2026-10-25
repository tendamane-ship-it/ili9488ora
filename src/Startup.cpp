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

    delay(200);

    // --------------------------------------------------------
    // SYSTEM LINES
    // --------------------------------------------------------

    drawBootLine(
        tft,
        "WiFi NETWORK",
        125
    );

    delay(200);

    drawBootLine(
        tft,
        "BME680 SENSOR",
        150
    );

    delay(200);

    drawBootLine(
        tft,
        "BH1750 LIGHT",
        175
    );

    delay(200);

    drawBootLine(
        tft,
        "OPEN-METEO ENGINE",
        200
    );

    delay(200);

    drawBootLine(
        tft,
        "TUYA CLOUD",
        225
    );

    delay(200);

    // --------------------------------------------------------
    // READY
    // --------------------------------------------------------

    drawCentered(
        tft,
        "TE GJITHA SISTEMET GATI",
        260,
        STARTUP_OK,
        2
    );

    delay(200);

    
    // --------------------------------------------------------
    // FINISHED
    // --------------------------------------------------------

    
}