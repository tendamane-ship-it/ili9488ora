# Instalimi dhe ngarkimi i firmware-it

[Shqip](INSTALLATION.sq.md) | [English](INSTALLATION.md)

## Kërkesat

- ESP32-S3 DevKitC-1 dhe pajisjet e listuara te [HARDWARE.sq.md](HARDWARE.sq.md)
- Visual Studio Code
- Zgjerimi PlatformIO IDE
- Kabllo USB që mbështet transferimin e të dhënave
- Rrjet Wi-Fi
- Kredenciale Tuya vetëm nëse kërkohet integrimi Tuya

## Shkarkimi i kodit

Klononi repository-n dhe hapni dosjen në VS Code:

```bash
git clone https://github.com/tendamane-ship-it/ili9488ora.git
cd ili9488ora
```

Për firmware-in kryesor zgjidhni mjedisin PlatformIO `esp32-s3-devkitc-1`.

## Kredencialet opsionale Tuya

Repository përmban modelin `src/secrets.example.h`. Krijoni kopjen lokale:

```powershell
Copy-Item src\secrets.example.h src\secrets.h
```

Vendosni Client ID, Client Secret dhe Device ID tuaj në `src/secrets.h`. Mos e publikoni këtë skedar dhe mos vendosni kredenciale reale në modelin shembull. Nëse nuk përdorni Tuya, ky integrim mund të lihet pa konfiguruar.

## Build dhe ngarkimi

1. Lidhni ESP32-S3 me USB.
2. Kontrolloni `upload_port` dhe `monitor_port` te `platformio.ini`; konfigurimi aktual në repository përdor COM4.
3. Ndërtoni mjedisin `esp32-s3-devkitc-1`.
4. Ngarkoni firmware-in.
5. Hapni Serial Monitor me shpejtësi 115200 baud.

Komandat nga terminali:

```bash
pio run -e esp32-s3-devkitc-1
pio run -e esp32-s3-devkitc-1 -t upload
pio device monitor -b 115200
```

Porta seriale mund të jetë ndryshe në kompjuterin tuaj.

## Konfigurimi i parë i Wi-Fi

Nëse mungojnë kredencialet e ruajtura ose lidhja dështon, firmware-i nis një pikë aksesi konfigurimi. SSID-ja e përcaktuar në kod është `ili9488ora-setup`; fjalëkalimi i përkohshëm krijohet gjatë nisjes dhe shfaqet në Serial Monitor. Lidheni telefonin ose kompjuterin me këtë rrjet dhe hapni:

```
http://192.168.4.1
```

Vendosni emrin dhe fjalëkalimin e routerit. Pajisja i ruan lokalisht dhe provon të lidhet. Pas lidhjes, merr adresë IP në rrjetin lokal përmes DHCP.

## Hapja e panelit web

Gjeni IP-në e ESP32-së në Serial Monitor ose në listën e pajisjeve të lidhura në router. Hapeni nga shfletuesi i një pajisjeje në të njëjtin rrjet lokal. IP-ja mund të ndryshojë kur routeri cakton një adresë të re përmes DHCP.

Paneli nuk ka aktualisht autentikim me përdorues/fjalëkalim; pajisjet në të njëjtin LAN mund të shohin statusin dhe të përdorin cilësimet ose rinisjen. Mbajeni në rrjet të besuar. Mos e ekspozoni portin drejtpërdrejt në internet; për akses nga distanca përdorni VPN të sigurt.

## Testi i veçantë i dritës së pasme

PlatformIO përmban edhe mjedisin `esp32-s3-backlight-test`, i cili ndërton programin e veçantë të testimit të backlight-it dhe jo firmware-in e orës:

```bash
pio run -e esp32-s3-backlight-test
pio run -e esp32-s3-backlight-test -t upload
```

## Zgjidhja e problemeve

- **Nuk ka dalje seriale:** kontrolloni kabllon USB, portën COM dhe shpejtësinë 115200.
- **Build nuk gjen një bibliotekë:** prisni instalimin e varësive nga PlatformIO dhe provoni përsëri.
- **Shfaqet portali Wi-Fi:** kontrolloni kredencialet dhe sinjalin e routerit.
- **Sensori nuk zbulohet:** kontrolloni furnizimin, GND, SDA/SCL dhe adresën I2C të modulit.
- **Touch nuk është i saktë:** kontrolloni lidhjet XPT2046 dhe kalibrimin e panelit konkret.

Mos publikoni fjalëkalime Wi-Fi, sekrete Tuya, tokenë aksesi ose të dhëna private të rrjetit.
