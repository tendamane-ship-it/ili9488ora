#include "Pages.h"
#include <WiFi.h>

String getDashboardPage()
{
    String html;

    html += "<!DOCTYPE html>";
    html += "<html lang='en'>";
    html += "<head>";

    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";

    html += "<title>ORA Dashboard</title>";

    html += "<style>";

    // --------------------------------------------------------
    // GLOBAL
    // --------------------------------------------------------

    html += "*{box-sizing:border-box;}";

    html += "body{";
    html += "margin:0;";
    html += "font-family:Arial,Helvetica,sans-serif;";
    html += "background:#0b0f14;";
    html += "color:#f5f7fa;";
    html += "}";

    html += ".container{";
    html += "width:100%;";
    html += "max-width:1100px;";
    html += "margin:auto;";
    html += "padding:20px;";
    html += "}";

    // --------------------------------------------------------
    // HEADER
    // --------------------------------------------------------

    html += ".header{";
    html += "display:flex;";
    html += "justify-content:space-between;";
    html += "align-items:center;";
    html += "margin-bottom:22px;";
    html += "}";

    html += ".logo{";
    html += "font-size:30px;";
    html += "font-weight:700;";
    html += "letter-spacing:3px;";
    html += "color:#00d9ff;";
    html += "}";

    html += ".status{";
    html += "display:flex;";
    html += "align-items:center;";
    html += "gap:8px;";
    html += "font-size:14px;";
    html += "color:#9aa4b2;";
    html += "}";

    html += ".dot{";
    html += "width:10px;";
    html += "height:10px;";
    html += "background:#28d17c;";
    html += "border-radius:50%;";
    html += "box-shadow:0 0 10px #28d17c;";
    html += "}";

    // --------------------------------------------------------
    // NAVIGATION
    // --------------------------------------------------------

    html += ".nav{";
    html += "display:flex;";
    html += "gap:8px;";
    html += "margin-bottom:24px;";
    html += "}";

    html += ".nav a{";
    html += "text-decoration:none;";
    html += "color:#aeb8c5;";
    html += "padding:10px 18px;";
    html += "border-radius:9px;";
    html += "background:#151b23;";
    html += "font-size:14px;";
    html += "}";

    html += ".nav a.active{";
    html += "background:#00a9c7;";
    html += "color:#fff;";
    html += "}";

    // --------------------------------------------------------
    // CLOCK CARD
    // --------------------------------------------------------

    html += ".clock-card{";
    html += "background:linear-gradient(135deg,#111923,#0e141c);";
    html += "border:1px solid #202b38;";
    html += "border-radius:18px;";
    html += "padding:28px;";
    html += "margin-bottom:18px;";
    html += "text-align:center;";
    html += "}";

    html += ".clock-label{";
    html += "font-size:13px;";
    html += "color:#7e8a99;";
    html += "text-transform:uppercase;";
    html += "letter-spacing:2px;";
    html += "}";

    html += ".clock{";
    html += "font-size:58px;";
    html += "font-weight:700;";
    html += "color:#00d9ff;";
    html += "margin:8px 0;";
    html += "letter-spacing:3px;";
    html += "}";

    html += ".date{";
    html += "font-size:16px;";
    html += "color:#aeb8c5;";
    html += "}";

    // --------------------------------------------------------
    // SENSOR GRID
    // --------------------------------------------------------

    html += ".grid{";
    html += "display:grid;";
    html += "grid-template-columns:repeat(4,1fr);";
    html += "gap:14px;";
    html += "margin-bottom:18px;";
    html += "}";

    html += ".card{";
    html += "background:#121820;";
    html += "border:1px solid #202b38;";
    html += "border-radius:16px;";
    html += "padding:20px;";
    html += "}";

    html += ".card-title{";
    html += "font-size:13px;";
    html += "color:#7e8a99;";
    html += "margin-bottom:12px;";
    html += "}";

    html += ".value{";
    html += "font-size:28px;";
    html += "font-weight:600;";
    html += "color:#f5f7fa;";
    html += "}";

    html += ".unit{";
    html += "font-size:14px;";
    html += "color:#7e8a99;";
    html += "margin-left:3px;";
    html += "}";

    // --------------------------------------------------------
    // SYSTEM GRID
    // --------------------------------------------------------

    html += ".system-grid{";
    html += "display:grid;";
    html += "grid-template-columns:repeat(2,1fr);";
    html += "gap:14px;";
    html += "margin-bottom:18px;";
    html += "}";

    html += ".info{";
    html += "display:flex;";
    html += "justify-content:space-between;";
    html += "padding:10px 0;";
    html += "border-bottom:1px solid #202832;";
    html += "}";

    html += ".info:last-child{";
    html += "border-bottom:none;";
    html += "}";

    html += ".info-label{";
    html += "color:#7e8a99;";
    html += "}";

    html += ".info-value{";
    html += "color:#f5f7fa;";
    html += "}";

    // --------------------------------------------------------
    // FOOTER
    // --------------------------------------------------------

    html += ".footer{";
    html += "text-align:center;";
    html += "color:#596575;";
    html += "font-size:12px;";
    html += "padding:15px;";
    html += "}";

    // --------------------------------------------------------
    // MOBILE
    // --------------------------------------------------------

    html += "@media(max-width:750px){";

    html += ".container{padding:14px;}";

    html += ".header{";
    html += "margin-bottom:15px;";
    html += "}";

    html += ".logo{";
    html += "font-size:25px;";
    html += "}";

    html += ".grid{";
    html += "grid-template-columns:repeat(2,1fr);";
    html += "}";

    html += ".system-grid{";
    html += "grid-template-columns:1fr;";
    html += "}";

    html += ".clock{";
    html += "font-size:44px;";
    html += "}";

    html += "}";

    html += "</style>";

    html += "</head>";

    // --------------------------------------------------------
    // BODY
    // --------------------------------------------------------

    html += "<body>";

    html += "<div class='container'>";

    // Header
    html += "<div class='header'>";

    html += "<div class='logo'>ORA</div>";

    html += "<div class='status'>";
    html += "<span class='dot'></span>";
    html += "<span>ONLINE</span>";
    html += "</div>";

    html += "</div>";

    // Navigation
    html += "<div class='nav'>";

    html += "<a class='active' href='/'>Dashboard</a>";
    html += "<a href='/settings'>Settings</a>";
    html += "<a href='/api'>API Status</a>";

    html += "</div>";

    // Clock
    html += "<div class='clock-card'>";

    html += "<div class='clock-label'>Current Time</div>";

    html += "<div class='clock' id='clock'>--:--:--</div>";

    html += "<div class='date' id='date'>Waiting for time...</div>";

    html += "</div>";

    // Sensor cards
    html += "<div class='grid'>";

    // Temperature
    html += "<div class='card'>";
    html += "<div class='card-title'>TEMPERATURE</div>";
    html += "<div class='value'>";
    html += "<span id='temperature'>--</span>";
    html += "<span class='unit'>°C</span>";
    html += "</div>";
    html += "</div>";

    // Humidity
    html += "<div class='card'>";
    html += "<div class='card-title'>HUMIDITY</div>";
    html += "<div class='value'>";
    html += "<span id='humidity'>--</span>";
    html += "<span class='unit'>%</span>";
    html += "</div>";
    html += "</div>";

    // Pressure
    html += "<div class='card'>";
    html += "<div class='card-title'>PRESSURE</div>";
    html += "<div class='value'>";
    html += "<span id='pressure'>--</span>";
    html += "<span class='unit'>hPa</span>";
    html += "</div>";
    html += "</div>";

    // Lux
    html += "<div class='card'>";
    html += "<div class='card-title'>LIGHT</div>";
    html += "<div class='value'>";
    html += "<span id='lux'>--</span>";
    html += "<span class='unit'>lux</span>";
    html += "</div>";
    html += "</div>";

    html += "</div>";

    // System
    html += "<div class='system-grid'>";

    // WiFi
    html += "<div class='card'>";

    html += "<div class='card-title'>NETWORK</div>";

    html += "<div class='info'>";
    html += "<span class='info-label'>SSID</span>";
    html += "<span class='info-value' id='ssid'>";
    html += WiFi.SSID();
    html += "</span>";
    html += "</div>";

    html += "<div class='info'>";
    html += "<span class='info-label'>IP Address</span>";
    html += "<span class='info-value' id='ip'>";
    html += WiFi.localIP().toString();
    html += "</span>";
    html += "</div>";

    html += "<div class='info'>";
    html += "<span class='info-label'>Signal</span>";
    html += "<span class='info-value' id='rssi'>";
    html += String(WiFi.RSSI());
    html += " dBm</span>";
    html += "</div>";

    html += "</div>";

    // System
    html += "<div class='card'>";

    html += "<div class='card-title'>SYSTEM</div>";

    html += "<div class='info'>";
    html += "<span class='info-label'>CPU</span>";
    html += "<span class='info-value' id='cpu'>-- MHz</span>";
    html += "</div>";

    html += "<div class='info'>";
    html += "<span class='info-label'>Free Heap</span>";
    html += "<span class='info-value' id='heap'>--</span>";
    html += "</div>";

    html += "<div class='info'>";
    html += "<span class='info-label'>Uptime</span>";
    html += "<span class='info-value' id='uptime'>--</span>";
    html += "</div>";

    html += "</div>";

    html += "</div>";

    // Footer
    html += "<div class='footer'>";
    html += "ORA Smart Clock &nbsp;•&nbsp; ESP32-S3";
    html += "</div>";

    html += "</div>";

    // --------------------------------------------------------
    // JAVASCRIPT
    // --------------------------------------------------------

    html += "<script>";

    html += "function updateData(){";

    html += "fetch('/api/status')";
    html += ".then(response=>response.json())";
    html += ".then(data=>{";

    html += "if(data.temperature!==undefined)";
    html += "document.getElementById('temperature').innerText=data.temperature;";

    html += "if(data.humidity!==undefined)";
    html += "document.getElementById('humidity').innerText=data.humidity;";

    html += "if(data.pressure!==undefined)";
    html += "document.getElementById('pressure').innerText=data.pressure;";

    html += "if(data.lux!==undefined)";
    html += "document.getElementById('lux').innerText=data.lux;";

    html += "if(data.time!==undefined)";
    html += "document.getElementById('clock').innerText=data.time;";

    html += "if(data.ip!==undefined)";
    html += "document.getElementById('ip').innerText=data.ip;";

    html += "if(data.rssi!==undefined)";
    html += "document.getElementById('rssi').innerText=data.rssi+' dBm';";

    html += "if(data.cpu!==undefined)";
    html += "document.getElementById('cpu').innerText=data.cpu+' MHz';";

    html += "if(data.heap!==undefined)";
    html += "document.getElementById('heap').innerText=data.heap;";

    html += "if(data.uptime!==undefined)";
    html += "document.getElementById('uptime').innerText=data.uptime;";

    html += "})";

    html += ".catch(error=>console.log('API error:',error));";

    html += "}";

    html += "updateData();";

    html += "setInterval(updateData,3000);";

    html += "</script>";

    html += "</body>";
    html += "</html>";

    return html;
}


