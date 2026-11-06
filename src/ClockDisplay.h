#ifndef CLOCK_DISPLAY_H
#define CLOCK_DISPLAY_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <time.h>

void resetClockDisplay();
void updateClockDisplay(TFT_eSPI& display, const struct tm& timeinfo, bool use24Hour);
void drawClockColon(TFT_eSPI& display, bool visible);
void updateDateDisplay(TFT_eSPI& display, const struct tm& timeinfo);

#endif
