#include <FastLED.h>

// --- Hardware Constraints ---
#define MAX_LEDS          212
#define LED_PIN           2          
#define ANALOG_READ       34         
#define LED_TYPE          WS2811
#define COLOR_ORDER       GRB

// --- Sound Velocity Constants ---
#define MIC_LOW           160.0
#define MIC_HIGH          3200.0

// --- Structural Global Registers ---
CRGB leds[MAX_LEDS];
uint8_t currentPatternIndex = 0;
uint8_t gHue = 0;

// --- DYNAMIC PARAMETERS ---
uint8_t globalBrightness = 100;
int dynamicNumLeds = MAX_LEDS; // Defaults to the full strip length

// --- Picker Targets ---
uint8_t pickerRed = 255;   
uint8_t pickerGreen = 140; 
uint8_t pickerBlue = 0;
static float smoothedLedCount = 0.0;

// --- Function Prototypes ---
void runSelectedPattern(uint8_t patternId);
void handleSerialCommunication();
void rainbow();
void rainbowWithGlitter();
void confetti();
void sinelon();
void juggle();
void bpm();
void visualize_music();
void visualize_music_solids(uint8_t profile);
void visualize_music_dual_tone();
void Fire2012();
void pacmanGhostChase();
void oceanWaves();
void plasmaNoise();
void strobeParty();
void meteorRain();
void cyberpunkNeon();
void auroraBorealis();
void rainbowVortex();
void customPulseBreathing();
void customColorScanner();
void addGlitter(fract8 chanceOfGlitter);

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(5); 
  Serial.println("System Core Booted.");
  Serial.println("Format syntax: EffectID,Brightness,LEDCount (e.g. 15,150,100)");

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, MAX_LEDS).setCorrection(TypicalLEDStrip);
  
  // Brownout safety guard
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 450); 
  FastLED.setBrightness(globalBrightness);

  FastLED.clear(true);
}

void loop() {
  handleSerialCommunication();
  runSelectedPattern(currentPatternIndex);
  
  FastLED.show();
  FastLED.delay(1000 / 120); // 120 Frames Per Second Target
  
  EVERY_N_MILLISECONDS(20) { gHue++; }
}

// --- Comma-Separated Serial Input Parser ---
void handleSerialCommunication() {
  if (Serial.available() > 0) {
    // Read the entire incoming string until a newline character arrives
    String inputBuffer = Serial.readStringUntil('\n');
    inputBuffer.trim(); // Clean up any hidden spaces, carriage returns (\r), or newlines (\n)

    if (inputBuffer.length() > 0) {
      int tempPattern = currentPatternIndex;
      int tempBrightness = globalBrightness;
      int tempLeds = dynamicNumLeds;

      // Extract values using a strict format match: "ID,Brightness,LEDCount"
      // sscanf returns how many variables it successfully populated
      int parsedFields = sscanf(inputBuffer.c_str(), "%d,%d,%d", &tempPattern, &tempBrightness, &tempLeds);

      // We need at least the first value (Pattern ID) to process anything
      if (parsedFields >= 1) {
        if (tempPattern >= 0 && tempPattern <= 24) {
          if (tempPattern != currentPatternIndex) {
            currentPatternIndex = tempPattern;
            Serial.print("Pattern -> "); Serial.println(currentPatternIndex);
            FastLED.clear(true); // Flash frame transitions instantly
          }
        }
      }

      // If the second value (Brightness) was provided in the comma string
      if (parsedFields >= 2) {
        if (tempBrightness >= 0 && tempBrightness <= 255) {
          if (tempBrightness != globalBrightness) {
            globalBrightness = tempBrightness;
            FastLED.setBrightness(globalBrightness);
            Serial.print("Global Brightness -> "); Serial.println(globalBrightness);
          }
        }
      }

      // If the third value (LED Count) was provided in the comma string
      if (parsedFields >= 3) {
        if (tempLeds > 0 && tempLeds < MAX_LEDS) {
          if (tempLeds != dynamicNumLeds) {
            dynamicNumLeds = tempLeds;
            Serial.print("Active LED Count -> "); Serial.println(dynamicNumLeds);
            FastLED.clear(true); // Clear old hanging pixels out of the active boundary
          }
        }
        if ( tempLeds >= MAX_LEDS) {
          if (tempLeds != dynamicNumLeds) {
            dynamicNumLeds = MAX_LEDS;
            Serial.print("Active LED Count -> "); Serial.println(dynamicNumLeds);
            FastLED.clear(true); // Clear old hanging pixels out of the active boundary
          }
        }
      }
    }
  }
}

