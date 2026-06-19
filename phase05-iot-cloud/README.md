# Phase 5: Global IoT Cloud Integration

## Objective
Extend control past the local network boundary, allowing secure visual management and state modifications from any external internet network using an IoT broker abstraction.

## Selected Stack Alternatives
- **Option A (Custom/Industrial):** Configure an external MQTT Client library routing commands through a public/private broker framework (like HiveMQ or Mosquitto).
- **Option B (Managed Framework):** Integrate the `Blink` or `SinricPro` system profiles using their pre-configured secure web application hooks.

## Functional Requirements
- Implement TLS encryption wrappers for secure external cloud stream operations.
- Introduce persistent connection keep-alive handshakes that automatically recover connection dropouts without stalling runtime LED processes.

## Exit Criteria
- Disabling local Wi-Fi connection on a smartphone and routing commands via cellular data successfully modifies the active lighting pattern on the device.