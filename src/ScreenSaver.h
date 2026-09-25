#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <Arduino.h>
#include <TFT_eSPI.h>

void initScreenSaver(TFT_eSPI* display);
void updateScreenSaver(bool active);

#endif