# ORA Clock me ESP32-S3

ORA është një orë dhe panel informues i ndërtuar me ESP32-S3 dhe ekran me prekje ILI9488. Shfaq orën, datën, motin dhe të dhënat e sensorëve, si dhe ofron një panel web lokal për statusin dhe konfigurimet.

> Dokumentacioni në anglisht: [README.md](README.md).

## Veçoritë kryesore

- Ora dhe data sinkronizohen me NTP.
- Data shfaqet në shqip.
- Moti aktual dhe parashikimi 7-ditor merren nga Open-Meteo për Sarandën.
- Ikonat e motit përfshijnë diellin/hënën, retë, shiun, borën, mjegullën dhe vetëtimat; disa prej tyre animohen.
- Galeria **WEATHER ICONS** lejon shikimin e ikonave të motit.
- BME680 mat temperaturën, lagështinë, presionin dhe rezistencën e gazit; Bosch BSEC2 llogarit IAQ.
- BH1750 mat ndriçimin e ambientit.
- Tuya Cloud shfaq të dhënat e pajisjes së konfiguruar.
- PIR, kursyesi i ekranit, modaliteti i natës dhe ndriçimi automatik/manuel i ekranit.
- Portal Wi-Fi për konfigurimin e rrjetit.
- Panel web lokal me statusin, konfigurimet dhe diagnostikimin e nisjeve.

## Dokumentacioni

- [Pajisjet dhe lidhjet GPIO](docs/HARDWARE.md)
- [Instalimi dhe ngarkimi i firmware-it](docs/INSTALLATION.md)
- [Arkitektura e softuerit](docs/SOFTWARE.md)
- [Funksionet e ORA-s](docs/FEATURES.md)

## Pajisjet dhe lidhjet

| Pajisja | ESP32-S3 |
|---|---|
| ILI9488 MOSI | GPIO 7 |
| ILI9488 MISO | GPIO 5 |
| ILI9488 SCLK | GPIO 4 |
| ILI9488 CS | GPIO 6 |
| ILI9488 DC | GPIO 2 |
| ILI9488 RST | GPIO 3 |
| XPT2046 Touch CS | GPIO 17 |
| XPT2046 Touch IRQ | GPIO 14 |
| I2C SDA (BME680, BH1750) | GPIO 8 |
| I2C SCL (BME680, BH1750) | GPIO 9 |
| PIR | GPIO 42 |
| Backlight (BL) | GPIO 15 |

Kontrolloni pinout-in e pllakës dhe të moduleve përpara lidhjes. Shihni [HARDWARE.md](docs/HARDWARE.md) për hollësi.

## Përgatitja e projektit

1. Instaloni VS Code dhe PlatformIO IDE.
2. Klononi repository-n dhe hapni dosjen në VS Code.
3. Kopjoni modelin e kredencialeve Tuya:

   ```powershell
   Copy-Item src\secrets.example.h src\secrets.h
   ```

4. Plotësoni Client ID, Client Secret dhe Device ID në `src/secrets.h`, nëse do të përdorni integrimin Tuya.
5. Zgjidhni portën COM të pajisjes në `platformio.ini`, nëse është e nevojshme.
6. Ekzekutoni **Build**, pastaj **Upload** nga PlatformIO.

Udhëzimet e plota gjenden te [INSTALLATION.md](docs/INSTALLATION.md).

## Paneli web dhe Wi-Fi

Nëse pajisja nuk ka kredenciale Wi-Fi të ruajtura ose nuk arrin të lidhet, firmware-i mund të hapë portalin e konfigurimit. SSID-ja dhe fjalëkalimi i përkohshëm shfaqen në Serial Monitor (115200 baud). Lidheni telefonin me pikën e aksesit dhe hapni `http://192.168.4.1`.

Pas lidhjes me routerin, hapni adresën IP lokale të ESP32-së në shfletues nga një pajisje në të njëjtin rrjet. IP-ja merret me DHCP dhe mund të ndryshojë.

**Siguria:** paneli është menduar për rrjetin lokal. Mos e ekspozoni drejtpërdrejt në internet dhe mos publikoni kredenciale Wi-Fi ose Tuya.

## Shënim për sensorin BME680

Rezistenca e gazit është tregues relativ i përbërjeve organike të paqëndrueshme (VOC), jo matje e drejtpërdrejtë e CO₂. IAQ nuk është matje mjekësore dhe pajisja nuk duhet përdorur si alarm apo pajisje sigurie. BSEC2 mund të kërkojë periudhë kondicionimi dhe ruan gjendjen e algoritmit në NVS.

## Fotot e projektit

Fotot reale të pajisjes së përfunduar, kutisë dhe montimit do të shtohen pas përfundimit të kutisë.

## Licencimi dhe komponentët e palëve të treta

Kontrolloni kushtet e licencës së Bosch BSEC2 përpara rishpërndarjes së firmware-it. Wrapper-i i bibliotekës Bosch dhe binarët BSEC mund të kenë kushte të ndryshme licencimi.
