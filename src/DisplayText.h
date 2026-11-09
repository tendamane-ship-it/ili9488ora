#ifndef DISPLAY_TEXT_H
#define DISPLAY_TEXT_H

#include <TFT_eSPI.h>

int16_t albanianTextWidth(const char* text, uint8_t size = 1);
int16_t drawAlbanianText(
    TFT_eSPI& display,
    const char* text,
    uint8_t size = 1
);

#endif
