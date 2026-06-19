# Phase 2: Local Wi-Fi Web Server

## Objective
Transition from Bluetooth to Wi-Fi station mode. The ESP32 will host a basic HTML page allowing color picking and active LED limits via standard web inputs over the local network.

## Core Architectural Upgrades
- Remove `BluetoothSerial.h` and introduce `WiFi.h` and `WebServer.h`.
- Hardcode temporary Wi-Fi SSID and Password variables for initial network connection verification.
- Modify `#define NUM_LEDS` logic into a dynamic variable boundary limit loop (`activeLeds`) to allow on-the-fly strip truncation.

## Web Interface Specifications
The hosted index page serves a single HTML payload containing:
- An HTML5 native color picker (`<input type="color">`).
- An active LED count range slider (`<input type="range">`).
- A background JavaScript `fetch()` script that converts inputs to query parameters (`/update?r=255&g=0&b=0&num=150`).

## Exit Criteria
- Device successfully connects to local access point and prints its local IP address to the Serial Monitor.
- Navigating to the IP address on a browser displays the control page.
- Modifying the slider successfully limits the active drawn LEDs on the physical strip.