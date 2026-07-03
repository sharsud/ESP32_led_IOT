#ifndef EFFECTS_H
#define EFFECTS_H

#include <FastLED.h>

// --- Linkage to Main Framework Global State Registers ---
extern CRGB targetLeds[];
extern uint8_t gHue;
extern uint8_t globalSpeed;
extern uint8_t globalDensity;
extern uint8_t pickerRed;
extern uint8_t pickerGreen;
extern uint8_t pickerBlue;
extern int dynamicNumLeds;

#define MIC_LOW           160.0
extern float dynamicMicHigh;
extern float smoothedLedCount;
extern void calculateAGC(int currentSample);
extern int const_of_d(int val, int low, int high);

// --- Internal Helper Block ---
void addGlitter(fract8 chanceOfGlitter) { 
  if (random8() < chanceOfGlitter) { 
    targetLeds[random16(dynamicNumLeds)] += CRGB::White;
  } 
}

// ============================================================================
// VISUAL ANIMATION ENGINES (0 to 36)
// ============================================================================

void rainbow() {
  uint8_t deltaHue = map(globalDensity, 0, 255, 2, 25);
  fill_rainbow(targetLeds, dynamicNumLeds, gHue, deltaHue);
}

void rainbowWithGlitter() { 
  rainbow(); 
 
  fadeToBlackBy(targetLeds, dynamicNumLeds, 20);
    addGlitter(globalDensity);
}

void confetti() {
  uint8_t fadeVal = map(globalDensity, 0, 255, 2, 40);
  fadeToBlackBy(targetLeds, dynamicNumLeds, fadeVal);
  int pos = random16(dynamicNumLeds);
  targetLeds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon() {
  uint8_t fadeVal = map(globalDensity, 0, 255, 10, 60);
  fadeToBlackBy(targetLeds, dynamicNumLeds, fadeVal);
  uint8_t calculatedBpm = map(globalSpeed, 0, 255, 4, 45);
  int pos = beatsin16(calculatedBpm, 0, dynamicNumLeds - 1);
  targetLeds[pos] += CHSV(gHue, 255, 192);
}

void bpm() {
  uint8_t BeatsPerMinute = map(globalSpeed, 0, 255, 20, 180);
  uint8_t beatSpread = map(globalDensity, 0, 255, 2, 20);
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < dynamicNumLeds; i++) {
    targetLeds[i] = ColorFromPalette(PartyColors_p, gHue + (i * 2), beat - gHue + (i * beatSpread));
  }
}

