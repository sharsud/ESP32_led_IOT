#include <FastLED.h>
#include "BluetoothSerial.h" // ESP32 Specific Bluetooth Library

// Verify Bluetooth is enabled in configuration
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif

BluetoothSerial ESP_BT; // Create Bluetooth object

/** BASIC CONFIGURATION **/
#define NUM_LEDS 212
#define LED_PIN 2         // Ensure this matches your physical ESP32 GPIO pin
#define ANALOG_READ 34     // ESP32 GPIO34 (ADC1_CH6) - Safe analog pin for microphone

#define LED_TYPE     WS2811 // Change to WS2812B if needed, keeping your baseline
#define COLOR_ORDER  GRB

// Scaled up by 4x for ESP32's 12-bit ADC (0-4095)
#define MIC_LOW 160.0
#define MIC_HIGH 3200.0

uint8_t old, incoming;
CRGB leds[NUM_LEDS];

#define BRIGHTNESS         100
#define FRAMES_PER_SECOND  120

uint8_t gHue = 0, noLed, minLed, maxLed;

// Forward declarations
void visualize_music();
void visualize_music_solids(uint8_t pat);
void Fire2012();
void rainbow();
void rainbowWithGlitter();
void addGlitter(fract8 chanceOfGlitter);
void confetti();
void sinelon();
void bpm();
void juggle();
//git Upload
void setup() {
  old = 0;
  Serial.begin(115200); // Standard ESP32 baud rate
  
  // Initialize Bluetooth Classic with a visible broadcast name
  ESP_BT.begin("ESP32_LED_Controller"); 
  Serial.println("Bluetooth Device is Ready to Pair!");

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  // Startup Test Sequence
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(80, 20, 100);
  FastLED.show();
  delay(1000);
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(0, 0, 0);
  FastLED.show();
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm};


void loop() {
  // Read from Bluetooth instead of standard Hardware Serial
  if (ESP_BT.available() > 0) {
    incoming = ESP_BT.parseInt();
    if ((incoming != 0) && (incoming != old)) {
      Serial.print("Bluetooth Received: ");
      Serial.println(incoming);
      old = incoming;
    }
  }

  if ((old >= 0) && (old <= 5)) {
    gPatterns[old]();
    FastLED.show();
    FastLED.delay(1000 / FRAMES_PER_SECOND);

    EVERY_N_MILLISECONDS(20) {
      gHue++; 
    }
  }
  else if (old == 6) {
    visualize_music();
  }
  else if ((old >= 7) && (old <= 12)) {
    visualize_music_solids(old);
  }
  else if (old == 13) {
    Fire2012();
  }
}

// ============================================================================
// LIGHT PATTERNS & EFFECTS (Preserved from Baseline)
// ============================================================================

void visualize_music() {
  int sensor_value, ledCount;
  sensor_value = analogRead(ANALOG_READ);
  
  if (sensor_value <= MIC_LOW) return;
  if (sensor_value >= MIC_HIGH) sensor_value = MIC_HIGH;
  
  ledCount = int(((NUM_LEDS / MIC_HIGH) * sensor_value) / 2);
  noLed = uint8_t(NUM_LEDS / 2);
  maxLed = noLed + ledCount;
  minLed = noLed - ledCount;
  
  if (maxLed > NUM_LEDS) maxLed = NUM_LEDS;
  if (minLed <= 0) minLed = 0;
  
  for (int i = noLed; i >= minLed; i--) leds[i] = CHSV(gHue += 3, 255, 255);
  for (int i = noLed + 1; i < maxLed; i++) leds[i] = CHSV(gHue += 3, 255, 255);
  
  FastLED.show();
  delay(10);

  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(0, 0, 0);
  FastLED.show();
}

void visualize_music_solids(uint8_t pat) {
  int sensor_value, ledCount;
  sensor_value = analogRead(ANALOG_READ);
  
  if (sensor_value <= MIC_LOW) return;
  if (sensor_value >= MIC_HIGH) sensor_value = MIC_HIGH;
  
  ledCount = int(((NUM_LEDS / MIC_HIGH) * sensor_value) / 2);
  noLed = uint8_t(NUM_LEDS / 2);
  maxLed = noLed + ledCount;
  minLed = noLed - ledCount;
  
  if (maxLed > NUM_LEDS) maxLed = NUM_LEDS;
  if (minLed <= 0) minLed = 0;
  
  for (int i = noLed; i >= minLed; i--) {
    if (pat == 7)       leds[i] = CRGB(255, 0, 0);
    else if (pat == 8)  leds[i] = CRGB(0, 255, 0);
    else if (pat == 9)  leds[i] = CRGB(0, 0, 255);
    else if (pat == 10) leds[i] = CRGB(255, 0, 255);
    else if (pat == 11) leds[i] = CRGB(0, 255, 255);
    else if (pat == 12) leds[i] = CRGB(255, 255, 0);
  }

  for (int i = noLed + 1; i < maxLed; i++) {
    if (pat == 7)       leds[i] = CRGB(255, 0, 0);
    else if (pat == 8)  leds[i] = CRGB(0, 255, 0);
    else if (pat == 9)  leds[i] = CRGB(0, 0, 255);
    else if (pat == 10) leds[i] = CRGB(255, 0, 255);
    else if (pat == 11) leds[i] = CRGB(0, 255, 255);
    else if (pat == 12) leds[i] = CRGB(255, 255, 0);
  }
  
  FastLED.show();
  delay(10);

  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(0, 0, 0);
  FastLED.show();
}

void rainbow() {
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() {
  rainbow();
  addGlitter(80);
  fadeToBlackBy(leds, NUM_LEDS, 20);
}

void addGlitter(fract8 chanceOfGlitter) {
  if (random8() < chanceOfGlitter) {
    leds[random16(NUM_LEDS)] += CRGB::White;
  }
}

void confetti() {
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS - 1);
  leds[pos] += CHSV(gHue, 255, 192);
}

void bpm() {
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  byte dothue = 0;
  for (int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

#define COOLING  20
#define SPARKING 20
bool gReverseDirection = false;

void Fire2012() {
  static byte heat[NUM_LEDS];
  for (int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }
  for (int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
  if (random8() < SPARKING) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }
  for (int j = 0; j < NUM_LEDS; j++) {
    CRGB color = HeatColor(heat[j]);
    int pixelnumber = gReverseDirection ? (NUM_LEDS - 1) - j : j;
    leds[pixelnumber] = color;
  }
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}