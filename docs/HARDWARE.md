# ORA Hardware Reference

[Shqip](HARDWARE.sq.md) | [English](HARDWARE.md)

## Main components

- ESP32-S3 DevKitC-1
- 3.95/4.0-inch ILI9488 SPI TFT display, 480 × 320
- XPT2046 resistive touch controller
- BME680 environmental sensor with Bosch BSEC2
- BH1750 ambient light sensor
- PIR motion sensor

Optional network integration: Wi-Fi, Tuya-compatible device through Tuya Cloud.

## GPIO map

| Signal / module | ESP32-S3 GPIO |
|---|---:|
| TFT MOSI | 7 |
| TFT MISO | 5 |
| TFT SCLK | 4 |
| TFT CS | 6 |
| TFT DC | 2 |
| TFT RESET | 3 |
| XPT2046 Touch CS | 17 |
| XPT2046 Touch IRQ | 14 |
| I2C SDA (BME680 + BH1750) | 8 |
| I2C SCL (BME680 + BH1750) | 9 |
| PIR output | 42 |
| TFT backlight (BL) | 15 |

The display and touch controller use SPI. The environmental and light sensors share the I2C bus.

## Display configuration

The project configures TFT_eSPI through `include/User_Setup.h`:
- Driver: ILI9488
- Resolution constants: 320 × 480 (library orientation; the UI is presented in landscape)
- SPI clock configured in the project: 27 MHz
- Touch SPI clock: 2.5 MHz

Check the exact pin labels on your ESP32-S3 board and display breakout before wiring. Board variants and display modules may differ.

## Backlight

The backlight control output is GPIO 15. Firmware PWM range is 0–255. Automatic brightness uses BH1750 readings and gradual transitions; manual brightness is also available in settings. Electrical backlight requirements vary by display module. Do not power a high-current LED backlight directly from an ESP32 GPIO; use an appropriate driver/transistor circuit if the module requires it.

## Wiring notes

- Confirm supply voltage and logic-level requirements for every breakout board.
- Connect grounds together.
- Keep SPI wiring short and stable.
- Do not assume that every ILI9488 module exposes identical pins or includes the same backlight driver.
- Verify the I2C addresses from the actual sensor boards if detection fails.

## Photos and wiring diagram

Photos of the completed enclosure and a verified wiring diagram will be added after the physical build is finished. No illustrative wiring image is presented as a verified build at this stage.
