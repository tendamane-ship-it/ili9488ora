# ORA firmware: auditimi i API-së dhe konfigurimit të IP-së

## Fusha e auditimit

Auditimi është bërë mbi burimet e degës `copilot-help` për ESP32-S3 DevKitC-1-N8 me ekran ILI9488. Nuk u ndryshua kodi. Ky dokument përshkruan gjendjen që është në degë; nuk është provë e gjendjes live të pajisjes.

## Gjendja e IP-së statike

Në gjendjen aktuale të `src/WiFiManager.cpp`, klienti station lidhet me `WiFi.begin(...)` dhe DHCP. Nuk thirret `WiFi.config(...)`, prandaj IP-ja e ORA-s në rrjetin e routerit caktohet nga DHCP dhe mund të ndryshojë. Kredencialet ruhen në Preferences/NVS (`wifi`: `ssid`, `password`); `begin()` merr vlera bosh si parazgjedhje.

Kur lidhja dështon, pajisja aktivizon portalin AP `ili9488ora-setup`, me adresën `192.168.4.1/24`. Ky është IP i portalit lokal, jo IP statike e klientit në rrjetin e routerit.

Nuk ka ndryshime lokale të padorëzuara në `WiFiManager` ose konfigurim IP në këtë checkout. Historia e Git tregon se konfigurimi i mëparshëm `192.168.1.133`, gateway `192.168.1.1`, subnet `255.255.255.0` dhe DNS `192.168.1.1` u hoq në commit `c450e0b` (“Përdor DHCP dhe shfaq arsyet e reset-it në panelin web”). Ky auditim nuk e rikthen atë konfigurim, sepse do të ndryshonte funksionimin aktual. IP-ja, SSID-ja dhe MAC-i aktual nuk mund të konfirmohen pa lexuar pajisjen live.

## Endpoint-et ekzistuese

Serveri web funksionon në portën 80 kur pajisja është e lidhur me Wi-Fi. Rrugët kryesore regjistrohen në `src/WebServerManager.cpp`; rrugët OTA në `src/OtaManager.cpp`.

| Metoda dhe rruga | Përmbajtja / veprimi |
|---|---|
| `GET /` | Paneli i orës, HTML. |
| `GET /settings` | Faqja HTML e cilësimeve. |
| `POST /settings/save` | Ruajtje e cilësimeve në Preferences/NVS dhe ridrejtim te `/settings`. |
| `GET /tuya` | Statusi Tuya TH01, temperatura, lagështia dhe bateria, HTML. |
| `GET /weather` | Moti aktual dhe parashikimi 7-ditor, HTML. |
| `POST /restart` | Përgjigje tekstuale dhe rinisje e ESP32-s. |
| `GET /api/status` | JSON me identitet/version, orë/datë, rrjet, CPU, heap të lirë, flash total, uptime, reset reason/histori, cilësime, sensorë dhe pjesë të motit. |
| `GET /api/device` | JSON i përmbledhur: pajisja, modeli, versioni, gjendja Wi-Fi, IP, RSSI dhe uptime. |
| `GET /api/logs` | Regjistri serial i fundit, tekst i thjeshtë. |
| `GET /logs` | Faqe HTML e regjistrit, rifreskon `/api/logs` çdo 2 sekonda. |
| `GET /api` | Faqe HTML që shfaq `/api/status`, rifreskohet çdo 3 sekonda. |
| `GET /api/ota/challenge` | Challenge për çiftim OTA. |
| `GET /api/ota/pair/status` | Gjendja e çiftimit OTA. |
| `POST /api/ota/pair/claim` | Përfundon çiftimin me çelësin përkatës. |
| `POST /api/ota/check` | Kontrollon versionin/pajtueshmërinë OTA. |
| `POST /api/ota/start` | Nis marrjen e imazhit OTA; kthen madhësinë dhe gjendjen ose gabim. |
| `POST /api/ota/chunk` | Pranon blloqet e imazhit dhe kthen progresin/gabimin. |
| `POST /api/ota/finish` | Verifikon imazhin dhe përfundon instalimin. |
| `POST /api/ota/abort` | Anulon OTA-n aktive. |
| `GET /api/ota/status` | Gjendja/versioni, bajtet e marra, madhësia, maksimumi, gabimi dhe çiftimi OTA. |

Kur Wi-Fi nuk lidhet, `WiFiManager` nis portalin: `GET /` jep formularin, `POST /save` ruan kredencialet; rrugët e tjera ridrejtohen te `192.168.4.1`.