void juggle() {
  fadeToBlackBy(targetLeds, dynamicNumLeds, 20);
  byte dothue = 0;
  uint8_t activeDots = map(globalDensity, 0, 255, 2, 12);
  uint8_t motionSpeed = map(globalSpeed, 0, 255, 1, 10);
  for (int i = 0; i < activeDots; i++) {
    targetLeds[beatsin16(i + 7 + motionSpeed, 0, dynamicNumLeds - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void visualize_music() {
  int sensor_value = analogRead(34); // Hardware Pin[cite: 1, 3]
  if (sensor_value < MIC_LOW) {
    sensor_value = MIC_LOW; // Cutoff floor boundary[cite: 1]
  }
  calculateAGC(sensor_value); // Baseline calculation engine[cite: 1, 3]
  
  // Apply our scaling factor multiplier (Normalized centered around 5)
  float manualGainModifier = (float)audioSens / 5.0;
  
  // Inject the modifier safely into target active bounds mapping
  int targetLedCount = int((((dynamicNumLeds / dynamicMicHigh) * sensor_value) / 2) * manualGainModifier);
  
  smoothedLedCount = (0.15 * targetLedCount) + (0.85 * smoothedLedCount); // Temporal lowpass damping filter[cite: 1, 3]
  int activeBound = (int)smoothedLedCount;
  
  // Boundary safety clamp constraint to stop buffer array overflow risks
  if (activeBound > (dynamicNumLeds / 2)) {
    activeBound = (dynamicNumLeds / 2);
  }
  
  uint8_t noLed = uint8_t(dynamicNumLeds / 2); // Anchor midpoint point allocation[cite: 1]
  int maxLed = noLed + activeBound;
  int minLed = noLed - activeBound;
  
  fadeToBlackBy(targetLeds, dynamicNumLeds, 255); 
  
  uint8_t localHue = gHue;
  for (int i = noLed; i >= minLed; i--) {
    if (i >= 0) { targetLeds[i] = CHSV(localHue, 255, 255); } // Lower safety buffer check[cite: 1]
    localHue += 3;
  }
  localHue = gHue;
  for (int i = noLed + 1; i < maxLed; i++) {
    if (i < dynamicNumLeds) { targetLeds[i] = CHSV(localHue, 255, 255); } // Upper safety buffer check[cite: 1]
    localHue += 3;
  }
}

void visualize_music_solids() {
  int sensor_value = analogRead(34);
  if (sensor_value < MIC_LOW) {
    sensor_value = MIC_LOW;
  }
  calculateAGC(sensor_value);
  int targetLedCount = int(((dynamicNumLeds / dynamicMicHigh) * sensor_value) / 2);
  smoothedLedCount = (0.15 * targetLedCount) + (0.85 * smoothedLedCount);
  int activeBound = (int)smoothedLedCount;
  uint8_t noLed = uint8_t(dynamicNumLeds / 2);
  int maxLed = noLed + activeBound;
  int minLed = noLed - activeBound;
  fadeToBlackBy(targetLeds, 300, 255);
  CRGB solidPickerColor = CRGB(pickerRed, pickerGreen, pickerBlue);
  for (int i = noLed; i >= minLed; i--) {
    targetLeds[i] = solidPickerColor;
  }
  for (int i = noLed + 1; i < maxLed; i++) {
    targetLeds[i] = solidPickerColor;
  }
}

void visualize_music_dual_tone() {
  int sensor_value = analogRead(34);
  if (sensor_value < MIC_LOW) {
    sensor_value = MIC_LOW;
  }
  calculateAGC(sensor_value);
  int targetLedCount = int(((dynamicNumLeds / dynamicMicHigh) * sensor_value) / 2);
  smoothedLedCount = (0.15 * targetLedCount) + (0.85 * smoothedLedCount);
  int activeBound = (int)smoothedLedCount;
  uint8_t noLed = uint8_t(dynamicNumLeds / 2);
  int maxLed = noLed + activeBound;
  int minLed = noLed - activeBound;
  fadeToBlackBy(targetLeds, 300, 255);
  for (int i = noLed; i >= minLed; i--) {
    targetLeds[i] = CRGB(pickerRed, pickerGreen, pickerBlue);
  }
  uint8_t localHue = gHue;
  for (int i = noLed + 1; i < maxLed; i++) {
    targetLeds[i] = CHSV(localHue, 255, 255);
    localHue += 3;
  }
}

void Fire2012() {
  static byte heat[300];
  uint8_t cooling = map(globalDensity, 0, 255, 10, 110);
  uint8_t sparking = map(globalSpeed, 0, 255, 10, 200);
  for (int i = 0; i < dynamicNumLeds; i++) {
    heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / dynamicNumLeds) + 2));
  }
  for (int k = dynamicNumLeds - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
  if (random8() < sparking) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }
  fadeToBlackBy(targetLeds, 300, 255);
  for (int j = 0; j < dynamicNumLeds; j++) {
    targetLeds[j] = HeatColor(heat[j]);
  }
}

void pacmanGhostChase() {
  CRGB ghostColors[] = {CRGB::Red, CRGB::DeepPink, CRGB::Cyan, CRGB::Orange};
  fadeToBlackBy(targetLeds, dynamicNumLeds, 40);
  uint8_t calculatedSpeed = map(globalSpeed, 0, 255, 5, 40);
  uint8_t spacing = map(globalDensity, 0, 255, 2, 12);
  int pos = beatsin16(calculatedSpeed, 0, dynamicNumLeds - (spacing * 4));
  for (int i = 0; i < 4; i++) {
    int ghostPos = pos + (i * spacing);
    if (ghostPos < dynamicNumLeds) {
      targetLeds[ghostPos] = ghostColors[i];
    }
  }
}

void oceanWaves() {
  uint8_t speedOscillation = map(globalSpeed, 0, 255, 1, 12);
  uint8_t spatialDensity = map(globalDensity, 0, 255, 1, 8);
  for (int i = 0; i < dynamicNumLeds; i++) {
    uint8_t index = beatsin8(speedOscillation, 0, 255) + (i * spatialDensity);
    uint8_t brightness = beatsin8(speedOscillation + 3, 100, 255, 0, i * 3);
    targetLeds[i] = ColorFromPalette(OceanColors_p, index, brightness, LINEARBLEND);
  }
}

void plasmaNoise() {
  static uint16_t x = 0;
  static uint16_t y = 0;
  uint8_t speedX = map(globalSpeed, 0, 255, 1, 10);
  uint8_t densityScale = map(globalDensity, 0, 255, 5, 60);
  for (int i = 0; i < dynamicNumLeds; i++) {
    uint8_t noiseVal = inoise8(i * densityScale + x, y);
    targetLeds[i] = CHSV(noiseVal + gHue, 255, inoise8(i * 10 + y, x));
  }
  x += speedX;
  y += (speedX + 2);
}

void strobeParty() {
  static bool isOn = false;
  uint8_t timingMs = map(globalSpeed, 0, 255, 200, 15);
  EVERY_N_MILLISECONDS_I(strobeTimer, 50) {
    strobeTimer.setPeriod(timingMs);
    isOn = !isOn;
  }
  for (int i = 0; i < dynamicNumLeds; i++) {
    if (isOn) {
      targetLeds[i] = (random8() < globalDensity) ? CRGB::White : CRGB(CHSV(gHue + (i * 2), 255, 255));
    } else {
      targetLeds[i] = CRGB::Black;
    }
  }
}

void meteorRain() {
  uint8_t decayVal = map(globalDensity, 0, 255, 30, 150);
  for (int i = 0; i < dynamicNumLeds; i++) {
    if (random8() < 50) {
      targetLeds[i].fadeToBlackBy(decayVal);
    }
  }
  uint8_t cycleSpeed = map(globalSpeed, 0, 255, 60, 10);
  EVERY_N_MILLISECONDS_I(meteorTimer, 30) {
    meteorTimer.setPeriod(cycleSpeed);
    static int meteorPos = 0;
    meteorPos++;
    if (meteorPos >= dynamicNumLeds) {
      meteorPos = 0;
    }
    for (int j = 0; j < 4; j++) {
      if ((meteorPos - j >= 0) && (meteorPos - j < dynamicNumLeds)) {
        targetLeds[meteorPos - j] = (j == 0) ? CRGB::White : CRGB(CHSV(gHue, 255, 255 - (j * 50)));
      }
    }
  }
}

void cyberpunkNeon() {
  uint8_t speed1 = map(globalSpeed, 0, 255, 1, 15);
  uint8_t spatialDensity = map(globalDensity, 0, 255, 1, 8);
  for (int i = 0; i < dynamicNumLeds; i++) {
    uint8_t pinkWave = beatsin8(speed1, 0, 255, 0, i * spatialDensity);
    uint8_t cyanWave = beatsin8(speed1 + 2, 0, 255, 0, i * (spatialDensity * 2));
    targetLeds[i] = CRGB(pinkWave, 0, cyanWave);
  }
}

void auroraBorealis() {
  static uint16_t noiseOffset = 0;
  uint8_t evolutionSpeed = map(globalSpeed, 0, 255, 1, 8);
  uint8_t matrixDensity = map(globalDensity, 0, 255, 5, 45);
  for (int i = 0; i < dynamicNumLeds; i++) {
    uint8_t noiseVal = inoise8(i * matrixDensity, noiseOffset);
    uint8_t hueVal = map(noiseVal, 0, 255, 80, 190);
    targetLeds[i] = CHSV(hueVal, 240, inoise8(noiseOffset, i * 10));
  }
  noiseOffset += evolutionSpeed;
  if (random8() < 15) {
    targetLeds[random16(dynamicNumLeds)] += CRGB(40, 40, 40);
  }
}

void rainbowVortex() {
  uint8_t stepSpread = map(globalDensity, 0, 255, 2, 40);
  fill_rainbow(targetLeds, dynamicNumLeds, gHue * 4, stepSpread);
}

void customPulseBreathing() {
  uint8_t breathingCycle = map(globalSpeed, 0, 255, 2, 20);
  uint8_t floorLimit = map(globalDensity, 0, 255, 0, 80);
  uint8_t breathLuminance = beatsin8(breathingCycle, floorLimit, 255);
  for (int i = 0; i < dynamicNumLeds; i++) {
    targetLeds[i] = CRGB(pickerRed, pickerGreen, pickerBlue);
    targetLeds[i].nscale8(breathLuminance);
  }
}

void customColorScanner() {
  uint8_t traceDecay = map(globalDensity, 0, 255, 10, 90);
  fadeToBlackBy(targetLeds, dynamicNumLeds, traceDecay);
  uint8_t scanSpeed = map(globalSpeed, 0, 255, 3, 35);
  int scanPos = beatsin16(scanSpeed, 0, dynamicNumLeds - 1);
  targetLeds[scanPos] = CRGB(pickerRed, pickerGreen, pickerBlue);
  if (scanPos + 1 < dynamicNumLeds) {
    targetLeds[scanPos + 1] += CRGB(pickerRed / 2, pickerGreen / 2, pickerBlue / 2);
  }
  if (scanPos - 1 >= 0) {
    targetLeds[scanPos - 1] += CRGB(pickerRed / 2, pickerGreen / 2, pickerBlue / 2);
  }
}

void solidColorPicker() {
  fill_solid(targetLeds, dynamicNumLeds, CRGB(pickerRed, pickerGreen, pickerBlue));
}

void warmFireflyShimmer() {
  uint8_t fadeSpeed = map(globalDensity, 0, 255, 5, 45);
  fadeToBlackBy(targetLeds, dynamicNumLeds, fadeSpeed);
  uint8_t spawnRate = map(globalSpeed, 0, 255, 1, 60);
  if (random8() < spawnRate) {
    int targetPixel = random16(dynamicNumLeds);
    targetLeds[targetPixel] = CHSV(random8(28, 42), 240, random8(180, 255));
  }
}

void cozyFireplaceCrackle() {
  uint8_t cooling = map(globalDensity, 0, 255, 10, 80);
  uint8_t sparkChance = map(globalSpeed, 0, 255, 10, 150);
  for (int i = 0; i < dynamicNumLeds; i++) {
    targetLeds[i].fadeToBlackBy(random8(0, ((cooling * 10) / dynamicNumLeds) + 4));
  }
  for (int k = dynamicNumLeds - 1; k >= 2; k--) {
    targetLeds[k] = (targetLeds[k - 1] + targetLeds[k - 2] + targetLeds[k - 2]) / 3;
  }
  if (random8() < sparkChance) {
    int location = random16(dynamicNumLeds);
    targetLeds[location] = CHSV(random8(0, 18), 255, random8(150, 255));
  }
}

void movingPastelWave() {
  uint8_t waveSpeed = map(globalSpeed, 0, 255, 1, 15);
  static uint16_t waveTracker = 0;
  waveTracker += waveSpeed;
  uint8_t waveDensity = map(globalDensity, 0, 255, 4, 40);
  for (int i = 0; i < dynamicNumLeds; i++) {
    uint8_t hueCoordinate = (i * waveDensity) + (waveTracker / 10);
    targetLeds[i] = CHSV(hueCoordinate, 135, 255);
  }
}

void organicNebulaDrift() {
  static uint32_t zAxis = 0;
  uint16_t speedOffset = map(globalSpeed, 0, 255, 2, 24);
  uint16_t spaceDensity = map(globalDensity, 0, 255, 8, 90);
  zAxis += speedOffset;
  for (int i = 0; i < dynamicNumLeds; i++) {
    uint8_t noise1 = inoise8(i * spaceDensity, zAxis);
    uint8_t noise2 = inoise8(zAxis, i * spaceDensity);
    targetLeds[i] = CHSV(noise1 + gHue, 220, qadd8(noise2, 40));
  }
}

void loungeWineBreathing() {
  uint8_t cyclePeriod = map(globalSpeed, 0, 255, 3, 20);
  uint8_t paletteRange = map(globalDensity, 0, 255, 0, 45);
  uint8_t breatheVal = beatsin8(cyclePeriod, 30, 255);
  uint8_t colorShift = beatsin8(2, 0, paletteRange);
  for (int i = 0; i < dynamicNumLeds; i++) {
    targetLeds[i] = CHSV(240 + colorShift, 255, breatheVal);
  }
}

void kineticSandGlass() {
  uint8_t clearRate = map(globalDensity, 0, 255, 50, 5);
  fadeToBlackBy(targetLeds, dynamicNumLeds, clearRate);
  uint8_t dropVelocity = map(globalSpeed, 0, 255, 5, 50);
  int activePixel = beatsin16(dropVelocity, 0, dynamicNumLeds - 1);
  targetLeds[activePixel] = CHSV(gHue, 180, 255);
  targetLeds[(activePixel + dynamicNumLeds / 2) % dynamicNumLeds] += CHSV(gHue + 128, 200, 200);
}

void theatreMarquee() {
  uint8_t moveSpeed = map(globalSpeed, 0, 255, 0, 20);
  static uint32_t movementClock = 0;
  if (moveSpeed > 0) {
    movementClock += moveSpeed;
  }
  uint8_t blockSpacing = map(globalDensity, 0, 255, 4, 32);
  uint32_t offsetIndex = movementClock / 40;
  for (int i = 0; i < dynamicNumLeds; i++) {
    if ((i + offsetIndex) % blockSpacing < (blockSpacing / 2)) {
      targetLeds[i] = CHSV(gHue, 240, 255);
    } else {
      targetLeds[i] = CRGB::Black;
    }
  }
}

void cosmicDustSupernova() {
  uint8_t twistSpeed = map(globalSpeed, 0, 255, 1, 6);
  static uint16_t coreOffset = 0;
  coreOffset += twistSpeed;
  uint8_t blastChance = map(globalDensity, 0, 255, 1, 35);
  CRGB basePickerColor = CRGB(pickerRed, pickerGreen, pickerBlue);
  for (int i = 0; i < dynamicNumLeds; i++) {
    uint8_t noiseLuminance = inoise8(coreOffset, i * 8) / 2;
    targetLeds[i] = basePickerColor;
    targetLeds[i].nscale8(noiseLuminance);
  }
  if (random8() < blastChance) {
    int burstCenter = random16(dynamicNumLeds);
    int radius = random8(3, 12);
    for (int j = -radius; j <= radius; j++) {
      int idx = burstCenter + j;
      if (idx >= 0 && idx < dynamicNumLeds) {
        targetLeds[idx] += CRGB(255, 255, 255);
      }
    }
  }
}

void glacialIceMelt() {
  uint8_t shiftVel = map(globalSpeed, 0, 255, 2, 22);
  static uint32_t thermalClock = 0;
  thermalClock += shiftVel;
  uint8_t ridgeThickness = map(globalDensity, 0, 255, 10, 110);
  CRGB basePickerColor = CRGB(pickerRed, pickerGreen, pickerBlue);
  CRGB fissureColor = CRGB(255 - pickerRed, 255 - pickerGreen, 255 - pickerBlue);
  if (fissureColor == CRGB(0,0,0)) {
    fissureColor = CRGB(0, 80, 180);
  }
  for (int i = 0; i < dynamicNumLeds; i++) {
    uint8_t fractureNoise = inoise8(i * ridgeThickness, thermalClock);
    if (fractureNoise > 165) {
      targetLeds[i] = fissureColor;
    } else {
      targetLeds[i] = basePickerColor;
    }
  }
}

void matrixDigitalRain() {
  uint8_t trailDecay = map(globalDensity, 0, 255, 45, 8);
  fadeToBlackBy(targetLeds, dynamicNumLeds, trailDecay);
  uint8_t dropFreq = map(globalSpeed, 0, 255, 90, 12);
  CRGB rainColor = CRGB(pickerRed, pickerGreen, pickerBlue);
  EVERY_N_MILLISECONDS_I(rainTimer, 30) {
    rainTimer.setPeriod(dropFreq);
    static int headPos = 0;
    headPos = (headPos + 1) % dynamicNumLeds;
    if (random8() < 140) {
      targetLeds[headPos] = rainColor;
      if (headPos % 2 == 0) {
        targetLeds[headPos] += CRGB(100, 100, 100);
      }
    }
  }
}

void interstellarWarpDrive() {
  uint8_t decay = map(globalDensity, 0, 255, 120, 25);
  fadeToBlackBy(targetLeds, dynamicNumLeds, decay);
  uint8_t propulsionSpeed = map(globalSpeed, 0, 255, 4, 30);
  static int currentStep = 0;
  currentStep += propulsionSpeed;
  CRGB warpColor = CRGB(pickerRed, pickerGreen, pickerBlue);
  int midPoint = dynamicNumLeds / 2;
  int streamPos = (currentStep / 20) % (midPoint + 1);
  if (midPoint + streamPos < dynamicNumLeds) {
    targetLeds[midPoint + streamPos] = warpColor;
  }
  if (midPoint - streamPos >= 0) {
    targetLeds[midPoint - streamPos] = warpColor;
  }
}

void diamondPrismShimmer() {
  CRGB groundCanvas = CRGB(pickerRed, pickerGreen, pickerBlue);
  fill_solid(targetLeds, dynamicNumLeds, groundCanvas);
  uint8_t sparkleRate = map(globalSpeed, 0, 255, 2, 85);
  uint8_t prismWidth = map(globalDensity, 0, 255, 5, 40);
  if (random8() < sparkleRate) {
    int focalPixel = random16(dynamicNumLeds);
    targetLeds[focalPixel] = CRGB::White;
    for (int k = 1; k <= (prismWidth / 10); k++) {
      if (focalPixel + k < dynamicNumLeds) {
        targetLeds[focalPixel + k] += CHSV(gHue + (k * 15), 200, 150);
      }
      if (focalPixel - k >= 0) {
        targetLeds[focalPixel - k] += CHSV(gHue - (k * 15), 200, 150);
      }
    }
  }
}

void magmaChamberFissure() {
  uint8_t pressureSpeed = map(globalSpeed, 0, 255, 1, 9);
  static uint32_t magmaClock = 0;
  magmaClock += pressureSpeed;
  uint8_t ridgeGap = map(globalDensity, 0, 255, 4, 30);
  CRGB customMagma = CRGB(pickerRed, pickerGreen, pickerBlue);
  for (int i = 0; i < dynamicNumLeds; i++) {
    uint8_t pressureNoise = inoise8(magmaClock, i * ridgeGap);
    if (pressureNoise > 195) {
      targetLeds[i] = CRGB::White;
    } else {
      targetLeds[i] = customMagma;
      uint8_t dynamicDimming = map(pressureNoise, 0, 195, 30, 255);
      targetLeds[i].nscale8(dynamicDimming);
    }
  }
}

void zenBambooForest() {
  uint8_t windPace = map(globalSpeed, 0, 255, 1, 8);
  static uint32_t foliageClock = 0;
  foliageClock += windPace;
  uint8_t canopyDepth = map(globalDensity, 0, 255, 2, 28);
  CRGB pickerColor = CRGB(pickerRed, pickerGreen, pickerBlue);
  for (int i = 0; i < dynamicNumLeds; i++) {
    uint8_t swayNoise = inoise8(i * canopyDepth, foliageClock);
    targetLeds[i] = pickerColor;
    if (swayNoise < 95) {
      targetLeds[i].nscale8(70);
    } else if (swayNoise < 175) {
      targetLeds[i].nscale8(180);
    } else {
      targetLeds[i] += CRGB(60, 60, 60);
    }
  }
}

void electricTeslaArc() {
  fadeToBlackBy(targetLeds, dynamicNumLeds, 75);
  uint8_t arcDischargeFreq = map(globalSpeed, 0, 255, 220, 20);
  uint8_t arcJumpLength = map(globalDensity, 0, 255, 2, 28);
  CRGB plasmaColor = CRGB(pickerRed, pickerGreen, pickerBlue);
  EVERY_N_MILLISECONDS_I(arcTimer, 40) {
    arcTimer.setPeriod(arcDischargeFreq);
    int anchorA = random16(dynamicNumLeds);
    int anchorB = const_of_d(anchorA + random8(-arcJumpLength, arcJumpLength), 0, dynamicNumLeds - 1);
    for (int i = min(anchorA, anchorB); i <= max(anchorA, anchorB); i++) {
      targetLeds[i] = plasmaColor;
    }
  }
}

void tokyoNeonRain() {
  for (int i = 0; i < dynamicNumLeds; i++) { 
    targetLeds[i].nscale8_video(240);
  }
  uint8_t dropSpeed = map(globalSpeed, 0, 255, 230, 25);
  uint8_t neonSpread = map(globalDensity, 0, 255, 10, 80);
  CRGB primarySplash = CRGB(pickerRed, pickerGreen, pickerBlue);
  CHSV hsvConvert = rgb2hsv_approximate(primarySplash);
  hsvConvert.hue += 42; 
  CRGB secondarySplash = hsvConvert;
  EVERY_N_MILLISECONDS_I(neonTimer, 30) {
    neonTimer.setPeriod(dropSpeed);
    int targetImpactPixel = random16(dynamicNumLeds);
    CRGB chosenNeonSplash = (random8() % 2 == 0) ? primarySplash : secondarySplash;
    targetLeds[targetImpactPixel] = chosenNeonSplash;
    if (targetImpactPixel + 1 < dynamicNumLeds && random8() < neonSpread) {
      targetLeds[targetImpactPixel + 1] = chosenNeonSplash;
    }
    if (targetImpactPixel - 1 >= 0 && random8() < neonSpread) {
      targetLeds[targetImpactPixel - 1] = chosenNeonSplash;
    }
  }
}

#endif