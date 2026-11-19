# ORA Features

[Shqip](FEATURES.sq.md) | [English](FEATURES.md)

## Clock and display

- Digital clock with NTP synchronization
- 12/24-hour display setting
- Albanian date presentation
- Touch-operated menu, settings and About screen
- Weather icon preview gallery

## Environmental monitoring

- BME680 temperature, relative humidity, pressure and gas-resistance readings
- Bosch BSEC2 Indoor Air Quality (IAQ) output and accuracy status
- BH1750 ambient illuminance in lux
- Sensor availability indicators

Gas resistance and IAQ are environmental indicators, not a direct CO₂ measurement or a certified safety reading.

## Weather

- Current conditions and 7-day forecast from Open-Meteo
- Weather icons for common conditions
- Animated weather elements for selected conditions
- Weather information configured for Sarandë, Albania

## Presence and display behavior

- PIR-based motion detection
- Screen saver after inactivity
- Optional welcome greeting
- Night mode using ambient-light thresholds
- Automatic or manual backlight level with gradual PWM transitions

## Connectivity and integrations

- Wi-Fi station connection with DHCP
- Setup access point and captive-style local configuration page
- Automatic reconnect attempts
- Local web dashboard and JSON status endpoints
- Optional Tuya Cloud device data

## Diagnostics

- Recent boot/reset history stored in NVS
- Runtime status indicators for Wi-Fi, NTP, sensors and network services
- Serial logging for setup and troubleshooting

Feature availability depends on the connected hardware, network access and configuration.
