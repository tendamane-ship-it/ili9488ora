#ifndef WEATHER_DISPLAY_H
#define WEATHER_DISPLAY_H

#include <TFT_eSPI.h>
#include "WeatherService.h"

void drawWeatherDisplay(TFT_eSPI& display, const WeatherData& data);
void updateWeatherAnimations(
    TFT_eSPI& display,
    const WeatherData& data,
    bool weatherPanelVisible
);

#endif