## Të dhënat që ekzistojnë dhe boshllëqet në API

### Diagnostika e pajisjes

`/api/status` kthen frekuencën CPU në MHz, heap-in momental të lirë, flash-in total, uptime dhe reset reason/historik (deri në 8 hyrje; numri total ruhet në NVS). Mungojnë minimumi i heap-it të lirë, blloku më i madh i lirë, chip model/revision, SDK version, madhësia/përdorimi i slotit aktiv dhe numri i bërthamave. `/api/device` kthen vetëm nënvargun e identitetit, rrjetit, RSSI-së dhe uptime-it.

### Wi-Fi

`/api/status` ekspozon SSID, gjendjen, IP dhe RSSI. `/api/device` jep gjendjen, IP dhe RSSI. MAC-i, kanali, gateway, subnet-i, mënyra e marrjes së IP-së dhe mosha e lidhjes mungojnë në API.

Konfigurimi i ndërtimit identifikon board-in si ESP32-S3 DevKitC-1-N8, 8 MB QD, pa PSRAM. Ky është konfigurim i build-it, jo lexim live i chip-it.

### Sensorët

- **BME680/BSEC2:** temperatura, lagështia, presioni, rezistenca e gazit (kΩ), IAQ, saktësia/etiketa IAQ, kategoria e ajrit, stabilization/run-in, flamujt e inicializimit/leximit dhe mosha e mostrës. Vlerat e pavlefshme kthehen `null`.
- **BH1750:** lux, vlefshmëria dhe mosha e mostrës.
- **PIR:** gjendja e filtruar/debounced dhe `pirMotion`; nuk ka histori ngjarjesh. PIR përdor GPIO 42 dhe debounce 150 ms.

Vlerat dhe leximet përpunohen në `src/Sensors.cpp`. BSEC ruan gjendjen e kalibrimit në Preferences, zakonisht çdo 6 orë; sensorët lokalë përditësohen në ciklin kryesor rreth çdo 5 sekonda.

### Shërbimet dhe ekrani

- **NTP:** serverët janë `pool.ntp.org`, `time.nist.gov`, `time.google.com`; gjendja e sinkronizimit mbahet në `timeSynchronized`, por nuk kthehet nga API-ja.
- **Moti:** Open-Meteo. `/api/status` jep temperaturë, lagështi, presion, erë, kod moti dhe `weatherValid`; mungojnë fusha të tjera aktuale dhe i gjithë parashikimi ditor. Të dhënat mbahen në RAM nga `WeatherService` me mutex; rifreskimi planifikohet rreth çdo 5 minuta.
- **Tuya:** `TuyaService` mban online, temperaturë, lagështi dhe bateri, por `/api/status` nuk i përfshin. Përditësimi planifikohet rreth çdo 60 sekonda. Faqja `/tuya` i shfaq këto vlera.
- **Ekrani:** nisja ka statuse për Wi-Fi, BME680, BH1750, NTP, motin dhe Tuya; modaliteti i ekranit/kursyesit/ndriçimit menaxhohet në `src/main.cpp`. Nuk ka endpoint të dedikuar që kthen statusin aktual të ekranit.
- **OTA:** ekziston grupi `/api/ota/*`; statusi nuk është i bashkuar me `/api/status`.

## Partition table dhe OTA

Partition table i build-it aktual është lexuar nga `.pio/build/esp32-s3-devkitc-1/partitions.bin`:

| Partition | Offset | Madhësia |
|---|---:|---:|
| NVS | `0x9000` | 20 KiB |
| OTA data | `0xE000` | 8 KiB |
| app0 | `0x10000` | 3264 KiB |
| app1 | `0x340000` | 3264 KiB |
| SPIFFS | `0x670000` | 1536 KiB |
| coredump | `0x7F0000` | 64 KiB |

Kjo ndarje përputhet me 8 MiB flash. `firmware.bin` ekzistues është 1,202,448 byte. Çdo slot aplikacioni është 3,342,336 byte, pra ka rreth 2,139,888 byte (rreth 2,090 KiB) hapësirë të papërdorur nga ky imazh deri në kufirin e slotit. OTA përdor partition-in pasiv dhe nuk duhet zvogëluar asnjëri slot.

SPIFFS ka 1536 KiB hapësirë të ndarë në flash. Në burimet e projektit nuk u gjet përdorim aktual i SPIFFS/LittleFS; montimi duhet verifikuar para përdorimit. NVS ruhet për cilësimet, kredencialet, diagnostikën e reset-it dhe gjendjen BSEC, jo për regjistrime të shpeshta.

