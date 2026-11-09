#include "DisplayText.h"

namespace
{
uint16_t nextAlbanianGlyph(const char*& text)
{
    const uint8_t first = static_cast<uint8_t>(*text++);
    if (first < 0x80)
        return first;

    if (first == 0xC3 &&
        static_cast<uint8_t>(*text) >= 0x80 &&
        static_cast<uint8_t>(*text) <= 0xBF)
    {
        const uint8_t second = static_cast<uint8_t>(*text++);
        const uint16_t codePoint =
            static_cast<uint16_t>(((first & 0x1F) << 6) | (second & 0x3F));

        switch (codePoint)
        {
            case 0x00C7: return 0x80;
            case 0x00CB: return 0xCB;
            case 0x00E7: return 0x87;
            case 0x00EB: return 0x89;
            default: return '?';
        }
    }

    return '?';
}
}

int16_t albanianTextWidth(const char* text, uint8_t size)
{
    if (text == nullptr)
        return 0;

    int16_t glyphCount = 0;
    while (*text)
    {
        nextAlbanianGlyph(text);
        ++glyphCount;
    }

    return glyphCount * 6 * size;
}

int16_t drawAlbanianText(
    TFT_eSPI& display,
    const char* text,
    uint8_t size
)
{
    if (text == nullptr)
        return 0;

    if (size == 0)
        size = 1;

    const int16_t startX = display.getCursorX();
    int16_t cursorX = startX;
    int16_t cursorY = display.getCursorY();
    const uint8_t previousSize = display.textsize;
    const uint8_t previousCodePage = display.getAttribute(CP437_SWITCH);

    display.setTextSize(size);
    display.setAttribute(CP437_SWITCH, true);

    const char* current = text;
    while (*current)
    {
        const uint16_t glyph = nextAlbanianGlyph(current);
        if (glyph == '\n')
        {
            cursorX = startX;
            cursorY += 8 * size;
            continue;
        }

        cursorX += display.drawChar(glyph, cursorX, cursorY, 1);
    }

    display.setCursor(cursorX, cursorY);
    display.setAttribute(CP437_SWITCH, previousCodePage);
    display.setTextSize(previousSize);
    return cursorX - startX;
}