String getSettingsPage()
{
    String html;

    html += "<!DOCTYPE html>";
    html += "<html lang='en'>";

    html += "<head>";

    html += "<meta charset='UTF-8'>";

    html += "<meta name='viewport' ";
    html += "content='width=device-width, initial-scale=1.0'>";

    html += "<title>ORA Settings</title>";

    html += "<style>";

    html += "*{box-sizing:border-box;}";

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
    html += "display:flex;";
    html += "justify-content:space-between;";
    html += "align-items:center;";
    html += "margin-bottom:20px;";
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
    html += "padding:16px 0;";
    html += "border-bottom:1px solid #202832;";
    html += "}";

    html += ".setting:last-child{";
    html += "border-bottom:none;";
    html += "}";

    html += ".label{";
    html += "font-size:15px;";
    html += "}";

    html += ".description{";
    html += "font-size:12px;";
    html += "color:#7e8a99;";
    html += "margin-top:4px;";
    html += "}";

    html += ".badge{";
    html += "background:#1d2833;";
    html += "color:#00d9ff;";
    html += "padding:6px 10px;";
    html += "border-radius:7px;";
    html += "font-size:12px;";
    html += "}";

    html += "</style>";

    html += "</head>";

    html += "<body>";

    html += "<div class='container'>";

    html += "<div class='header'>";

    html += "<div class='logo'>ORA</div>";

    html += "<a class='back' href='/'>Dashboard</a>";

    html += "</div>";

    // Display settings
    html += "<div class='card'>";

    html += "<h2>Display</h2>";

    html += "<div class='setting'>";
    html += "<div>";
    html += "<div class='label'>Brightness</div>";
    html += "<div class='description'>Display brightness</div>";
    html += "</div>";
    html += "<div class='badge'>Coming soon</div>";
    html += "</div>";

    html += "<div class='setting'>";
    html += "<div>";
    html += "<div class='label'>Clock format</div>";
    html += "<div class='description'>12 / 24 hour format</div>";
    html += "</div>";
    html += "<div class='badge'>24H</div>";
    html += "</div>";

    html += "<div class='setting'>";
    html += "<div>";
    html += "<div class='label'>Date</div>";
    html += "<div class='description'>Show date on display</div>";
    html += "</div>";
    html += "<div class='badge'>ON</div>";
    html += "</div>";

    html += "</div>";

    // Sensors
    html += "<div class='card'>";

    html += "<h2>Sensors</h2>";

    html += "<div class='setting'>";
    html += "<div>";
    html += "<div class='label'>Weather</div>";
    html += "<div class='description'>Temperature and weather information</div>";
    html += "</div>";
    html += "<div class='badge'>ON</div>";
    html += "</div>";

    html += "<div class='setting'>";
    html += "<div>";
    html += "<div class='label'>Light sensor</div>";
    html += "<div class='description'>BH1750 ambient light</div>";
    html += "</div>";
    html += "<div class='badge'>ON</div>";
    html += "</div>";

    html += "</div>";

    // System
    html += "<div class='card'>";

    html += "<h2>System</h2>";

    html += "<div class='setting'>";
    html += "<div>";
    html += "<div class='label'>Web Server</div>";
    html += "<div class='description'>ESP32 web interface</div>";
    html += "</div>";
    html += "<div class='badge'>RUNNING</div>";
    html += "</div>";

    html += "</div>";

    html += "</div>";

    html += "</body>";
    html += "</html>";

    return html;
}