## Projektimi i rekomanduar i API-së së zgjeruar

Ruaj endpoint-et ekzistuese për pajtueshmëri dhe shto rrugë të versionuara:

- `GET /api/v1/system`: chip/model/revision, SDK, CPU, heap aktual/minimal, largest free block, flash total, ndarja e app-it, uptime dhe reset reason.
- `GET /api/v1/network`: gjendja, SSID, IP, MAC, RSSI, channel dhe konfigurimi DHCP/AP. Kufizo SSID-në për klientë të autorizuar.
- `GET /api/v1/sensors/current`: BME680/BSEC, BH1750 dhe PIR me njësi, timestamp/moshë dhe flamuj `valid`.
- `GET /api/v1/services`: NTP, moti (vlefshmëri/ora e rifreskimit), Tuya, OTA dhe gjendja relevante e ekranit.
- `GET /api/v1/history?from=&to=&interval=`: histori me interval dhe limit të kontrolluar; përgjigje me faqe për diapazonet e gjata.

## Historiku 7-ditor

Rekomandohet skedar rrethor binar në SPIFFS me mostër të agreguar çdo 5 minuta: 2,016 pika për 7 ditë. Një rekord i rreshtuar 32-byte (timestamp, gjashtë vlera float: temperaturë, lagështi, presion, gaz, IAQ, lux, dhe flamuj vlefshmërie) zë 64,512 byte, afërsisht 63 KiB. Kodimi kompakt me vlera të shkallëzuara 16-bit mund ta ulë në rreth 35–40 KiB. Ngjarjet PIR mund të ruhen veçmas ose si evente.

Me mostra çdo minutë ka 10,080 pika në javë, rreth 315 KiB me rekord 32-byte. Kufizo dritaren në 7 ditë, valido CRC/metadata pas nisjes dhe grupo shkrimet për të ulur konsumimin e flash-it. Mos bëj shkrim për çdo mostër në NVS.

## Rreziqet dhe ndikimi

- Ndërtimi i përgjigjeve JSON me `String` konsumon heap; përgjigjet duhet të jenë me madhësi të kufizuar dhe historiku të lexohet me faqe/streaming.
- Leximet e historikut dhe gjendjet e shërbimeve duhet të sinkronizohen; WeatherService dhe TuyaService kanë mutex, ndërsa vlerat globale të sensorëve nuk kanë një shtresë të përbashkët sinkronizimi.
- Shkrimet në flash kanë jetëgjatësi të kufizuar; agregimi dhe grupimi i shkrimeve janë të rëndësishëm. Ndërprerja e energjisë mund të humbë mostrën e fundit në varësi të intervalit të checkpoint-it.
- Mbaj partition table dhe madhësitë e dy slot-eve OTA të pandryshuara. SPIFFS është i ndarë veçmas nga slotet e aplikacionit, por duhet verifikuar montimi dhe hapësira reale para aktivizimit.
- Boolean aktual i NTP tregon që ora është sinkronizuar; nuk provon se serveri NTP është aktualisht i arritshëm.
- MAC, IP, SSID dhe RSSI janë runtime values; vetëm pajisja live mund t'i japë saktë. Mbroji nga qasje e paautorizuar.

## Skedarët kryesorë të burimit

- `src/WebServerManager.cpp`, `src/WebServerManager.h` — endpoint-et ekzistuese dhe JSON status.
- `src/OtaManager.cpp`, `src/OtaManager.h` — rrjedha dhe statusi OTA.
- `src/WiFiManager.cpp`, `src/WiFiManager.h` — DHCP, portal AP, kredencialet dhe rilidhja.
- `src/Sensors.cpp`, `src/Sensors.h` — BME680/BSEC2, BH1750 dhe PIR.
- `src/WeatherService.cpp`, `src/WeatherService.h` — Open-Meteo dhe parashikimi ditor.
- `src/TuyaService.cpp`, `src/TuyaService.h` — statusi cloud Tuya.
- `src/ResetDiagnostics.cpp`, `src/ResetDiagnostics.h` — arsye/histori reset-i në NVS.
- `src/main.cpp`, `src/Startup.cpp`, `src/AboutScreen.cpp` — NTP, planifikimi i shërbimeve, nisja dhe gjendja e ekranit.
- `src/settings.cpp`, `src/settings.h` — Preferences e cilësimeve.
- `platformio.ini`, `include/User_Setup.h` — konfigurimi i build-it, board-it dhe ekranit.
