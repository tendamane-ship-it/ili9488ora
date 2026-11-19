# Software Architecture

[Shqip](SOFTWARE.sq.md) | [English](SOFTWARE.md)

## Platform

The firmware is written for the Arduino framework on ESP32-S3 and is built with PlatformIO. The display uses TFT_eSPI; touch input uses XPT2046_Touchscreen. Environmental sensing uses Bosch BME68x/BSEC2 and BH1750 libraries.

## Main runtime responsibilities

The main application coordinates:
- TFT initialization and clock/menu/about screens
- Touch input and UI state
- NTP time synchronization
- Wi-Fi setup and reconnection
- Local sensor readings and backlight control
- Screen saver and PIR activity
- Weather and Tuya service status

The main loop also rotates the lower information area between local sensor values, Tuya data and weather content. The configured lower-panel rotation interval is 10 seconds.

## Network services

Weather and Tuya operations are coordinated through a FreeRTOS task in `src/main.cpp`, separate from the interactive UI loop. The weather refresh interval is configured as five minutes. Tuya refresh is configured as 60 seconds in the current source.

Open-Meteo provides weather information. Tuya Cloud is optional and requires user-supplied credentials in the ignored local `src/secrets.h` file.

## Persistent settings and diagnostics

Preferences/NVS are used for device settings and stored Wi-Fi credentials. Reset diagnostics retain a limited history of recent starts. BSEC2 state is persisted locally so the algorithm can continue adapting across restarts.

## Web interface

`WebServerManager` implements the local web interface and JSON status endpoints. The Wi-Fi setup portal is implemented separately by `WiFiManager`.

The dashboard currently has no user/password authentication. Any device with LAN access may view status and use settings or restart endpoints. Treat it as a local administration interface on a trusted network; do not expose it directly to the public internet.

## Memory-aware display behavior

The screen saver uses a sprite buffer. The implementation checks free heap and skips or releases the sprite when memory protection thresholds are reached, preserving resources for the rest of the firmware.

## Source layout

The source tree is organized into modules for clock display, pages, sensors, weather, Tuya, Wi-Fi, web server, screen saver, startup, settings, diagnostics and serial logging. Exact module boundaries may evolve as the project develops.