void runSelectedPattern(uint8_t patternId) {
  bool standardRenderPipeline = true;

  switch (patternId) {
    case 0:  rainbow();            break;
    case 1:  rainbowWithGlitter(); break;
    case 2:  confetti();           break;
    case 3:  sinelon();            break;
    case 4:  juggle();             break;
    case 5:  bpm();                break;
    case 6:  visualize_music();           standardRenderPipeline = false; break;
    case 7:  case 8:  case 9: 
    case 10: case 11: case 12: 
      visualize_music_solids(patternId);  standardRenderPipeline = false; break;
    case 13: Fire2012();                  standardRenderPipeline = false; break;
    case 14: pacmanGhostChase();   break;
    case 15: oceanWaves();        break;
    case 16: plasmaNoise();       break;
    case 17: strobeParty();       break;
    case 18: meteorRain();        break;
    case 19: cyberpunkNeon();     break;
    case 20: auroraBorealis();    break;
    case 21: rainbowVortex();     break;
    case 22: customPulseBreathing(); break;
    case 23: customColorScanner();   break;
    case 24: visualize_music_dual_tone(); standardRenderPipeline = false; break;
    default: rainbow();            break; 
  }

  // Clear unmapped background pixels beyond the dynamic active limit
  if (dynamicNumLeds < MAX_LEDS) {
    fill_solid(leds + dynamicNumLeds, MAX_LEDS - dynamicNumLeds, CRGB::Black);
  }

  if (standardRenderPipeline) {
    // Global clock parameters are handled in the main loop thread cleanly
  }
}

// ============================================================================
// MODULAR EFFECTS MATRIX (Updated loops to use dynamicNumLeds)
// ============================================================================

void rainbow() {
  fill_rainbow(leds, dynamicNumLeds, gHue, 7);
}

void rainbowWithGlitter() {
  rainbow();
  addGlitter(80);
  fadeToBlackBy(leds, dynamicNumLeds, 20);
}

void addGlitter(fract8 chanceOfGlitter) {
  if (random8() < chanceOfGlitter) {
    leds[random16(dynamicNumLeds)] += CRGB::White;
  }
}

