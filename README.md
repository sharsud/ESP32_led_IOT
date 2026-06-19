# ESP32 Modular LED Controller Ecosystem

An incremental, phase-by-phase migration and upgrade of a FastLED-based addressable LED controller. This project transitions a baseline sound-reactive WS2812B setup from an Arduino Uno architecture into an advanced, wireless ESP32 system utilizing Bluetooth, Local Web Servers, Captive Portals, WebSockets, and Cloud IoT integration.

---

## 🛠️ Hardware Requirements

* **Microcontroller:** ESP32 Development Board (e.g., ESP32-WROOM-32E)
* **LED Strip:** WS2812B / WS2811 (212 LEDs baseline)
* **Power Supply:** 5V DC Power Supply (Amperage sized to match total LED count)
* **Logic Level Shifter:** 74HCT125 or matching IC (Converts ESP32's 3.3V logic data line to the 5V signal expected by the LED strip)
* **Audio Sensor:** Analog Microphone Module (Connected to ADC1)

---

## 🗺️ Project Architecture & Roadmap

This repository is strictly structured into modular phases. Each phase represents a complete, functional system upgrade that establishes a stable baseline before moving to the next.

```text
ESP32_led_IOT/
├── phase01-bluetooth-baseline/  -> Local Bluetooth Classic control (Current)
├── phase02-local-webserver/     -> Hardcoded Wi-Fi hosting HTML controls
├── phase03-captive-portal/      -> Smart Wi-Fi manager (No hardcoded credentials)
├── phase04-async-websockets/    -> Zero-latency real-time slider communication
└── phase05-iot-cloud/           -> Global internet access via MQTT / IoT framework