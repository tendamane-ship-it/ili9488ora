# Ora Clock me ESP32-S3

Ora është një orë dhe panel i vogël informues, i ndërtuar me ESP32-S3 dhe ekran
ILI9488. Shfaq orën, motin dhe të dhënat e sensorëve; ofron gjithashtu një
faqe lokale në rrjet për statusin dhe konfigurimet.

## Veçoritë

- Ora dhe data sinkronizohen me NTP.
- Data shfaq ditën e javës dhe datën në shqip, në dy rreshta për lexim më të
  qartë; teksti në ekran përdor fontin e integruar të pajisjes.
- Moti aktual dhe parashikimi 7-ditor merren nga Open-Meteo për Sarandën.
- Ikonat e motit përfshijnë diellin/hënën, retë, shiun, borën, mjegullën dhe
  vetëtimat; retë, pikat, flokët e borës dhe vetëtimat animohen sipas motit.
- Galeria **WEATHER ICONS** në menynë e ekranit lejon shikimin e ikonave pa
  pritur kushtet përkatëse të motit.
- Sensori BME680 mat temperaturën, lagështinë, presionin dhe rezistencën e
  gazit; Bosch BSEC2 llogarit indeksin IAQ dhe BH1750 mat ndriçimin.
- Lidhja me Tuya Cloud shfaq të dhënat e pajisjes së konfiguruar.
- PIR, kursyesi i ekranit, modaliteti i natës dhe paneli lokal web mund të
  përdoren sipas konfigurimit të projektit.
- Kur nuk ka kredenciale Wi-Fi të ruajtura, pajisja hap portalin e konfigurimit.
- Wi-Fi përdor DHCP për të marrë automatikisht adresën IP nga routeri.
- Shkaku dhe historia e tetë nisjeve të fundit ruhen në NVS dhe shfaqen te paneli
  web; mund të kontrollohen nga telefoni në të njëjtin rrjet. Regjistrohet
  arsyeja vetëm kur ESP32 arrin të nisë kodin pas reset-it; një dështim i
  furnizimit që pengon nisjen nuk mund të regjistrohet nga firmware-i.

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

Kontrollo gjithmonë pinout-in e pllakës dhe të moduleve para lidhjes. Drita e
pasme e ekranit (BL) e këtij konfigurimi nuk kontrollohet nga kodi.

## Përgatitja dhe ngarkimi

1. Instalo VS Code dhe zgjerimin PlatformIO IDE.
2. Klono projektin dhe hape dosjen në VS Code.
3. Krijo skedarin lokal të kredencialeve Tuya duke kopjuar modelin:

   ```powershell
   Copy-Item src\secrets.example.h src\secrets.h
   ```

4. Plotëso vlerat Tuya në `src/secrets.h`. Ky skedar përjashtohet nga Git; mos
   e publiko dhe mos i vendos kredencialet reale në `secrets.example.h`.
5. Zgjidh portën COM të ESP32-S3 te `platformio.ini` nëse nuk është `COM4`.
6. Nga PlatformIO ekzekuto **Build**, pastaj **Upload**.

Në nisjen e parë, lidhu me pikën e aksesit të konfigurimit Wi-Fi. Emri i saj
dhe fjalëkalimi i përkohshëm shfaqen në Serial Monitor me shpejtësi `115200`.
Pasi të lidhesh me pikën e aksesit, hap `http://192.168.4.1` në shfletues,
ruaj emrin dhe fjalëkalimin e routerit dhe ESP32 do t'i ruajë lokalisht.

## Konfigurimi i shërbimeve

- Koordinatat e Open-Meteo dhe zona kohore përcaktohen në kodin e shërbimit të
  motit.
- Tuya kërkon Client ID, Client Secret dhe Device ID në `src/secrets.h`.
- Pas lidhjes me Wi-Fi, adresa IP e ESP32-së përdoret për të hapur panelin web
  nga një pajisje në të njëjtin rrjet lokal. Adresa caktohet nga routeri me
  DHCP dhe mund të ndryshojë; kontrolloje te paneli i routerit ose në Serial
  Monitor pas nisjes.

## Shënim për BME680

Rezistenca e gazit e BME680 është një tregues relativ i përbërjeve organike
të paqëndrueshme (VOC); nuk është matje e drejtpërdrejtë e CO₂ dhe nuk duhet
përdorur si pajisje sigurie.

BSEC2 raporton IAQ në shkallën 0–500 bashkë me saktësinë 0–3. Vlerat me
saktësi 0 shënohen si në kalibrim; sensorit mund t'i duhen deri në 24 orë
kondicionim dhe algoritmi përshtatet me historikun e mjedisit. Gjendja e
algoritmit ruhet lokalisht në NVS çdo 6 orë. Biblioteka e wrapper-it Bosch
është BSD-3-Clause, ndërsa binarët BSEC mbulohen nga licenca e veçantë e
Bosch-it; pranoni kushtet e saj para rishpërndarjes së firmware-it.
