# Arkitektura e softuerit

[Shqip](SOFTWARE.sq.md) | [English](SOFTWARE.md)

## Platforma

Firmware-i është shkruar për framework-un Arduino në ESP32-S3 dhe ndërtohet me PlatformIO. Ekrani përdor TFT_eSPI; prekja përdor XPT2046_Touchscreen. Sensorët përdorin bibliotekat Bosch BME68x/BSEC2 dhe BH1750.

## Përgjegjësitë kryesore gjatë punës

Aplikacioni kryesor koordinon:
- Inicializimin e TFT-së dhe ekranet e orës, menysë dhe About
- Hyrjen me prekje dhe gjendjen e ndërfaqes
- Sinkronizimin e orës me NTP
- Konfigurimin dhe rilidhjen me Wi-Fi
- Leximet e sensorëve dhe kontrollin e dritës së pasme
- Kursyesin e ekranit dhe aktivitetin e PIR
- Statusin e shërbimeve të motit dhe Tuya

Loop-i kryesor alternon gjithashtu përmbajtjen e panelit të poshtëm mes sensorëve lokalë, të dhënave Tuya dhe motit. Intervali i konfigurimit për këtë alternim është 10 sekonda.

## Shërbimet e rrjetit

Shërbimet e motit dhe Tuya koordinohen përmes një task-u FreeRTOS në `src/main.cpp`, të ndarë nga loop-i që trajton ndërfaqen. Rifreskimi i motit është konfiguruar çdo pesë minuta. Rifreskimi Tuya është konfiguruar çdo 60 sekonda në kodin aktual.

Open-Meteo siguron të dhënat e motit. Tuya Cloud është opsional dhe kërkon kredenciale personale në skedarin lokal `src/secrets.h`, i cili përjashtohet nga Git.

## Cilësimet dhe diagnostikimi i ruajtur

Preferences/NVS përdoret për cilësimet e pajisjes dhe kredencialet Wi-Fi. Diagnostikimi i reset-eve ruan një histori të kufizuar të nisjeve të fundit. Gjendja e BSEC2 ruhet lokalisht që algoritmi të vazhdojë përshtatjen edhe pas rinisjes.

## Ndërfaqja web

`WebServerManager` zbaton ndërfaqen lokale web dhe endpoint-et JSON të statusit. Portali i konfigurimit Wi-Fi zbatohet veçmas nga `WiFiManager`.

Paneli nuk ka aktualisht autentikim me përdorues/fjalëkalim. Çdo pajisje me qasje në LAN mund të shohë statusin dhe të përdorë endpoint-et e cilësimeve ose rinisjes. Trajtojeni si ndërfaqe administrimi lokale në rrjet të besuar dhe mos e ekspozoni drejtpërdrejt në internet.

## Menaxhimi i memories

Kursyesi i ekranit përdor një buffer sprite. Implementimi kontrollon heap-in e lirë dhe shmang ose liron sprite-in kur arrihen pragjet e mbrojtjes së memories, për të ruajtur burimet për firmware-in.

## Organizimi i kodit

Struktura e kodit ndahet në module për ekranin e orës, faqet, sensorët, motin, Tuya, Wi-Fi, serverin web, kursyesin e ekranit, nisjen, cilësimet, diagnostikimin dhe log-et seriale. Kufijtë mes moduleve mund të ndryshojnë gjatë zhvillimit.
