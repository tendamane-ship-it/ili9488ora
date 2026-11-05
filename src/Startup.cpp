#include "Startup.h"

// ============================================================
// COLORS
// ============================================================

#define STARTUP_BG      TFT_BLACK
#define STARTUP_TEXT    TFT_WHITE
#define STARTUP_ACCENT  TFT_CYAN
#define STARTUP_OK_COLOR TFT_GREEN
#define STARTUP_ERROR_COLOR TFT_RED
#define STARTUP_WAIT_COLOR  TFT_YELLOW

static TFT_eSPI* startupDisplay = nullptr;
static volatile StartupStatus statuses[STARTUP_ITEM_COUNT];
static volatile bool startupNeedsRedraw = false;

static const char* const labels[STARTUP_ITEM_COUNT] = {
    "WiFi",
    "BME680 SENSOR",
    "BH1750 SENSOR",
    "CLOCK / NTP",
    "WEATHER SERVICE",
    "TUYA CLOUD"
};

static const char* statusText(StartupStatus status)
{
    switch (status)
    {
        case STARTUP_CHECKING: return "CHECKING";
        case STARTUP_OK: return "READY";
        case STARTUP_ERROR: return "ERROR";
        case STARTUP_PORTAL: return "SETUP PORTAL";
        case STARTUP_SKIPPED: return "WAITING FOR WIFI";
        case STARTUP_BACKGROUND: return "IN BACKGROUND";
        case STARTUP_WAITING:
        default: return "WAITING";
    }
}

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

// STARTUP FILM
// ============================================================

void runStartup(TFT_eSPI* tft)
{
    if (tft == nullptr)
        return;

    startupDisplay = tft;
    for (int i = 0; i < STARTUP_ITEM_COUNT; i++)
        statuses[i] = STARTUP_WAITING;
    startupNeedsRedraw = true;

    tft->fillScreen(STARTUP_BG);
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

    refreshStartup();
}

void setStartupStatus(StartupItem item, StartupStatus status)
{
    if (item < 0 || item >= STARTUP_ITEM_COUNT)
        return;

    statuses[item] = status;
    startupNeedsRedraw = true;
}

void refreshStartup()
{
    if (startupDisplay == nullptr || !startupNeedsRedraw)
        return;

    startupNeedsRedraw = false;
    startupDisplay->setTextFont(2);
    startupDisplay->setTextSize(1);

    for (int i = 0; i < STARTUP_ITEM_COUNT; i++)
    {
        int y = 125 + i * 25;
        StartupStatus status = statuses[i];
        uint16_t color = STARTUP_WAIT_COLOR;

        if (status == STARTUP_OK)
            color = STARTUP_OK_COLOR;
        else if (status == STARTUP_ERROR || status == STARTUP_PORTAL)
            color = STARTUP_ERROR_COLOR;

        startupDisplay->setTextColor(STARTUP_TEXT, STARTUP_BG);
        startupDisplay->setCursor(45, y);
        startupDisplay->print(labels[i]);
        startupDisplay->fillRect(285, y - 2, 160, 20, STARTUP_BG);
        startupDisplay->setTextColor(color, STARTUP_BG);
        int width = startupDisplay->textWidth(statusText(status));
        startupDisplay->setCursor(435 - width, y);
        startupDisplay->print(statusText(status));
    }
}

bool isStartupComplete()
{
    for (int i = 0; i < STARTUP_ITEM_COUNT; i++)
    {
        if (i == STARTUP_TUYA)
            continue;

        StartupStatus status = statuses[i];
        if (status == STARTUP_WAITING || status == STARTUP_CHECKING)
            return false;
    }

    return true;
}