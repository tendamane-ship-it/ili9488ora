# Referenca e pajisjeve të ORA-s

[Shqip](HARDWARE.sq.md) | [English](HARDWARE.md)

## Komponentët kryesorë

- ESP32-S3 DevKitC-1
- Ekran TFT SPI ILI9488, 3.95/4.0 inç, 480 × 320
- Kontrollues prekjeje rezistive XPT2046
- Sensor mjedisor BME680 me Bosch BSEC2
- Sensor ndriçimi BH1750
- Sensor lëvizjeje PIR

Integrimi opsional në rrjet përfshin Wi-Fi dhe pajisje të përputhshme me Tuya Cloud.

## Harta e GPIO-ve

| Sinjali / moduli | GPIO ESP32-S3 |
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
| Dalja PIR | 42 |
| Drita e pasme TFT (BL) | 15 |

Ekrani dhe kontrolluesi i prekjes përdorin SPI. Sensorët mjedisorë dhe ai i ndriçimit ndajnë të njëjtin bus I2C.

## Konfigurimi i ekranit

Projekti konfiguron TFT_eSPI përmes `include/User_Setup.h`:
- Driver: ILI9488
- Konstantet e rezolucionit: 320 × 480 (orientimi i bibliotekës; ndërfaqja paraqitet horizontalisht)
- Frekuenca SPI e konfiguruar: 27 MHz
- Frekuenca SPI për touch: 2.5 MHz

Kontrolloni etiketat e pinave në pllakën ESP32-S3 dhe në ekran përpara lidhjes. Variantet e pllakave dhe moduleve mund të ndryshojnë.

## Drita e pasme

Kontrolli i dritës së pasme përdor GPIO 15. Firmware-i përdor PWM 0–255. Ndriçimi automatik bazohet te BH1750 dhe ndryshon gradualisht; ofrohet edhe kontrolli manual. Kërkesat elektrike ndryshojnë sipas modulit të ekranit. Mos furnizoni një LED backlight me rrymë të lartë drejtpërdrejt nga GPIO; përdorni një driver ose qark tranzistori të përshtatshëm kur nevojitet.

## Shënime për lidhjet

- Verifikoni tensionin e furnizimit dhe nivelet logjike të çdo moduli.
- Lidhni tokëzimet (GND) së bashku.
- Mbajini kabllot SPI të shkurtra dhe të qëndrueshme.
- Jo çdo modul ILI9488 ka të njëjtat pina ose të njëjtin qark për dritën e pasme.
- Nëse sensorët nuk zbulohen, verifikoni adresat I2C të moduleve konkrete.

## Fotot dhe skema e lidhjeve

Fotot e kutisë së përfunduar dhe skema e verifikuar e lidhjeve do të shtohen pasi të përfundojë montimi fizik. Deri atëherë nuk paraqitet asnjë ilustrim si skemë e verifikuar e pajisjes.
