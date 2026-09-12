#include "Pages.h"
#include <WiFi.h>

String getDashboardPage()
{
    String html;

    html += "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";

    html += "<meta name='viewport' ";
    html += "content='width=device-width, initial-scale=1'>";

    html += "<title>testili9848</title>";

    html += "<style>";

    html += "body{";
    html += "font-family:Arial,sans-serif;";
    html += "background:#111;";
    html += "color:#fff;";
    html += "margin:0;";
    html += "padding:20px;";
    html += "}";

    html += ".container{";
    html += "max-width:600px;";
    html += "margin:auto;";
    html += "}";

    html += ".card{";
    html += "background:#222;";
    html += "border-radius:12px;";
    html += "padding:20px;";
    html += "margin-bottom:15px;";
    html += "}";

    html += "h1{";
    html += "margin-top:0;";
    html += "}";

    html += ".value{";
    html += "font-size:20px;";
    html += "margin:8px 0;";
    html += "}";

    html += "a{";
    html += "display:inline-block;";
    html += "background:#333;";
    html += "color:#fff;";
    html += "padding:12px 18px;";
    html += "border-radius:8px;";
    html += "text-decoration:none;";
    html += "margin-top:10px;";
    html += "}";

    html += "</style>";

    html += "</head>";

    html += "<body>";

    html += "<div class='container'>";

    html += "<div class='card'>";

    html += "<h1>testili9848</h1>";

    html += "<p>Dashboard</p>";

    html += "</div>";

    html += "<div class='card'>";

    html += "<h2>WiFi</h2>";

    html += "<div class='value'>";
    html += "SSID: ";
    html += WiFi.SSID();
    html += "</div>";

    html += "<div class='value'>";
    html += "IP: ";
    html += WiFi.localIP().toString();
    html += "</div>";

    html += "<div class='value'>";
    html += "RSSI: ";
    html += String(WiFi.RSSI());
    html += " dBm";
    html += "</div>";

    html += "</div>";

    html += "<div class='card'>";

    html += "<h2>System</h2>";

    html += "<div class='value'>";
    html += "Uptime: ";
    html += String(millis() / 1000);
    html += " s";
    html += "</div>";

    html += "<div class='value'>";
    html += "WebServer: Running";
    html += "</div>";

    html += "</div>";

    html += "<a href='/settings'>Settings</a>";

    html += "</div>";

    html += "</body>";
    html += "</html>";

    return html;
}


String getSettingsPage()
{
    String html;

    html += "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";

    html += "<meta name='viewport' ";
    html += "content='width=device-width, initial-scale=1'>";

    html += "<title>Settings - testili9848</title>";

    html += "<style>";

    html += "body{";
    html += "font-family:Arial,sans-serif;";
    html += "background:#111;";
    html += "color:#fff;";
    html += "margin:0;";
    html += "padding:20px;";
    html += "}";

    html += ".container{";
    html += "max-width:600px;";
    html += "margin:auto;";
    html += "}";

    html += ".card{";
    html += "background:#222;";
    html += "border-radius:12px;";
    html += "padding:20px;";
    html += "}";

    html += "h1{";
    html += "margin-top:0;";
    html += "}";

    html += ".setting{";
    html += "background:#333;";
    html += "padding:15px;";
    html += "border-radius:8px;";
    html += "margin-bottom:10px;";
    html += "}";

    html += "a{";
    html += "display:inline-block;";
    html += "background:#333;";
    html += "color:#fff;";
    html += "padding:12px 18px;";
    html += "border-radius:8px;";
    html += "text-decoration:none;";
    html += "margin-top:10px;";
    html += "}";

    html += "</style>";

    html += "</head>";

    html += "<body>";

    html += "<div class='container'>";

    html += "<div class='card'>";

    html += "<h1>Settings</h1>";

    html += "<div class='setting'>";
    html += "<b>Brightness</b>";
    html += "<p>Coming soon</p>";
    html += "</div>";

    html += "<div class='setting'>";
    html += "<b>Clock format</b>";
    html += "<p>12 / 24 hour</p>";
    html += "</div>";

    html += "<div class='setting'>";
    html += "<b>Date</b>";
    html += "<p>Show / Hide</p>";
    html += "</div>";

    html += "<div class='setting'>";
    html += "<b>Weather</b>";
    html += "<p>Show / Hide</p>";
    html += "</div>";

    html += "<a href='/'>Back to Dashboard</a>";

    html += "</div>";

    html += "</div>";

    html += "</body>";
    html += "</html>";

    return html;
}