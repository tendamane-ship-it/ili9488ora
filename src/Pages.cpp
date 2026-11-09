#include "Pages.h"
#include <WiFi.h>

extern bool setting24Hour;
extern bool settingDate;
extern bool settingPIR;
extern bool settingWeather;
extern bool settingLightSensor;
extern bool settingTuyaDisplay;
extern bool settingScreenSaver;
extern unsigned long screenSaverDelay;
extern bool settingGreeting;
extern bool settingNightMode;
extern float nightSleepLux;
extern float nightWakeLux;

extern unsigned long localSensorsDisplayTime;
extern unsigned long weatherDisplayTime;
extern unsigned long tuyaDisplayTime;
String getDashboardPage()
{
    String html;

    html += "<!DOCTYPE html>";
    html += "<html lang='sq'>";
    html += "<head>";

    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<meta charset='UTF-8'>";
    html += "<title>ORA - Paneli</title>";

    // ============================================================
    // STYLE
    // ============================================================

    html += "<style>";

    html += "*{box-sizing:border-box;}";

    html += "body{";
    html += "margin:0;";
    html += "font-family:Arial,Helvetica,sans-serif;";
    html += "background:#080b10;";
    html += "color:#e8faff;";
    html += "}";

    html += ".container{";
    html += "max-width:1200px;";
    html += "margin:auto;";
    html += "padding:20px;";
    html += "}";

    // HEADER
    html += ".header{";
html += "margin-bottom:20px;";
html += "}";
    html += ".logo{";
    html += "font-size:30px;";
    html += "font-weight:bold;";
    html += "letter-spacing:4px;";
    html += "color:#00eaff;";
    html += "}";

    html += ".status{";
    html += "display:flex;";
    html += "align-items:center;";
    html += "gap:8px;";
    html += "font-size:13px;";
    html += "color:#9aa7b3;";
    html += "}";

    html += ".dot{";
    html += "width:10px;";
    html += "height:10px;";
    html += "border-radius:50%;";
    html += "background:#00e676;";
    html += "box-shadow:0 0 10px #00e676;";
    html += "}";

    // NAV
    html += ".nav{";
    html += "display:flex;";
    html += "gap:8px;";
    html += "margin-bottom:20px;";
    html += "flex-wrap:wrap;";
    html += "}";

    html += ".nav a{";
    html += "text-decoration:none;";
    html += "color:#9aa7b3;";
    html += "background:#111720;";
    html += "border:1px solid #202b36;";
    html += "padding:9px 15px;";
    html += "border-radius:8px;";
    html += "font-size:14px;";
    html += "}";

    html += ".nav a:hover{";
    html += "color:#00eaff;";
    html += "border-color:#00eaff;";
    html += "}";

    html += ".nav a.active{";
    html += "color:#00eaff;";
    html += "border-color:#00eaff;";
    html += "}";

    // MAIN CLOCK
    html += ".clock-card{";
    html += "background:linear-gradient(135deg,#0d141c,#101923);";
    html += "border:1px solid #1c3540;";
    html += "border-radius:16px;";
    html += "padding:24px;";
    html += "margin-bottom:18px;";
    html += "box-shadow:0 0 25px rgba(0,234,255,.06);";
    html += "}";

    html += ".clock-label{";
    html += "font-size:12px;";
    html += "letter-spacing:2px;";
    html += "color:#6f8995;";
    html += "margin-bottom:5px;";
    html += "}";

    html += ".clock{";
    html += "font-size:58px;";
    html += "font-weight:bold;";
    html += "letter-spacing:4px;";
    html += "color:#00eaff;";
    html += "text-shadow:0 0 18px rgba(0,234,255,.25);";
    html += "}";

    html += ".date{";
    html += "font-size:15px;";
    html += "color:#8fa2ad;";
    html += "margin-top:5px;";
    html += "}";

    // SECTION
    html += ".section-title{";
    html += "font-size:13px;";
    html += "letter-spacing:2px;";
    html += "color:#6f8995;";
    html += "margin:24px 0 10px 2px;";
    html += "}";

    // GRID
    html += ".grid{";
    html += "display:grid;";
    html += "grid-template-columns:repeat(3,1fr);";
    html += "gap:12px;";
    html += "}";

    // CARD
    html += ".card{";
    html += "background:#10161e;";
    html += "border:1px solid #202b36;";
    html += "border-radius:12px;";
    html += "padding:16px;";
    html += "min-height:125px;";
    html += "}";

    html += ".card-title{";
    html += "font-size:12px;";
    html += "color:#73838f;";
    html += "letter-spacing:1px;";
    html += "text-transform:uppercase;";
    html += "margin-bottom:12px;";
    html += "}";

    html += ".value{";
    html += "font-size:28px;";
    html += "font-weight:bold;";
    html += "color:#e8faff;";
    html += "}";

    html += ".unit{";
    html += "font-size:14px;";
    html += "color:#71838e;";
    html += "margin-left:4px;";
    html += "}";

    html += ".small-value{";
    html += "font-size:16px;";
    html += "font-weight:bold;";
    html += "color:#d7e7ed;";
    html += "}";

    html += ".muted{";
    html += "color:#687985;";
    html += "}";

    // STATUS BADGES
    html += ".badge{";
    html += "display:inline-block;";
    html += "padding:5px 9px;";
    html += "border-radius:20px;";
    html += "font-size:11px;";
    html += "font-weight:bold;";
    html += "letter-spacing:.5px;";
    html += "background:#18231f;";
    html += "color:#00e676;";
    html += "}";

    html += ".badge.off{";
    html += "background:#21191a;";
    html += "color:#ff6b6b;";
    html += "}";

    // TWO COLUMN INFORMATION
    html += ".info-grid{";
    html += "display:grid;";
    html += "grid-template-columns:repeat(2,1fr);";
    html += "gap:12px;";
    html += "}";

    html += ".info-card{";
    html += "background:#10161e;";
    html += "border:1px solid #202b36;";
    html += "border-radius:12px;";
    html += "padding:16px;";
    html += "}";

    html += ".info-row{";
    html += "display:flex;";
    html += "justify-content:space-between;";
    html += "gap:15px;";
    html += "padding:8px 0;";
    html += "border-bottom:1px solid #1b252e;";
    html += "}";

    html += ".info-row:last-child{";
    html += "border-bottom:none;";
    html += "}";

    html += ".info-label{";
    html += "color:#72838e;";
    html += "font-size:13px;";
    html += "}";

    html += ".info-value{";
    html += "color:#d9e9ef;";
    html += "font-size:13px;";
    html += "font-weight:bold;";
    html += "text-align:right;";
    html += "}";

    // FOOTER
    html += ".footer{";
    html += "text-align:center;";
    html += "margin-top:28px;";
    html += "font-size:11px;";
    html += "color:#4d5c65;";
    html += "}";

    // RESPONSIVE
    html += "@media(max-width:800px){";
    html += ".grid{grid-template-columns:repeat(2,1fr);}";
    html += ".info-grid{grid-template-columns:1fr;}";
    html += ".clock{font-size:46px;}";
    html += "}";

    html += "@media(max-width:520px){";
    html += ".container{padding:12px;}";
    html += ".grid{grid-template-columns:1fr;}";
    html += ".clock{font-size:38px;}";
    html += ".logo{font-size:25px;}";
    html += ".nav{gap:7px;}";
    html += ".nav a{flex:1 1 105px;text-align:center;padding:11px 8px;}";
    html += ".info-row{gap:8px;flex-wrap:wrap;}";
    html += ".info-value{text-align:left;overflow-wrap:anywhere;}";
    html += "}";

    html += "</style>";

    html += "</head>";
    html += "<body>";

    html += "<div class='container'>";

    // ============================================================
    // NAVIGATION
    // ============================================================

    html += "<div class='nav'>";
html += "<a class='active' href='/'>Ballina</a>";
html += "<a href='/weather'>Moti</a>";
html += "<a href='/tuya'>Tuya</a>";
html += "<a href='/settings'>Cilësimet</a>";
html += "<a href='/logs'>Regjistri</a>";
html += "<a href='/api'>Gjendja API</a>";
    html += "</div>";

    // ============================================================
    // CLOCK
    // ============================================================

    html += "<div class='clock-card'>";

    html += "<div class='clock-label'>ORA E SAKTË</div>";

    html += "<div class='clock' id='time'>--:--:--</div>";

    html += "<div class='date' id='date'>--</div>";

    html += "</div>";

    // ============================================================
    // ENVIRONMENT
    // ============================================================

    html += "<div class='section-title'>MJEDISI</div>";

    html += "<div class='grid'>";

    // Temperature
    html += "<div class='card'>";
    html += "<div class='card-title'>Temperatura</div>";
    html += "<div class='value' id='temperature'>--</div>";
    html += "<span class='unit'>°C</span>";
    html += "</div>";

    // Humidity
    html += "<div class='card'>";
    html += "<div class='card-title'>Lagështira</div>";
    html += "<div class='value' id='humidity'>--</div>";
    html += "<span class='unit'>%</span>";
    html += "</div>";

    // Pressure
    html += "<div class='card'>";
    html += "<div class='card-title'>Presioni</div>";
    html += "<div class='value' id='pressure'>--</div>";
    html += "<span class='unit'>hPa</span>";
    html += "</div>";

    // Lux
    html += "<div class='card'>";
    html += "<div class='card-title'>Ndriçimi</div>";
    html += "<div class='value' id='lux'>--</div>";
    html += "<span class='unit'>lux</span>";
    html += "</div>";

    // Air Quality
    html += "<div class='card'>";
    html += "<div class='card-title'>Cilësia e Ajrit</div>";
    html += "<div class='value' id='air'>--</div>";
    html += "<div class='unit' id='gas'>--</div>";
    html += "</div>";

    // Motion
    html += "<div class='card'>";
    html += "<div class='card-title'>Lëvizja</div>";
    html += "<div id='motion' class='badge'>Pa lëvizje</div>";
    html += "<div class='muted' style='margin-top:12px;font-size:12px;' id='pirStatus'>PIR</div>";
    html += "</div>";

    html += "</div>";

    // ============================================================
    // WEATHER
    // ============================================================

    html += "<div class='section-title'>SHËRBIMI I MOTIT</div>";

    html += "<div class='grid'>";

    html += "<div class='card'>";
    html += "<div class='card-title'>Temperatura e Motit</div>";
    html += "<div class='value' id='weatherTemp'>--</div>";
    html += "<span class='unit'>°C</span>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<div class='card-title'>Era</div>";
    html += "<div class='value' id='wind'>--</div>";
    html += "<span class='unit'>km/h</span>";
    html += "</div>";

    html += "<div class='card'>";
    html += "<div class='card-title'>Kodi i Motit</div>";
    html += "<div class='value' id='weatherCode'>--</div>";
    html += "</div>";

    html += "</div>";

    // ============================================================
    // NETWORK + SYSTEM
    // ============================================================

    html += "<div class='section-title'>SISTEMI</div>";

    html += "<div class='info-grid'>";

    // Network
    html += "<div class='info-card'>";

    html += "<div class='card-title'>Rrjeti</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Wi-Fi</span>";
    html += "<span class='info-value' id='wifi'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Adresa IP</span>";
    html += "<span class='info-value' id='ip'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Sinjali</span>";
    html += "<span class='info-value' id='rssi'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Lidhja</span>";
    html += "<span class='info-value' id='wifiStatus'>--</span>";
    html += "</div>";

    html += "</div>";

    // System
    html += "<div class='info-card'>";

    html += "<div class='card-title'>ESP32 SYSTEM</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>CPU</span>";
    html += "<span class='info-value' id='cpu'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Memoria e lirë</span>";
    html += "<span class='info-value' id='heap'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Flash</span>";
    html += "<span class='info-value' id='flash'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Koha e punës</span>";
    html += "<span class='info-value' id='uptime'>--</span>";
    html += "</div>";

    html += "</div>";

    html += "</div>";

    // ============================================================
    // ORA SETTINGS STATUS
    // ============================================================

    html += "<div class='section-title'>CILËSIMET E ORËS</div>";

    html += "<div class='info-card'>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Formati i orës</span>";
    html += "<span class='info-value' id='format'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Data</span>";
    html += "<span class='info-value' id='dateSetting'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Moti</span>";
    html += "<span class='info-value' id='weatherSetting'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Sensori i dritës</span>";
    html += "<span class='info-value' id='lightSetting'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Shfaqja Tuya</span>";
    html += "<span class='info-value' id='tuyaSetting'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>PIR</span>";
    html += "<span class='info-value' id='pirSetting'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Koha e motit</span>";
    html += "<span class='info-value' id='weatherTime'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Koha e sensorëve</span>";
    html += "<span class='info-value' id='localTime'>--</span>";
    html += "</div>";

    html += "<div class='info-row'>";
    html += "<span class='info-label'>Kohëzgjatja e shfaqjes Tuya</span>";
    html += "<span class='info-value' id='tuyaTime'>--</span>";
    html += "</div>";

    html += "</div>";

    // ============================================================
    // FOOTER
    // ============================================================

    html += "<div class='footer'>";
    html += "<div>ORA • ORË ILI9488 • ESP32-S3</div>";
    html += "<div style='margin-top:5px'>Mundësuar nga Murat Mane</div>";
    html += "</div>";

    html += "</div>";

    // ============================================================
    // JAVASCRIPT
    // ============================================================

    html += "<script>";

    html += "function setText(id,value){";
    html += "const e=document.getElementById(id);";
    html += "if(e)e.textContent=value;";
    html += "}";

    html += "function onOff(v){return v?'ON':'OFF';}";

    html += "function formatUptime(sec){";
    html += "sec=Math.floor(sec||0);";
    html += "let d=Math.floor(sec/86400);";
    html += "sec%=86400;";
    html += "let h=Math.floor(sec/3600);";
    html += "sec%=3600;";
    html += "let m=Math.floor(sec/60);";
    html += "let s=sec%60;";
    html += "return (d?d+' ditë ':'')+String(h).padStart(2,'0')+':'+String(m).padStart(2,'0')+':'+String(s).padStart(2,'0');";
    html += "}";

    html += "function updateDashboard(){";

    html += "fetch('/api/status')";
    html += ".then(r=>r.json())";
    html += ".then(d=>{";

    // Status
    html += "setText('statusText',d.wifiConnected?'NË LINJË':'JASHTË LINJËS');";

    // Time
    html += "setText('time',d.time||'--:--:--');";
    html += "setText('date',d.date||'--');";

    // BME680
    html += "setText('temperature',d.bmeTemperature==null?'--':Number(d.bmeTemperature).toFixed(1));";
    html += "setText('humidity',d.bmeHumidity==null?'--':Number(d.bmeHumidity).toFixed(1));";

    // Pressure from weather/API
    html += "setText('pressure',d.pressure==null?'--':Number(d.pressure).toFixed(1));";

    // Lux
    html += "setText('lux',d.lux==null?'--':Number(d.lux).toFixed(0));";

    // Gas / Air
    html += "setText('gas',d.bmeGas==null?'Gaz --':('Gaz '+Number(d.bmeGas).toFixed(0)));";

    html += "let air='--';";
    html += "if(d.bme680OK){";
    html += "if(d.bmeIaq==null||d.bmeIaqAccuracy===0)air='Po kalibrohet';";
    html += "else air=Number(d.bmeIaq).toFixed(0)+' '+(d.airQualityCategory||'')+' - Saktesia: '+(d.bmeIaqAccuracyLabel||'E pasigurt');";
    html += "}";
    html += "setText('air',air);";

    // PIR
    html += "const motion=document.getElementById('motion');";
    html += "if(d.pirMotion){";
    html += "motion.textContent='LËVIZJE';";
    html += "motion.className='badge';";
    html += "}else{";
    html += "motion.textContent='PA LËVIZJE';";
    html += "motion.className='badge off';";
    html += "}";

    html += "setText('pirStatus',d.pirState===1?'U zbulua lëvizje':'Në vëzhgim');";

    // Weather
    html += "setText('weatherTemp',d.temperature==null?'--':Number(d.temperature).toFixed(1));";
    html += "setText('wind',d.windSpeed==null?'--':Number(d.windSpeed).toFixed(1));";
    html += "setText('weatherCode',d.weatherCode==null?'--':d.weatherCode);";

    // Network
    html += "setText('wifi',d.wifi||'--');";
    html += "setText('ip',d.ip||'--');";
    html += "setText('rssi',d.rssi==null?'--':d.rssi+' dBm');";
    html += "setText('wifiStatus',d.wifiConnected?'I LIDHUR':'I SHKËPUTUR');";

    // System
    html += "setText('cpu',d.cpu==null?'--':d.cpu+' MHz');";
    html += "setText('heap',d.heap==null?'--':Math.round(d.heap/1024)+' KB');";
    html += "setText('flash',d.flash==null?'--':Math.round(d.flash/1048576)+' MB');";
    html += "setText('uptime',formatUptime(d.uptime));";

    // Settings
    html += "setText('format',d.setting24Hour?'24 ORË':'12 ORË');";
    html += "setText('dateSetting',onOff(d.settingDate));";
    html += "setText('weatherSetting',onOff(d.settingWeather));";
    html += "setText('lightSetting',onOff(d.settingLightSensor));";
    html += "setText('tuyaSetting',onOff(d.settingTuyaDisplay));";
    html += "setText('pirSetting',onOff(d.settingPIR));";

    html += "setText('weatherTime',d.weatherDisplayTime+' sek');";
    html += "setText('localTime',d.localSensorsDisplayTime+' sek');";
    html += "setText('tuyaTime',d.tuyaDisplayTime+' sek');";

    html += "})";

    html += ".catch(e=>{";
    html += "setText('statusText','JASHTË LINJËS');";
    html += "});";

    html += "}";

    html += "updateDashboard();";
    html += "setInterval(updateDashboard,3000);";

    html += "</script>";

    html += "</body>";
    html += "</html>";

    return html;
}
String getSettingsPage()
{
    String html;
   

    html += "<!DOCTYPE html>";
    html += "<html lang='sq'>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>ORA - Cilësimet</title>";



    html += "<style>";

    html += "*{box-sizing:border-box;}";
    html += "html{font-size:16px;-webkit-text-size-adjust:100%;}";

    html += "body{";
    html += "margin:0;";
    html += "font-family:Arial,Helvetica,sans-serif;";
    html += "background:#0b0f14;";
    html += "color:#f5f7fa;";
    html += "}";

    html += ".container{";
    html += "max-width:800px;";
    html += "margin:auto;";
    html += "padding:20px;";
    html += "}";

    html += ".header{";
html += "margin-bottom:20px;";
html += "}";

html += ".logo{";
html += "margin-bottom:14px;";
html += "}";

    html += ".logo{";
    html += "font-size:30px;";
    html += "font-weight:700;";
    html += "letter-spacing:3px;";
    html += "color:#00d9ff;";
    html += "}";

    html += ".back{";
    html += "text-decoration:none;";
    html += "color:#fff;";
    html += "background:#151b23;";
    html += "padding:10px 16px;";
    html += "border-radius:9px;";
    html += "}";

    html += ".card{";
    html += "background:#121820;";
    html += "border:1px solid #202b38;";
    html += "border-radius:16px;";
    html += "padding:22px;";
    html += "margin-bottom:16px;";
    html += "}";

    html += "h2{";
    html += "margin-top:0;";
    html += "font-size:18px;";
    html += "}";

    html += ".setting{";
    html += "display:flex;";
    html += "justify-content:space-between;";
    html += "align-items:center;";
    html += "gap:20px;";
    html += "padding:16px 0;";
    html += "border-bottom:1px solid #202832;";
    html += "}";

    html += ".setting:last-child{";
    html += "border-bottom:none;";
    html += "}";

    html += ".label{";
    html += "font-size:17px;";
    html += "}";

    html += ".description{";
    html += "font-size:14px;";
    html += "color:#7e8a99;";
    html += "margin-top:4px;";
    html += "}";

    html += ".control{";
    html += "display:flex;";
    html += "align-items:center;";
    html += "gap:8px;";
    html += "flex-shrink:0;";
    html += "}";

    html += "select,input[type='number']{";
    html += "background:#1d2833;";
    html += "color:#fff;";
    html += "border:1px solid #344352;";
    html += "border-radius:8px;";
    html += "padding:9px 12px;";
    html += "font-size:16px;";
    html += "}";

    html += "select{";
    html += "min-width:100px;";
    html += "}";

    html += "input[type='number']{";
    html += "width:100px;";
    html += "}";

    html += ".badge{";
    html += "background:#1d2833;";
    html += "color:#00d9ff;";
    html += "padding:6px 10px;";
    html += "border-radius:7px;";
    html += "font-size:14px;";
    html += "}";

    html += ".save{";
    html += "width:100%;";
    html += "border:none;";
    html += "border-radius:10px;";
    html += "padding:14px;";
    html += "background:#00a9c7;";
    html += "color:#fff;";
    html += "font-size:16px;";
    html += "font-weight:600;";
    html += "cursor:pointer;";
    html += "}";

    html += ".save:hover{";
    html += "background:#00bddc;";
    html += "}";

    html += ".restart{";
    html += "display:block;";
    html += "width:100%;";
    html += "margin-top:16px;";
    html += "padding:14px;";
    html += "border:1px solid #ff5b62;";
    html += "border-radius:10px;";
    html += "background:linear-gradient(180deg,#d93643,#b8202d);";
    html += "color:#fff;";
    html += "font-size:16px;";
    html += "font-weight:700;";
    html += "cursor:pointer;";
    html += "box-shadow:0 4px 12px rgba(255,59,72,.22);";
    html += "}";

    html += ".restart:hover{";
    html += "background:linear-gradient(180deg,#f04754,#ce2937);";
    html += "border-color:#ff858b;";
    html += "}";

    html += ".restart:focus-visible{";
    html += "outline:3px solid rgba(255,91,98,.45);";
    html += "outline-offset:3px;";
    html += "}";

    html += ".note{";
    html += "font-size:12px;";
    html += "color:#7e8a99;";
    html += "text-align:center;";
    html += "margin-top:12px;";
    html += "}";

    html += "@media(max-width:600px){";

    html += ".container{";
    html += "padding:14px;";
    html += "}";

    html += ".setting{";
    html += "align-items:flex-start;";
    html += "}";

    html += ".control{";
    html += "margin-top:2px;";
    html += "}";

    html += ".nav{";
    html += "gap:7px;";
    html += "}";
    html += ".nav a{";
    html += "flex:1 1 105px;";
    html += "text-align:center;";
    html += "padding:11px 8px;";
    html += "}";

    html += "}";
    html += ".nav{";
    html += "display:flex;";
    html += "gap:8px;";
    html += "width:100%;";
    html += "flex-wrap:wrap;";
    html += "margin-top:12px;";
    html += "margin-bottom:20px;";
    html += "justify-content:flex-start;";
    html += "}";

    html += ".nav a{";
    html += "text-decoration:none;";
    html += "color:#9aa7b3;";
    html += "background:#151b23;";
    html += "border:1px solid #202b38;";
    html += "padding:9px 15px;";
    html += "border-radius:8px;";
    html += "font-size:15px;";
    html += "}";

    html += ".nav a:hover{";
    html += "color:#00eaff;";
    html += "border-color:#00eaff;";
    html += "}";

    html += ".nav a.active{";
    html += "color:#00eaff;";
    html += "border-color:#00eaff;";
    html += "}";

    html += "@media(max-width:600px){";
    html += ".container{padding:16px;}";
    html += ".logo{font-size:32px;}";
    html += "h2{font-size:20px;}";
    html += ".setting{flex-direction:column;align-items:stretch;gap:10px;padding:18px 0;}";
    html += ".label{font-size:18px;line-height:1.35;}";
    html += ".description{font-size:15px;line-height:1.4;}";
    html += ".control{width:100%;margin-top:0;}";
    html += "select,input[type='number']{min-height:48px;font-size:17px;}";
    html += "select{width:100%;}";
    html += "input[type='number']{width:120px;}";
    html += ".nav{gap:8px;}";
    html += ".nav a{flex:1 1 30%;text-align:center;padding:12px 8px;font-size:15px;}";
    html += ".save,.restart{min-height:52px;font-size:18px;}";
    html += ".note{font-size:14px;line-height:1.4;}";
    html += "}";

    html += "</style>";

    html += "</head>";

    html += "<body>";

html += "<div class='container'>";

html += "<div class='header'>";

html += "<div class='logo'>ORA</div>";

html += "<div class='nav'>";
html += "<a href='/'>Ballina</a>";
html += "<a href='/weather'>Moti</a>";
html += "<a href='/tuya'>Tuya</a>";
html += "<a class='active' href='/settings'>Cilësimet</a>";
html += "<a href='/logs'>Regjistri</a>";
html += "<a href='/api'>Gjendja API</a>";
html += "</div>";

html += "</div>";



    
    // --------------------------------------------------------
    // TIME / DISPLAY
    // --------------------------------------------------------

    html += "<form method='POST' action='/settings/save'>";

    html += "<div class='card'>";

    html += "<h2>Ora dhe ekrani</h2>";

    // Clock format
    html += "<div class='setting'>";

    html += "<div>";
    html += "<div class='label'>Formati i orës</div>";
    html += "<div class='description'>Zgjidh formatin 12- ose 24-orësh</div>";
    html += "</div>";

    html += "<div class='control'>";

    html += "<select name='24hour'>";

    if (setting24Hour)
        html += "<option value='1' selected>24H</option>";
    else
        html += "<option value='1'>24H</option>";

    if (!setting24Hour)
        html += "<option value='0' selected>12H</option>";
    else
        html += "<option value='0'>12H</option>";

    html += "</select>";

    html += "</div>";

    html += "</div>";

    // Date
    html += "<div class='setting'>";

    html += "<div>";
    html += "<div class='label'>Data</div>";
    html += "<div class='description'>Shfaq datën në ekran</div>";
    html += "</div>";

    html += "<div class='control'>";

    html += "<select name='date'>";

    if (settingDate)
        html += "<option value='1' selected>ON</option>";
    else
        html += "<option value='1'>ON</option>";

    if (!settingDate)
        html += "<option value='0' selected>OFF</option>";
    else
        html += "<option value='0'>OFF</option>";

    html += "</select>";

    html += "</div>";

    html += "</div>";

    html += "</div>";

    // --------------------------------------------------------
    // LOWER PANEL
    // --------------------------------------------------------

    html += "<div class='card'>";

    html += "<h2>Paneli i poshtëm</h2>";

    // Weather panel
    html += "<div class='setting'>";

    html += "<div>";
    html += "<div class='label'>Moti nga interneti</div>";
    html += "<div class='description'>Shfaq ose fshih motin nga interneti</div>";
    html += "</div>";

    html += "<div class='control'>";

    html += "<select name='weather'>";

    if (settingWeather)
        html += "<option value='1' selected>ON</option>";
    else
        html += "<option value='1'>ON</option>";

    if (!settingWeather)
        html += "<option value='0' selected>OFF</option>";
    else
        html += "<option value='0'>OFF</option>";

    html += "</select>";

    html += "</div>";

    html += "</div>";

    html += "<div class='setting'>";

    html += "<div>";
    html += "<div class='label'>Kohëzgjatja e motit</div>";
    html += "<div class='description'>Sekondat për shfaqjen e motit nga interneti</div>";
    html += "</div>";

    html += "<div class='control'>";

    html += "<input type='number' name='weatherTime' min='1' max='300' value='";
    html += String(weatherDisplayTime / 1000);
    html += "'>";

    html += "<span class='badge'>sek</span>";

    html += "</div>";

    html += "</div>";

    html += "<div class='setting'>";

    html += "<div>";
    html += "<div class='label'>Sensorët lokalë</div>";
    html += "<div class='description'>Shfaq ose fshih leximet BME680 / BH1750</div>";
    html += "</div>";

    html += "<div class='control'>";

    html += "<select name='light'>";

    if (settingLightSensor)
        html += "<option value='1' selected>ON</option>";
    else
        html += "<option value='1'>ON</option>";

    if (!settingLightSensor)
        html += "<option value='0' selected>OFF</option>";
    else
        html += "<option value='0'>OFF</option>";

    html += "</select>";

    html += "</div>";

    html += "</div>";

    html += "<div class='setting'>";

    html += "<div>";
    html += "<div class='label'>Kohëzgjatja e sensorëve lokalë</div>";
    html += "<div class='description'>Sekondat për shfaqjen e leximeve BME680 / BH1750</div>";
    html += "</div>";

    html += "<div class='control'>";

    html += "<input type='number' name='localTime' min='1' max='300' value='";
    html += String(localSensorsDisplayTime / 1000);
    html += "'>";

    html += "<span class='badge'>sek</span>";

    html += "</div>";

    html += "</div>";

    html += "<div class='setting'>";

    html += "<div>";
    html += "<div class='label'>Shfaqja Tuya</div>";
    html += "<div class='description'>Shfaq ose fshih temperaturën, lagështirën dhe baterinë Tuya</div>";
    html += "</div>";

    html += "<div class='control'>";

    html += "<select name='tuyaDisplay'>";

    if (settingTuyaDisplay)
        html += "<option value='1' selected>ON</option>";
    else
        html += "<option value='1'>ON</option>";

    if (!settingTuyaDisplay)
        html += "<option value='0' selected>OFF</option>";
    else
        html += "<option value='0'>OFF</option>";

    html += "</select>";

    html += "</div>";

    html += "</div>";

    html += "<div class='setting'>";

    html += "<div>";
    html += "<div class='label'>Kohëzgjatja e shfaqjes Tuya</div>";
    html += "<div class='description'>Sekondat për shfaqjen e leximeve Tuya</div>";
    html += "</div>";

    html += "<div class='control'>";

    html += "<input type='number' name='tuyaTime' min='1' max='300' value='";
    html += String(tuyaDisplayTime / 1000);
    html += "'>";

    html += "<span class='badge'>sek</span>";

    html += "</div>";

    html += "</div>";

    html += "</div>";

    // --------------------------------------------------------
    // SENSORS
    // --------------------------------------------------------

    html += "<div class='card'>";

    html += "<h2>Sensorët</h2>";

    // Weather
    html += "<div class='setting'>";

    html += "<div>";
    html += "<div class='label'>Moti</div>";
    html += "<div class='description'>Informacioni i motit nga interneti</div>";
    html += "</div>";

    html += "<div class='badge'>";
    html += settingWeather ? "ON" : "OFF";
    html += "</div>";

    html += "</div>";

    // Light sensor
    html += "<div class='setting'>";

    html += "<div>";
    html += "<div class='label'>Sensori i dritës</div>";
    html += "<div class='description'>Matësi i ndriçimit të ambientit BH1750</div>";
    html += "</div>";

    html += "<div class='badge'>";
    html += settingLightSensor ? "ON" : "OFF";
    html += "</div>";

    html += "</div>";

    // PIR
    html += "<div class='setting'>";

    html += "<div>";
    html += "<div class='label'>Sensori PIR</div>";
    html += "<div class='description'>Zbulimi i lëvizjes dhe zgjimi i ekranit</div>";
    html += "</div>";

    html += "<div class='control'>";

    html += "<select name='pir'>";

    if (settingPIR)
        html += "<option value='1' selected>ON</option>";
    else
        html += "<option value='1'>ON</option>";

    if (!settingPIR)
        html += "<option value='0' selected>OFF</option>";
    else
        html += "<option value='0'>OFF</option>";

    html += "</select>";

    html += "</div>";

    html += "</div>";

    html += "</div>";

  // --------------------------------------------------------
// SYSTEM
// --------------------------------------------------------
html += "<div class='card'>";

html += "<h2>Sistemi</h2>";

// Screensaver
html += "<div class='setting'>";

html += "<div>";
html += "<div class='label'>Ruajtësi i ekranit</div>";
html += "<div class='description'>Aktivizo ose çaktivizo ruajtësin e ekranit</div>";
html += "</div>";

html += "<div class='control'>";
html += "<select name='screensaver'>";

if (settingScreenSaver)
    html += "<option value='1' selected>ON</option>";
else
    html += "<option value='1'>ON</option>";

if (!settingScreenSaver)
    html += "<option value='0' selected>OFF</option>";
else
    html += "<option value='0'>OFF</option>";

html += "</select>";
html += "</div>";

html += "</div>";

// Screensaver timeout
html += "<div class='setting'>";

html += "<div>";
html += "<div class='label'>Koha deri te ruajtësi i ekranit</div>";
html += "<div class='description'>Minutat para se të aktivizohet ruajtësi i ekranit</div>";
html += "</div>";

html += "<div class='control'>";
html += "<input type='number' name='screenSaverMin' min='1' max='60' value='";

html += String(screenSaverDelay / (60UL * 1000UL));

html += "'>";
html += "</div>";

html += "</div>";


html += "<div class='setting'>";

html += "<div>";
html += "<div class='label'>Përshëndetja</div>";
html += "<div class='description'>Shfaq përshëndetjen kur sensori PIR zgjon orën</div>";
html += "</div>";

html += "<div class='control'>";
html += "<select name='greeting'>";

if (settingGreeting)
    html += "<option value='1' selected>ON</option>";
else
    html += "<option value='1'>ON</option>";

if (!settingGreeting)
    html += "<option value='0' selected>OFF</option>";
else
    html += "<option value='0'>OFF</option>";

html += "</select>";
html += "</div>";

html += "</div>";

html += "<div class='setting'>";

html += "<div>";
html += "<div class='label'>Serveri i uebit</div>";
html += "<div class='description'>Ndërfaqja e uebit e ESP32</div>";
html += "</div>";

html += "<div class='badge'>NË PUNË</div>";

html += "</div>";

html += "</div>";


// --------------------------------------------------------
// NIGHT MODE
// --------------------------------------------------------

html += "<div class='card'>";

html += "<h2>Modaliteti i natës</h2>";

// Night Mode ON/OFF
html += "<div class='setting'>";

html += "<div>";
html += "<div class='label'>Modaliteti i natës</div>";
html += "<div class='description'>Fike ekranin kur dhoma errësohet</div>";
html += "</div>";

html += "<div class='control'>";
html += "<select name='nightMode'>";

if (settingNightMode)
    html += "<option value='1' selected>ON</option>";
else
    html += "<option value='1'>ON</option>";

if (!settingNightMode)
    html += "<option value='0' selected>OFF</option>";
else
    html += "<option value='0'>OFF</option>";

html += "</select>";
html += "</div>";

html += "</div>";

// Sleep below lux
html += "<div class='setting'>";

html += "<div>";
html += "<div class='label'>Fike nën</div>";
html += "<div class='description'>Ekrani fiket kur ndriçimi bie nën këtë nivel</div>";
html += "</div>";

html += "<div class='control'>";
html += "<input type='number' ";
html += "name='nightSleepLux' ";
html += "min='0' ";
html += "max='1000' ";
html += "step='0.1' ";
html += "value='";
html += String(nightSleepLux, 1);
html += "'>";
html += "<span class='badge'>lux</span>";
html += "</div>";

html += "</div>";

// Wake above lux
html += "<div class='setting'>";

html += "<div>";
html += "<div class='label'>Ndize mbi</div>";
html += "<div class='description'>Ekrani ndizet kur ndriçimi kalon këtë nivel</div>";
html += "</div>";

html += "<div class='control'>";
html += "<input type='number' ";
html += "name='nightWakeLux' ";
html += "min='0' ";
html += "max='1000' ";
html += "step='0.1' ";
html += "value='";
html += String(nightWakeLux, 1);
html += "'>";
html += "<span class='badge'>lux</span>";
html += "</div>";

html += "</div>";

html += "</div>";
    // --------------------------------------------------------
    // SAVE
    // --------------------------------------------------------

    html += "<button class='save' type='submit'>";
    html += "Ruaj cilësimet";
    html += "</button>";

    html += "<button type='button' class='restart' onclick='restartESP()'>";
    html += "Rinis ESP32";
    html += "</button>";

    html += "<div class='note'>";
    html += "Cilësimet ruhen në memorien e ESP32.";
    html += "</div>";
html += "</form>";

    html += "<div style='margin:24px 0 8px;text-align:center;color:#71838e;font-size:11px'>Mundësuar nga Murat Mane</div>";

    html += "</div>";

html += "<script>";

html += "function restartESP(){";
html += "if(!confirm('Dëshironi ta rinisni ESP32-n?')) return;";
html += "fetch('/restart',{method:'POST'});";
html += "alert('ESP32 po riniset...');";
html += "}";

html += "</script>";

html += "</body>";
html += "</html>";

return html;
}