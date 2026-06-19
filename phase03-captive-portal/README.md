# Phase 3: Captive Portal Wi-Fi Manager

## Objective
Eliminate hardcoded Wi-Fi credentials. Implement an automated captive configuration portal so the controller can dynamically connect to any available wireless network without firmware modification.

## Software Requirements
- `WiFiManager` Library by tzapu (via Arduino Library Manager)

## Functional Workflow
1. On boot, the ESP32 attempts to connect to the last saved network profile in its Non-Volatile Storage (NVS).
2. If connection fails or times out, the ESP32 shifts automatically to Access Point (AP) mode, broadcasting an open network named `ESP32-LED-SETUP`.
3. Connecting a smartphone to this AP triggers a captive portal window displaying available local SSIDs.
4. Saving credentials reboots the ESP32 into standard station mode.

## Exit Criteria
- Wiping known router profiles forces the chip into AP state.
- The captive webpage successfully saves configuration info, transitions back to standard operational routines, and runs the baseline LED loops.