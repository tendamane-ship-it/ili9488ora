# ORA TV Smart Display

## Route and compatibility

`GET /tv` is registered on the existing port-80 `WebServer` in `WebServerManager::begin()`. Before this change there was no `/tv` route; `/` worked because it had its own handler and unknown paths fell through to the WebServer's 404 response.

The page is a separate, self-contained HTML/CSS/ES5 JavaScript document in `src/TvPage.cpp`. It is served from flash with `send_P`; it does not construct a large temporary Arduino `String`, add a server, load a framework, or add an external internet dependency. The dashboard at `/` and other handlers remain in place.

## Data sources

- `GET /api/status` supplies the device clock/date, connection state, identity, sensor readings and validity flags, IAQ estimate/category/accuracy, and current weather.
- `GET /api/history/info` and `GET /api/history?after=…&limit=100` supply real temperature, humidity and pressure samples. The TV requests only a bounded recent page and refreshes it every five minutes.
- The status response gains an additive `weatherDaily` array populated from the existing `WeatherService::getDaily()` values. Each item has date, min/max temperature, precipitation, precipitation probability, max wind, sunrise/sunset and WMO weather code. No new route is added.

The current firmware has no hourly forecast series, so the display uses the daily forecast that already exists in `WeatherService`. If current weather or sensor readings are invalid, the page shows an unavailable state instead of a fabricated value. On a lost connection it clears current readings and retries automatically.

## TV behavior and resource use

The layout targets 16:9 1920×1080 and scales for 4K. It has a flexbox fallback and uses CSS Grid only when supported. The clock digits are drawn as CSS seven-segment elements. The page avoids frameworks, CSS custom properties, and modern JavaScript syntax and includes a manual Fullscreen control with browser-prefixed fallbacks. The built-in Samsung browser may still restrict fullscreen or vary CSS/canvas support by TV model and firmware; test on the target TV.

The browser polls `/api/status` every 15 seconds, the sensor history every five minutes, and advances the displayed clock locally between status responses. The HTML is stored in program memory and served with `send_P`. The route logs free heap before and after serving. Runtime heap could not be sampled during development because `192.168.1.55` was unreachable from the build environment; the firmware's `/api/status` already exposes `heap` for on-device measurement.

## Firmware update status

The page and additive status fields are source changes only. They have not been uploaded to an ORA device. Build locally for `esp32-s3-devkitc-1`, then check `/tv`, `/`, `/api/status`, and free heap on hardware before installing a firmware release.
