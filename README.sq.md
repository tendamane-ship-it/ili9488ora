[English documentation](README.md) | **Shqip**

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
- Nëse mungojnë kredencialet Wi-Fi të ruajtura ose lidhja me routerin dështon, pajisja hap portalin e konfigurimit.
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
pasme (BL) lidhet te GPIO 15.

### Kontrolli i dritës së pasme

Te **Cilësimet → Drita e pasme**, zgjidh kontrollin automatik me BH1750 ose
manual dhe cakto nivelin `0`–`255`. Automatikja fik BL kur aktivizohet Night
Mode dhe e ndez kur sensori raporton dritë të mjaftueshme. Nëse BH1750 dështon
në modalitetin automatik, ora del nga Night Mode dhe e vendos BL në `255` si
masë rezervë. Modaliteti manual anashkalon BH1750 dhe Night Mode; niveli manual
mund të vendoset nga `0` deri në `255` (fillimisht `255`), ku `0` e fik BL.

Kur sensori funksionon dhe Night Mode nuk e ka fikur ekranin, modaliteti
automatik përdor këto nivele PWM:

| Ndriçimi BH1750 | PWM i dritës së pasme |
|---|---:|
| Nën 20 lux | 32 |
| 20–99 lux | 64 |
| 100–299 lux | 112 |
| 300–799 lux | 180 |
| 800 lux e lart | 255 |

Kalimi mes niveleve zbutet me rritje/ulje graduale të PWM-së. Këto janë vlera
fillestare praktike; mund të kalibrohen sipas ekranit dhe ndriçimit të dhomës.

### Testi i dritës së pasme

Testi ngarkohet veçmas dhe nuk përfshin kodin e orës. Zgjidh mjedisin
`esp32-s3-backlight-test` në PlatformIO dhe ekzekuto **Build**, pastaj **Upload**.
Hap Serial Monitor me shpejtësi `115200` dhe dërgo komandat me rresht të ri:

- `0`–`255`: cakton ndriçimin, ku `0` fikur dhe `255` maksimum.
- `a`: nis ose ndal ciklin automatik të ndriçimit.
- `s`: ndal ciklin dhe mban ndriçimin aktual.
- `?`: shfaq ndihmën.

Programi nis me ndriçim `128/255`. Për ndërtim nga terminali përdor
`pio run -e esp32-s3-backlight-test`; për ngarkim shto
`-t upload`.

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
  Monitor pas nisjes. Paneli web nuk ka aktualisht autentikim me përdorues dhe
  fjalëkalim: pajisjet në të njëjtin LAN mund të shohin statusin dhe të përdorin
  funksionet e cilësimeve ose rinisjes. Mbaje vetëm në rrjet të besuar dhe mos e
  ekspozo drejtpërdrejt në internet.

## Shënim për BME680

Rezistenca e gazit e BME680 është një tregues relativ i përbërjeve organike
të paqëndrueshme (VOC); nuk është matje e drejtpërdrejtë e CO₂ dhe nuk duhet
përdorur si pajisje sigurie.

BSEC2 raporton IAQ në shkallën 0–500 bashkë me saktësinë 0–3. Vlerat me
saktësi 0 shënohen si në kalibrim; sensorit mund t'i duhen deri në 24 orë
kondicionim dhe algoritmi përshtatet me historikun e mjedisit. Gjendja e
algoritmit ruhet lokalisht në NVS çdo 6 orë. Biblioteka BME68x dhe BSEC2
janë komponentë të palëve të treta me kushte licencimi të veçanta. Ky
kodi origjinal dhe dokumentacioni i projektit ORA publikohen me licencën MIT
(shih skedarin [`LICENSE`](LICENSE)). Kjo licencë nuk zëvendëson dhe nuk
ndryshon licencat e komponentëve të palëve të treta. Në veçanti, kodi,
libraritë dhe binarët Bosch BME68x/BSEC2 mbeten subjekt i kushteve të tyre
përkatëse. Kontrolloni licencën Bosch BSEC para rishpërndarjes së firmware-it.
