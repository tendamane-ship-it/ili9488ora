#include "ClockDisplay.h"
#include "DisplayText.h"

namespace
{
constexpr uint16_t COLOR_BG = TFT_BLACK;
constexpr uint16_t COLOR_TIME = TFT_CYAN;
constexpr int DIGIT_W = 84;
constexpr int DIGIT_H = 130;
constexpr int SEG_THICK = 13;
constexpr int DIGIT_GAP = 10;
constexpr int CLOCK_Y = 25;
constexpr int MAIN_CONTENT_CENTER_X = 277;
constexpr int CLOCK_SHIFT_X = MAIN_CONTENT_CENTER_X - 237;
constexpr int COLON_X = 232 + CLOCK_SHIFT_X;

const bool digitSegments[10][7] =
{
    { true,  true,  true,  true,  true,  true,  false },
    { false, true,  true,  false, false, false, false },
    { true,  true,  false, true,  true,  false, true  },
    { true,  true,  true,  true,  false, false, true  },
    { false, true,  true,  false, false, true,  true  },
    { true,  false, true,  true,  false, true,  true  },
    { true,  false, true,  true,  true,  true,  true  },
    { true,  true,  true,  false, false, false, false },
    { true,  true,  true,  true,  true,  true,  true  },
    { true,  true,  true,  true,  false, true,  true  }
};

const char* const days[] = {
    "E diel", "E hënë", "E martë", "E mërkurë",
    "E enjte", "E premte", "E shtunë"
};

const char* const months[] = {
    "janar", "shkurt", "mars", "prill", "maj", "qershor",
    "korrik", "gusht", "shtator", "tetor", "nëntor", "dhjetor"
};

int lastDigits[4] = { -1, -1, -1, -1 };
int lastDay = -1;

int getDigitX(int index)
{
    const int startX = 43 + CLOCK_SHIFT_X;
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

void drawHorizontalSegment(
    TFT_eSPI& display,
    int x,
    int y,
    int w,
    int h,
    uint16_t color
)
{
    constexpr int tip = 5;
    display.fillRect(x + tip, y, w - 2 * tip, h, color);
    display.fillTriangle(x, y + h / 2, x + tip, y, x + tip, y + h, color);
    display.fillTriangle(x + w, y + h / 2, x + w - tip, y,
                         x + w - tip, y + h, color);
}

void drawVerticalSegment(
    TFT_eSPI& display,
    int x,
    int y,
    int w,
    int h,
    uint16_t color
)
{
    constexpr int tip = 5;
    display.fillRect(x, y + tip, w, h - 2 * tip, color);
    display.fillTriangle(x + w / 2, y, x, y + tip, x + w, y + tip, color);
    display.fillTriangle(x, y + h - tip, x + w, y + h - tip,
                         x + w / 2, y + h, color);
}

void drawDigit(TFT_eSPI& display, int x, int y, int number)
{
    display.fillRect(x - 2, y - 2, DIGIT_W + 4, DIGIT_H + 4, COLOR_BG);
    if (number < 0 || number > 9)
        return;

    const int verticalH = 52;
    const int middleY = y + 59;
    const int bottomY = y + DIGIT_H - SEG_THICK;
    const int leftX = x;
    const int rightX = x + DIGIT_W - SEG_THICK;

    if (digitSegments[number][0])
        drawHorizontalSegment(display, x + 6, y, DIGIT_W - 12, SEG_THICK, COLOR_TIME);
    if (digitSegments[number][1])
        drawVerticalSegment(display, rightX, y + 6, SEG_THICK, verticalH, COLOR_TIME);
    if (digitSegments[number][2])
        drawVerticalSegment(display, rightX, y + 72, SEG_THICK, verticalH, COLOR_TIME);
    if (digitSegments[number][3])
        drawHorizontalSegment(display, x + 6, bottomY, DIGIT_W - 12, SEG_THICK, COLOR_TIME);
    if (digitSegments[number][4])
        drawVerticalSegment(display, leftX, y + 72, SEG_THICK, verticalH, COLOR_TIME);
    if (digitSegments[number][5])
        drawVerticalSegment(display, leftX, y + 6, SEG_THICK, verticalH, COLOR_TIME);
    if (digitSegments[number][6])
        drawHorizontalSegment(display, x + 6, middleY, DIGIT_W - 12, SEG_THICK, COLOR_TIME);
}
}

void resetClockDisplay()
{
    for (int& digit : lastDigits)
        digit = -1;
    lastDay = -1;
}

void updateClockDisplay(TFT_eSPI& display, const struct tm& timeinfo, bool use24Hour)
{
    int hour = timeinfo.tm_hour;
    if (!use24Hour)
    {
        hour %= 12;
        if (hour == 0)
            hour = 12;
    }

    const int digits[4] = {
        (!use24Hour && hour < 10) ? -1 : hour / 10,
        hour % 10,
        timeinfo.tm_min / 10,
        timeinfo.tm_min % 10
    };

    for (int i = 0; i < 4; ++i)
    {
        if (digits[i] == lastDigits[i])
            continue;
        drawDigit(display, getDigitX(i), CLOCK_Y, digits[i]);
        lastDigits[i] = digits[i];
    }
}

void drawClockColon(TFT_eSPI& display, bool visible)
{
    display.fillRect(COLON_X - 5, CLOCK_Y, 30, DIGIT_H, COLOR_BG);
    if (!visible)
        return;
    display.fillCircle(COLON_X + 5, CLOCK_Y + 43, 7, COLOR_TIME);
    display.fillCircle(COLON_X + 5, CLOCK_Y + 88, 7, COLOR_TIME);
}

void updateDateDisplay(TFT_eSPI& display, const struct tm& timeinfo)
{
    if (timeinfo.tm_mday == lastDay ||
        timeinfo.tm_wday < 0 || timeinfo.tm_wday >= 7 ||
        timeinfo.tm_mon < 0 || timeinfo.tm_mon >= 12)
        return;

    lastDay = timeinfo.tm_mday;
    display.fillRect(0, 185, 480, 55, COLOR_BG);
    char dateText[32];
    snprintf(dateText, sizeof(dateText), "%d %s %d",
             timeinfo.tm_mday, months[timeinfo.tm_mon], timeinfo.tm_year + 1900);
    display.setTextColor(COLOR_TIME, COLOR_BG);
    constexpr uint8_t dateTextSize = 3;
    display.setTextFont(1);
    display.setTextSize(dateTextSize);
    display.setCursor(
        MAIN_CONTENT_CENTER_X -
            albanianTextWidth(days[timeinfo.tm_wday], dateTextSize) / 2,
        184
    );
    drawAlbanianText(display, days[timeinfo.tm_wday], dateTextSize);
    display.setCursor(
        MAIN_CONTENT_CENTER_X - albanianTextWidth(dateText, dateTextSize) / 2,
        208
    );
    drawAlbanianText(display, dateText, dateTextSize);
    display.setTextSize(1);
}
