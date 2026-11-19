# ORA — ESP32-S3 Smart Clock

ORA is an ESP32-S3-based smart clock and compact information display built around an ILI9488 touchscreen. It presents time, date, weather and environmental sensor data, with a local web dashboard for status and configuration.

**Languages:** [English](README.md) | [Shqip](README.sq.md)

## Highlights

- NTP-synchronized clock with 12/24-hour display option and Albanian date presentation.
- ILI9488 TFT display with XPT2046 touch input, on-screen menu and About page.
- Current weather and 7-day forecast from Open-Meteo, configured for Sarandë, Albania.
- Weather icon gallery and selected animated weather elements.
- BME680 environmental measurements with Bosch BSEC2 IAQ output.
- BH1750 ambient-light measurement and automatic/manual backlight control.
- PIR motion detection, inactivity screen saver and optional welcome greeting.
- Night mode based on ambient-light thresholds.
- Wi-Fi setup portal, DHCP connectivity and automatic reconnect attempts.
- Local web dashboard, JSON status endpoints and recent boot/reset diagnostics.
- Optional Tuya Cloud device integration.

## Documentation

| Guide | English | Shqip |
|---|---|---|
| Hardware and GPIO | [HARDWARE.md](docs/HARDWARE.md) | [HARDWARE.sq.md](docs/HARDWARE.sq.md) |
| Installation | [INSTALLATION.md](docs/INSTALLATION.md) | [INSTALLATION.sq.md](docs/INSTALLATION.sq.md) |
| Software architecture | [SOFTWARE.md](docs/SOFTWARE.md) | [SOFTWARE.sq.md](docs/SOFTWARE.sq.md) |
| Features | [FEATURES.md](docs/FEATURES.md) | [FEATURES.sq.md](docs/FEATURES.sq.md) |

## Hardware at a glance

| Component / signal | ESP32-S3 |
|---|---:|
| ILI9488 MOSI / MISO / SCLK | GPIO 7 / 5 / 4 |
| ILI9488 CS / DC / RESET | GPIO 6 / 2 / 3 |
| XPT2046 Touch CS / IRQ | GPIO 17 / 14 |
| I2C SDA / SCL (BME680, BH1750) | GPIO 8 / 9 |
| PIR output | GPIO 42 |
| TFT backlight control | GPIO 15 |

See the [hardware guide](docs/HARDWARE.md) for wiring notes and electrical precautions.

## Build and upload

The project uses PlatformIO with the Arduino framework. Clone the repository, open it in VS Code with PlatformIO IDE, select the `esp32-s3-devkitc-1` environment, then build and upload.

```bash
git clone https://github.com/tendamane-ship-it/ili9488ora.git
cd ili9488ora
pio run -e esp32-s3-devkitc-1
pio run -e esp32-s3-devkitc-1 -t upload
```

The checked-in `platformio.ini` currently specifies COM4 for upload and serial monitoring; change it to the port assigned by your computer.

## Wi-Fi and local dashboard

If saved Wi-Fi credentials are unavailable or the connection attempt fails, the firmware starts the setup access point `ili9488ora-setup`. The temporary password is generated at runtime and printed to Serial Monitor at 115200 baud. Connect to the access point and open `http://192.168.4.1` to enter your router credentials.

After the ESP32 joins the LAN, open its DHCP-assigned IP address in a browser on the same local network. The address can change when the router renews the DHCP lease.

**Security:** the dashboard is intended for a trusted local network. Do not expose it directly to the public internet. Never commit Wi-Fi credentials, Tuya secrets or access tokens.

## Sensors and measurement limitations

The BME680 gas-resistance value is a relative indicator of volatile organic compounds (VOC), not a direct CO₂ measurement. BSEC2 IAQ is an algorithmic air-quality estimate and may require conditioning time. ORA is not a certified air-quality or life-safety device.

## Project photos

Photos of the completed enclosure, assembled device and verified wiring will be added after the physical build is finished.

## Third-party software and licensing

This project depends on third-party libraries, including Bosch BME68x and BSEC2 components. The Bosch wrapper library and BSEC binary components may be subject to different license terms. Review and comply with Bosch's applicable BSEC license before redistributing firmware binaries. A repository-wide project license should be selected and added separately; do not assume third-party dependencies are covered by it.

## Contributing

Issues, corrections and build notes from other makers are welcome. Please do not include private credentials, personal network details or secrets in issues and pull requests.