void confetti() {
  fadeToBlackBy(leds, dynamicNumLeds, 10);
  int pos = random16(dynamicNumLeds);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon() {
  fadeToBlackBy(leds, dynamicNumLeds, 20);
  int pos = beatsin16(13, 0, dynamicNumLeds - 1);
  leds[pos] += CHSV(gHue, 255, 192);
}

void bpm() {
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < dynamicNumLeds; i++) {
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  fadeToBlackBy(leds, dynamicNumLeds, 20);
  byte dothue = 0;
  for (int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, dynamicNumLeds - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void visualize_music() {
  int sensor_value = analogRead(ANALOG_READ);
  if (sensor_value < MIC_LOW) sensor_value = MIC_LOW;
  if (sensor_value > MIC_HIGH) sensor_value = MIC_HIGH;
  
  int targetLedCount = int(((dynamicNumLeds / MIC_HIGH) * sensor_value) / 2);
  smoothedLedCount = (0.15 * targetLedCount) + (0.85 * smoothedLedCount);
  int activeBound = (int)smoothedLedCount;

  uint8_t noLed = uint8_t(dynamicNumLeds / 2);
  int maxLed = noLed + activeBound;
  int minLed = noLed - activeBound;
  
  fadeToBlackBy(leds, MAX_LEDS, 255); 

  uint8_t localHue = gHue;
  for (int i = noLed; i >= minLed; i--) { leds[i] = CHSV(localHue, 255, 255); localHue += 3; }
  localHue = gHue;
  for (int i = noLed + 1; i < maxLed; i++) { leds[i] = CHSV(localHue, 255, 255); localHue += 3; }
  delay(4);
}

void visualize_music_solids(uint8_t profile) {
  int sensor_value = analogRead(ANALOG_READ);
  if (sensor_value < MIC_LOW) sensor_value = MIC_LOW;
  if (sensor_value > MIC_HIGH) sensor_value = MIC_HIGH;
  
  int targetLedCount = int(((dynamicNumLeds / MIC_HIGH) * sensor_value) / 2);
  smoothedLedCount = (0.15 * targetLedCount) + (0.85 * smoothedLedCount);
  int activeBound = (int)smoothedLedCount;

  uint8_t noLed = uint8_t(dynamicNumLeds / 2);
  int maxLed = noLed + activeBound;
  int minLed = noLed - activeBound;
  
  CRGB solidColor;
  if      (profile == 7)  solidColor = CRGB(255, 0, 0);
  else if (profile == 8)  solidColor = CRGB(0, 255, 0);
  else if (profile == 9)  solidColor = CRGB(0, 0, 255); // 0,0,255
  else if (profile == 10) solidColor = CRGB(255, 0, 255);
  else if (profile == 11) solidColor = CRGB(0, 255, 255);
  else if (profile == 12) solidColor = CRGB(255, 255, 0);
  if (profile == 9) solidColor = CRGB(0, 0, 255);

  fadeToBlackBy(leds, MAX_LEDS, 255); 

  for (int i = noLed; i >= minLed; i--)     leds[i] = solidColor;
  for (int i = noLed + 1; i < maxLed; i++) leds[i] = solidColor;
  delay(4);
}

void visualize_music_dual_tone() {
  int sensor_value = analogRead(ANALOG_READ);
  if (sensor_value < MIC_LOW) sensor_value = MIC_LOW;
  if (sensor_value > MIC_HIGH) sensor_value = MIC_HIGH;
  
  int targetLedCount = int(((dynamicNumLeds / MIC_HIGH) * sensor_value) / 2);
  smoothedLedCount = (0.15 * targetLedCount) + (0.85 * smoothedLedCount);
  int activeBound = (int)smoothedLedCount;

  uint8_t noLed = uint8_t(dynamicNumLeds / 2);
  int maxLed = noLed + activeBound;
  int minLed = noLed - activeBound;
  
  fadeToBlackBy(leds, MAX_LEDS, 255); 

  for (int i = noLed; i >= minLed; i--) leds[i] = CRGB(pickerRed, pickerGreen, pickerBlue);
  uint8_t localHue = gHue;
  for (int i = noLed + 1; i < maxLed; i++) { leds[i] = CHSV(localHue, 255, 255); localHue += 3; }
  delay(4);
}

void Fire2012() {
  static byte heat[MAX_LEDS];
  for (int i = 0; i < dynamicNumLeds; i++) {
    heat[i] = qsub8(heat[i], random8(0, ((20 * 10) / dynamicNumLeds) + 2));
  }
  for (int k = dynamicNumLeds - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
  if (random8() < 20) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }
  fadeToBlackBy(leds, MAX_LEDS, 255);
  for (int j = 0; j < dynamicNumLeds; j++) {
    leds[j] = HeatColor(heat[j]);
  }
}

void pacmanGhostChase() {
  CRGB ghostColors[] = {CRGB::Red, CRGB::DeepPink, CRGB::Cyan, CRGB::Orange};
  fadeToBlackBy(leds, dynamicNumLeds, 40);
  int pos = beatsin16(15, 0, dynamicNumLeds - 20);
  for(int i = 0; i < 4; i++) {
    int ghostPos = pos + (i * 5);
    if(ghostPos < dynamicNumLeds) leds[ghostPos] = ghostColors[i];
  }
}

void oceanWaves() {
  CRGBPalette16 oceanPalette = OceanColors_p;
  for(int i = 0; i < dynamicNumLeds; i++) {
    uint8_t index = beatsin8(4, 0, 255) + (i * 2);
    uint8_t brightness = beatsin8(7, 100, 255, 0, i * 3);
    leds[i] = ColorFromPalette(oceanPalette, index, brightness, LINEARBLEND);
  }
}

void plasmaNoise() {
  static uint16_t x = 0;
  static uint16_t y = 0;
  for(int i = 0; i < dynamicNumLeds; i++) {
    uint8_t noiseVal = inoise8(i * 30 + x, y);
    leds[i] = CHSV(noiseVal + gHue, 255, inoise8(i * 10 + y, x));
  }
  x += 3; y += 5;
}

void strobeParty() {
  static bool isOn = false;
  EVERY_N_MILLISECONDS(50) { isOn = !isOn; }
  for(int i = 0; i < dynamicNumLeds; i++) {
    leds[i] = isOn ? ((random8() < 40) ? CRGB::White : CRGB(CHSV(gHue + (i * 2), 255, 255))) : CRGB::Black;
  }
}

void meteorRain() {
  for(int i=0; i<dynamicNumLeds; i++) {
    if(random8() < 50) { leds[i].fadeToBlackBy(80); }
  }
  EVERY_N_MILLISECONDS(30) {
    static int meteorPos = 0;
    meteorPos++;
    if(meteorPos >= dynamicNumLeds) { meteorPos = 0; }
    for(int j = 0; j < 4; j++) {
      if((meteorPos - j >= 0) && (meteorPos - j < dynamicNumLeds)) {
        leds[meteorPos - j] = (j == 0) ? CRGB::White : CRGB(CHSV(gHue, 255, 255 - (j * 50)));
      }
    }
  }
}

void cyberpunkNeon() {
  for (int i = 0; i < dynamicNumLeds; i++) {
    uint8_t pinkWave = beatsin8(5, 0, 255, 0, i * 2);
    uint8_t cyanWave = beatsin8(7, 0, 255, 0, i * 4);
    leds[i] = CRGB(pinkWave, 0, cyanWave);
  }
}

void auroraBorealis() {
  static uint16_t noiseOffset = 0;
  for (int i = 0; i < dynamicNumLeds; i++) {
    uint8_t noiseVal = inoise8(i * 15, noiseOffset);
    uint8_t hueVal = map(noiseVal, 0, 255, 80, 190);
    leds[i] = CHSV(hueVal, 240, inoise8(noiseOffset, i * 10));
  }
  noiseOffset += 2;
  if (random8() < 5) leds[random16(dynamicNumLeds)] += CRGB(40, 40, 40);
}

void rainbowVortex() {
  fill_rainbow(leds, dynamicNumLeds, gHue * 4, 15);
}

void customPulseBreathing() {
  uint8_t breathLuminance = beatsin8(6, 20, 255); 
  for (int i = 0; i < dynamicNumLeds; i++) {
    leds[i] = CRGB(pickerRed, pickerGreen, pickerBlue);
    leds[i].nscale8(breathLuminance);
  }
}

void customColorScanner() {
  fadeToBlackBy(leds, dynamicNumLeds, 35); 
  int scanPos = beatsin16(12, 0, dynamicNumLeds - 1); 
  leds[scanPos] = CRGB(pickerRed, pickerGreen, pickerBlue);
  if(scanPos + 1 < dynamicNumLeds) leds[scanPos + 1] += CRGB(pickerRed/2, pickerGreen/2, pickerBlue/2);
  if(scanPos - 1 >= 0)       leds[scanPos - 1] += CRGB(pickerRed/2, pickerGreen/2, pickerBlue/2);
}