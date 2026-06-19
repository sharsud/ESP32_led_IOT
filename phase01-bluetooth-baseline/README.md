# Phase 1: Bluetooth Classic Baseline

## Objective
Migrate the original Arduino Uno code to the ESP32 architecture and replace physical hardware Serial control with Bluetooth Serial communication.

## Hardware Configuration
- **Board:** ESP32 Dev Module
- **LED Pin:** GPIO 2
- **Microphone Analog Pin:** GPIO 34 (ADC1)
- **Logic Level Shifter:** 3.3V to 5V (e.g., 74HCT125) required for WS2812 data line stability.

## Software Requirements
- `FastLED` Library
- `BluetoothSerial` Library (Built into ESP32 Core)
- **Arduino IDE Partition Scheme:** Set to `Huge APP (3MB No OTA)` to accommodate the Bluetooth stack.

## Data Schema & Controls
Commands are sent as raw integers via a Bluetooth Terminal App (Baud rate: 115200):
- `0 - 5`: Standard FastLED animations (Rainbow, Confetti, etc.)
- `6`: Reactive Music Visualizer (Dynamic Rainbow)
- `7 - 12`: Reactive Music Visualizer (Solid Colors)
- `13`: Fire2012 Simulation

## Exit Criteria
- Code compiles completely without space allocation errors.
- ESP32 pairs cleanly with mobile device as "ESP32_LED_Controller".
- Standard single integer inputs switch patterns without crashing or causing execution lag.