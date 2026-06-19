# Phase 4: Real-Time Async WebSockets

## Objective
Replace standard synchronous HTTP polling requests with persistent TCP WebSockets channels to eliminate command processing latency and prevent visual pattern stuttering during rapid value changes.

## Software Requirements
- `ESPAsyncWebServer` Library
- `AsyncTCP` Library

## Core Architectural Upgrades
- Replace standard `WebServer.h` with an asynchronous network server architecture that runs non-blocking routines alongside FastLED drawing frames.
- Establish a full-duplex binary/text pipeline (`ws://<ip>/ws`).
- JavaScript event listeners send immediate positional coordinates on slider change events without opening and closing HTTP connection loops.

## Exit Criteria
- Dragging the UI color wheel produces a real-time, fluid color shift on the physical LED strip without dropped frames or stuttering animations.