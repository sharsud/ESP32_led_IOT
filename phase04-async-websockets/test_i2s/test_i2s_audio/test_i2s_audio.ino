/*
 * INMP441 I2S Microphone Test with LED Visualization
 * Using ESP_I2S library (higher-level API to avoid ADC conflicts)
 *
 * Hardware Setup:
 * INMP441 -> ESP32
 * VDD  -> 3.3V
 * GND  -> GND
 * SCK  -> D18 (GPIO 18)
 * WS   -> D5 (GPIO 5)
 * SD   -> D4 (GPIO 4)
 * L/R  -> GND
 *
 * LED Data -> D2 (GPIO 2)
 */

#include <Arduino.h>
#include <ESP_I2S.h>
#include <FastLED.h>
#include <math.h>

// =====================================================
// WS2812
// =====================================================

#define LED_PIN          2
#define NUM_LEDS         60
#define LED_BRIGHTNESS   255

CRGB leds[NUM_LEDS];

// =====================================================
// INMP441
// =====================================================

#define I2S_BCLK 18
#define I2S_WS   5
#define I2S_DIN  4

#define SAMPLE_RATE 16000

I2SClass I2S;

// =====================================================
// AUDIO SETTINGS
// =====================================================

// Based on your quiet-room measurements
float noiseFloor = 70000.0;

// Signal required for maximum LED output
float maxLevel = 1000000.0;

// Runtime sensitivity
float sensitivity = 1.0;

// Smoothed audio level
float audioLevel = 0;

// =====================================================
// NOISE GATE / HYSTERESIS
// =====================================================

// LED must reach this level before turning on
float gateOpen = 80000.0;

// Once ON, it stays ON until level falls below this
float gateClose = 50000.0;

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println("==========================================");
  Serial.println("INMP441 I2S Audio + LED Test (ESP_I2S lib)");
  Serial.println("==========================================");

  // -------------------------
  // FastLED
  // -------------------------

  Serial.println("[SETUP] Initializing FastLED...");
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  Serial.println("[SETUP] FastLED initialized");

  // -------------------------
  // I2S
  // -------------------------

  Serial.println("[SETUP] Initializing I2S with ESP_I2S library...");
  I2S.setPins(
    I2S_BCLK,
    I2S_WS,
    -1,
    I2S_DIN,
    -1
  );

  if (!I2S.begin(
        I2S_MODE_STD,
        SAMPLE_RATE,
        I2S_DATA_BIT_WIDTH_32BIT,
        I2S_SLOT_MODE_MONO,
        I2S_STD_SLOT_LEFT
      )) {

    Serial.println("[SETUP] I2S ERROR - Init failed");
    fill_solid(leds, 10, CRGB::Red);
    FastLED.show();
    while (true) {
      delay(1000);
    }
  }

  Serial.println("[SETUP] I2S initialized successfully");
  Serial.println("[SETUP] System Ready!");
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  s 1.0   = sensitivity");
  Serial.println("  s 2.0   = more sensitive");
  Serial.println("  s 0.5   = less sensitive");
  Serial.println("  s       = show sensitivity");
  Serial.println();
}

// =====================================================
// SERIAL COMMANDS
// =====================================================

void processSerial() {

  if (!Serial.available()) {
    return;
  }

  String command = Serial.readStringUntil('\n');
  command.trim();

  if (command.equalsIgnoreCase("s")) {

    Serial.print("Sensitivity: ");
    Serial.println(sensitivity, 2);

    return;
  }

  if (command.startsWith("s ") ||
      command.startsWith("S ")) {

    float value = command.substring(2).toFloat();

    if (value > 0.01 && value <= 10.0) {

      sensitivity = value;

      Serial.print("Sensitivity changed to: ");
      Serial.println(sensitivity, 2);

    } else {

      Serial.println("Use sensitivity between 0.01 and 10.0");
    }
  }
}

// =====================================================
// AUDIO PROCESSING
// =====================================================

float readAudioLevel() {

  const int SAMPLES = 160;

  int32_t samples[SAMPLES];

  float total = 0;
  int validSamples = 0;

  // ---------------------------------------------------
  // Read one complete block
  // ---------------------------------------------------

  for (int i = 0; i < SAMPLES; i++) {

    int32_t raw = I2S.read();

    if (raw == 0 || raw == 1) {
      samples[i] = 0;
      continue;
    }

    int32_t sample = raw >> 8;

    samples[i] = sample;

    total += sample;
    validSamples++;
  }

  if (validSamples == 0) {
    return 0;
  }

  // ---------------------------------------------------
  // Calculate DC average
  // ---------------------------------------------------

  float average = total / validSamples;

  // ---------------------------------------------------
  // Calculate average absolute amplitude
  // ---------------------------------------------------

  float amplitude = 0;
  int valid = 0;

  for (int i = 0; i < SAMPLES; i++) {

    if (samples[i] == 0) {
      continue;
    }

    float value = fabs(
      (float)samples[i] - average
    );

    amplitude += value;
    valid++;
  }

  if (valid == 0) {
    return 0;
  }

  return amplitude / valid;
}

// =====================================================
// LED STATE
// =====================================================

bool ledsActive = false;

// =====================================================
// LOOP
// =====================================================

void loop() {

  processSerial();

  // ---------------------------------------------------
  // Read audio
  // ---------------------------------------------------

  float level = readAudioLevel();

  // Apply sensitivity
  level *= sensitivity;

  // ---------------------------------------------------
  // Noise gate with hysteresis
  // ---------------------------------------------------

  if (!ledsActive) {

    // Currently OFF
    // Need a stronger signal to wake up

    if (level >= gateOpen) {
      ledsActive = true;
    } else {
      level = 0;
    }

  } else {

    // Currently ON
    // Allow it to fall lower before switching OFF

    if (level <= gateClose) {
      ledsActive = false;
    }
  }

  // ---------------------------------------------------
  // Remove noise floor
  // ---------------------------------------------------

  float usableLevel = 0;

  if (ledsActive) {

    usableLevel = level - noiseFloor;

    if (usableLevel < 0) {
      usableLevel = 0;
    }
  }

  // ---------------------------------------------------
  // Smooth audio
  // ---------------------------------------------------

  if (usableLevel > audioLevel) {

    // Fast attack
    audioLevel +=
      (usableLevel - audioLevel) * 0.50;

  } else {

    // Slow decay
    audioLevel +=
      (usableLevel - audioLevel) * 0.06;
  }

  // ---------------------------------------------------
  // Convert audio to LED brightness
  // ---------------------------------------------------

  float brightness = 0;

  if (audioLevel > 0) {

    brightness =
      (audioLevel /
      (maxLevel - noiseFloor)) * 255.0;
  }

  brightness = constrain(
    brightness,
    0,
    255
  );

  // ---------------------------------------------------
  // LED
  // ---------------------------------------------------

  if (!ledsActive || brightness < 1) {

    fill_solid(
      leds,
      NUM_LEDS,
      CRGB::Black
    );

  } else {

    CRGB color = CRGB(
      brightness * 0.05,
      brightness * 0.35,
      brightness
    );

    fill_solid(
      leds,
      NUM_LEDS,
      color
    );
  }

  FastLED.show();

  // ---------------------------------------------------
  // Debug
  // ---------------------------------------------------

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    Serial.print("Audio: ");
    Serial.print((int)level);
    Serial.print("  LED: ");
    Serial.println((int)brightness);
    lastPrint = millis();
  }

  delay(15);
}
