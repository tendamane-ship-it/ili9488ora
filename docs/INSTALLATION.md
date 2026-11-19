# Installation and Firmware Upload

[Shqip](INSTALLATION.sq.md) | [English](INSTALLATION.md)

## Requirements

- ESP32-S3 DevKitC-1 and the hardware listed in [HARDWARE.md](HARDWARE.md)
- Visual Studio Code
- PlatformIO IDE extension
- USB data cable
- Wi-Fi network
- Tuya developer credentials only if the Tuya integration is required

## Get the source

Clone this repository and open the project folder in VS Code:

```bash
git clone https://github.com/tendamane-ship-it/ili9488ora.git
cd ili9488ora
```

Select the `esp32-s3-devkitc-1` PlatformIO environment for the main firmware.

## Optional Tuya credentials

The repository includes `src/secrets.example.h`. Create a local private copy:

```powershell
Copy-Item src\secrets.example.h src\secrets.h
```

Add your own Tuya Client ID, Client Secret and Device ID to `src/secrets.h`. Do not commit this file or put real credentials in the example template. Tuya integration can be left unused if you do not have credentials.

## Build and upload

1. Connect the ESP32-S3 over USB.
2. Check `upload_port` and `monitor_port` in `platformio.ini`; the checked-in configuration currently specifies COM4.
3. Build the `esp32-s3-devkitc-1` environment.
4. Upload the firmware.
5. Open Serial Monitor at 115200 baud.

Command-line equivalents:

```bash
pio run -e esp32-s3-devkitc-1
pio run -e esp32-s3-devkitc-1 -t upload
pio device monitor -b 115200
```

The actual serial port may differ on your computer.

## First Wi-Fi setup

If saved credentials are missing or the connection attempt fails, the firmware starts a setup access point. The current source defines its SSID as `ili9488ora-setup`; the temporary password is generated at runtime and printed to Serial Monitor. Connect your phone or computer to that access point and open:

```
http://192.168.4.1
```

Enter the home router SSID and password. The device stores the credentials locally and attempts to connect. Once connected, it obtains its LAN IP address through DHCP.

## Open the local dashboard

Find the ESP32 IP address in Serial Monitor or the router's connected-device list. Open that IP from a browser on the same local network. The IP may change when DHCP assigns a new lease.

Do not forward the dashboard port to the public internet. Use a secure VPN if remote access is needed.

## Separate backlight test

PlatformIO also defines `esp32-s3-backlight-test`, which builds the separate backlight test program rather than the clock firmware:

```bash
pio run -e esp32-s3-backlight-test
pio run -e esp32-s3-backlight-test -t upload
``

## Troubleshooting

- **No serial output:** check the USB data cable, selected COM port and 115200 baud.
- **Build cannot find a library:** allow PlatformIO to finish dependency installation and retry.
- **Wi-Fi setup portal appears:** verify the router credentials and signal; the portal is the current recovery/configuration path.
- **A sensor is not detected:** verify power, GND, SDA/SCL and the module's actual I2C address.
- **Touch is inaccurate:** verify XPT2046 wiring and calibration values for the specific panel.

Never publish Wi-Fi passwords, Tuya secrets, access tokens or private network details.
