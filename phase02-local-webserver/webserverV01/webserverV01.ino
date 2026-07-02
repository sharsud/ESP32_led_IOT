#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>

// --- Hardware Constraints ---
#define MAX_LEDS          300        // Expanded hardware ceiling
#define LED_PIN           2          
#define ANALOG_READ       34         
#define LED_TYPE          WS2811
#define COLOR_ORDER       GRB

// --- Temporary Wi-Fi Station Credentials ---
const char* ssid = "Leenderweg74A";
const char* password = "Lovelife@9742";

// --- Network Infrastructure Instance ---
WebServer server(80);

// --- Advanced Audio Infrastructure registers ---
#define MIC_LOW           160.0
static float dynamicMicHigh = 1500.0;
static float smoothedLedCount = 0.0;

// --- Dual-Array Crossfade Architecture ---
CRGB leds[MAX_LEDS];       
CRGB targetLeds[MAX_LEDS]; 
uint8_t crossfadeSpeed = 15;

// --- System Global State Registers ---
uint8_t currentPatternIndex = 0;
uint8_t gHue = 0;
uint8_t globalBrightness = 100;
int dynamicNumLeds = 212;            // Keeps 212 pixels as the system boot default

// --- ADVANCED INTERACTIVE MODIFIER REGISTERS ---
uint8_t globalSpeed = 120;   
uint8_t globalDensity = 30;

// --- DYNAMIC COLOR PICKER TARGETS ---
uint8_t pickerRed = 255;   
uint8_t pickerGreen = 140; 
uint8_t pickerBlue = 0;

// --- Function Prototypes ---
void runSelectedPattern(uint8_t patternId);
void handleSerialCommunication();
void handleWebUpdateRequest();
void handleWebRootRequest();
void blendFrames();
void calculateAGC(int currentSample);
int const_of_d(int val, int low, int high);

// --- Pattern Matrix Prototype Map (Sequenced 0 - 36) ---
void rainbow();
void rainbowWithGlitter(); 
void confetti(); 
void sinelon(); 
void juggle(); 
void bpm();
void visualize_music(); 
void visualize_music_solids(); 
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
void solidColorPicker();
void warmFireflyShimmer(); 
void cozyFireplaceCrackle(); 
void movingPastelWave();
void organicNebulaDrift(); 
void loungeWineBreathing(); 
void kineticSandGlass(); 
void theatreMarquee();
void cosmicDustSupernova(); 
void glacialIceMelt(); 
void matrixDigitalRain();
void interstellarWarpDrive();
void diamondPrismShimmer(); 
void magmaChamberFissure(); 
void zenBambooForest(); 
void electricTeslaArc();
void tokyoNeonRain(); 
void addGlitter(fract8 chanceOfGlitter);

void setup() {
  Serial.begin(115200);
  while (!Serial) { 
    ; 
  }
  delay(500); 
  Serial.setTimeout(5); 

  Serial.println("==================================================================");
  Serial.println("System Core Architecture Online. Initiating Phase 2 Wi-Fi Setup.");
  Serial.println("==================================================================");

  // Initialize Station Mode
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi Access Point: ");
  Serial.println(ssid);

  // Non-blocking wait visualization loop via Serial
  int connectionAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && connectionAttempts < 30) {
    delay(500);
    Serial.print(".");
    connectionAttempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("==================================================================");
    Serial.println("Wi-Fi Station Connection Verified successfully!");
    Serial.print("Local Target IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("==================================================================");
  } else {
    Serial.println("");
    Serial.println("WARNING: Wi-Fi connection timed out. Falling back to standalone mode.");
  }

  // Define Web Server Request Rules
  server.on("/", HTTP_GET, handleWebRootRequest);
  server.on("/update", HTTP_GET, handleWebUpdateRequest);
  server.begin();
  Serial.println("Asynchronous Local HTTP Web Server Layer Online.");

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, MAX_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 450); 
  FastLED.setBrightness(globalBrightness);
  FastLED.clear(true);
}

void loop() {
  // Poll Network and Communication Buffer Queues
  server.handleClient();
  handleSerialCommunication();
  
  runSelectedPattern(currentPatternIndex);
  blendFrames();
  
  FastLED.show();
  FastLED.delay(1000 / 120); 
  
  uint8_t scaledHueSpeed = map(globalSpeed, 0, 255, 1, 10);
  EVERY_N_MILLISECONDS(20) { 
    gHue += scaledHueSpeed; 
  }
}

void handleWebRootRequest() {
  // Native HTML5 payload delivery matching execution constraints
  String htmlPayload = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  htmlPayload += "<title>ESP32 LED Panel Console</title>";
  htmlPayload += "<style>body{font-family:sans-serif;background:#111;color:#eee;text-align:center;padding-top:30px;}";
  htmlPayload += ".container{max-width:400px;margin:0 auto;background:#222;padding:20px;border-radius:10px;box-shadow:0 4px 10px rgba(0,0,0,0.5);}";
  htmlPayload += "div{margin-bottom:25px;}label{display:block;font-size:1.1em;margin-bottom:10px;font-weight:bold;}";
  htmlPayload += "input[type='color']{-webkit-appearance:none;border:none;width:100%;height:60px;border-radius:5px;cursor:pointer;}";
  htmlPayload += "input[type='range']{width:100%;height:10px;border-radius:5px;cursor:pointer;}";
  htmlPayload += ".val-display{font-size:0.9em;color:#aaa;margin-top:5px;}</style></head><body>";
  htmlPayload += "<div class=\"container\"><h2>System Control Panel</h2>";
  
  // Pattern Index Selection UI
  htmlPayload += "<div><label>Active Animation Preset</label>";
  htmlPayload += "<input type=\"range\" id=\"pattern\" min=\"0\" max=\"36\" value=\"" + String(currentPatternIndex) + "\" oninput=\"dispatchParameters()\">";
  htmlPayload += "<div class=\"val-display\">Pattern ID: <span id=\"patternVal\">" + String(currentPatternIndex) + "</span></div></div>";
  
  // Color Picker UI
  char hexBuffer[8];
  sprintf(hexBuffer, "#%02X%02X%02X", pickerRed, pickerGreen, pickerBlue);
  htmlPayload += "<div><label>Hardware Color Register</label>";
  htmlPayload += "<input type=\"color\" id=\"picker\" value=\"" + String(hexBuffer) + "\" onchange=\"dispatchParameters()\"></div>";
  
  // Truncation Range Slider UI - Capped at dynamic 300 Max value ceiling
  htmlPayload += "<div><label>Active Strip Boundary Limit</label>";
  htmlPayload += "<input type=\"range\" id=\"numLeds\" min=\"1\" max=\"" + String(MAX_LEDS) + "\" value=\"" + String(dynamicNumLeds) + "\" oninput=\"dispatchParameters()\">";
  htmlPayload += "<div class=\"val-display\">Pixel Limit: <span id=\"numLedsVal\">" + String(dynamicNumLeds) + "</span> / " + String(MAX_LEDS) + "</div></div>";
  
  // JavaScript background fetch logic
  htmlPayload += "</div><script>function dispatchParameters(){";
  htmlPayload += "var patternStr = document.getElementById('pattern').value;";
  htmlPayload += "var colorHex = document.getElementById('picker').value;";
  htmlPayload += "var numStr = document.getElementById('numLeds').value;";
  htmlPayload += "document.getElementById('patternVal').innerText = patternStr;";
  htmlPayload += "document.getElementById('numLedsVal').innerText = numStr;";
  htmlPayload += "var r = parseInt(colorHex.substring(1,3),16);";
  htmlPayload += "var g = parseInt(colorHex.substring(3,5),16);";
  htmlPayload += "var b = parseInt(colorHex.substring(5,7),16);";
  htmlPayload += "var targetUrl = '/update?p=' + patternStr + '&r=' + r + '&g=' + g + '&b=' + b + '&num=' + numStr;";
  htmlPayload += "fetch(targetUrl);}</script></body></html>";
  
  server.send(200, "text/html", htmlPayload);
}

void handleWebUpdateRequest() {
  if (server.hasArg("p")) {
    int parsedPattern = server.arg("p").toInt();
    if (parsedPattern >= 0 && parsedPattern <= 36) {
      currentPatternIndex = parsedPattern;
    }
  }
  
  if (server.hasArg("r")) {
    pickerRed = constrain(server.arg("r").toInt(), 0, 255);
  }
  
  if (server.hasArg("g")) {
    pickerGreen = constrain(server.arg("g").toInt(), 0, 255);
  }
  
  if (server.hasArg("b")) {
    pickerBlue = constrain(server.arg("b").toInt(), 0, 255);
  }
  
  if (server.hasArg("num")) {
    int parsedTruncation = server.arg("num").toInt();
    if (parsedTruncation >= 1 && parsedTruncation <= MAX_LEDS) {
      dynamicNumLeds = parsedTruncation;
    }
  }
  
  server.send(200, "text/plain", "OK");
}

void blendFrames() {
  nblend(leds, targetLeds, MAX_LEDS, crossfadeSpeed);
  for (int i = 0; i < MAX_LEDS; i++) {
    if (!targetLeds[i]) { 
      leds[i] = CRGB::Black;
    }
  }
  
  // Real-Time Truncation Guard
  if (dynamicNumLeds < MAX_LEDS) {
    fill_solid(leds + dynamicNumLeds, MAX_LEDS - dynamicNumLeds, CRGB::Black);
    fill_solid(targetLeds + dynamicNumLeds, MAX_LEDS - dynamicNumLeds, CRGB::Black);
  }
}

void handleSerialCommunication() {
  if (Serial.available() > 0) {
    String inputBuffer = Serial.readStringUntil('\n');
    inputBuffer.trim(); 

    if (inputBuffer.length() > 0) {
      int tempPattern = currentPatternIndex;
      int tempBrightness = globalBrightness;
      int tempLeds = dynamicNumLeds;
      int tempSpeed = globalSpeed;
      int tempDensity = globalDensity;
      int r = pickerRed;
      int g = pickerGreen;
      int b = pickerBlue;

      int parsedFields = sscanf(inputBuffer.c_str(), "%d,%d,%d,%d,%d,%d,%d,%d", 
                               &tempPattern, &tempBrightness, &tempLeds, 
                               &tempSpeed, &tempDensity, &r, &g, &b);
      if (parsedFields >= 1 && tempPattern >= 0 && tempPattern <= 36) {
        if (tempPattern != currentPatternIndex) { 
          currentPatternIndex = tempPattern;
        }
      }
      if (parsedFields >= 2 && tempBrightness >= 0 && tempBrightness <= 255) {
        if (tempBrightness != globalBrightness) { 
          globalBrightness = tempBrightness;
          FastLED.setBrightness(globalBrightness); 
        }
      }
      if (parsedFields >= 3 && tempLeds > 0 && tempLeds <= MAX_LEDS) {
        if (tempLeds != dynamicNumLeds) { 
          dynamicNumLeds = tempLeds;
        }
      }
      if (parsedFields >= 4 && tempSpeed >= 0 && tempSpeed <= 255)  { 
        globalSpeed = tempSpeed;
      }
      if (parsedFields >= 5 && tempDensity >= 0 && tempDensity <= 255) { 
        globalDensity = tempDensity;
      }
      if (parsedFields == 8) {
        pickerRed = constrain(r, 0, 255);
        pickerGreen = constrain(g, 0, 255);
        pickerBlue = constrain(b, 0, 255);
      }
    }
  }
}

void calculateAGC(int currentSample) {
  static int peakSample = 500;
  uint8_t decayRate = map(globalSpeed, 0, 255, 1, 8);
  
  EVERY_N_MILLISECONDS(15) { 
    if (peakSample > 400) {
      peakSample -= decayRate;
    }
  }
  
  if (currentSample > peakSample) { 
    peakSample = currentSample;
  }
  dynamicMicHigh = peakSample;
}

void runSelectedPattern(uint8_t patternId) {
  switch (patternId) {
    case 0:  rainbow();                   break;
    case 1:  rainbowWithGlitter();        break;
    case 2:  confetti();                  break;
    case 3:  sinelon();                   break;
    case 4:  juggle();                    break;
    case 5:  bpm();                       break;
    case 6:  visualize_music();           break;
    case 7:  visualize_music_solids();    break; 
    case 8:  visualize_music_dual_tone(); break;
    case 9:  Fire2012();                  break;
    case 10: pacmanGhostChase();          break;
    case 11: oceanWaves();                break;
    case 12: plasmaNoise();               break;
    case 13: strobeParty();               break;
    case 14: meteorRain();                break;
    case 15: cyberpunkNeon();             break;
    case 16: auroraBorealis();            break;
    case 17: rainbowVortex();             break;
    case 18: customPulseBreathing();      break;
    case 19: customColorScanner();        break;
    case 20: solidColorPicker();          break; 
    case 21: warmFireflyShimmer();        break;
    case 22: cozyFireplaceCrackle();      break;
    case 23: movingPastelWave();          break;
    case 24: organicNebulaDrift();        break;
    case 25: loungeWineBreathing();       break;
    case 26: kineticSandGlass();          break;
    case 27: theatreMarquee();            break;
    case 28: cosmicDustSupernova();       break;
    case 29: glacialIceMelt();            break;
    case 30: matrixDigitalRain();         break;
    case 31: interstellarWarpDrive();     break;
    case 32: diamondPrismShimmer();       break;
    case 33: magmaChamberFissure();       break;
    case 34: zenBambooForest();           break;
    case 35: electricTeslaArc();          break;
    case 36: tokyoNeonRain();             break;
    default: rainbow();                   break;
  }
}

// ============================================================================
// VISUAL ANIMATION ENGINES (0 to 27)
// ============================================================================

void rainbow() {
  uint8_t deltaHue = map(globalDensity, 0, 255, 2, 25);
  fill_rainbow(targetLeds, dynamicNumLeds, gHue, deltaHue);
}

void rainbowWithGlitter() { 
  rainbow(); 
  addGlitter(globalDensity); 
  fadeToBlackBy(targetLeds, dynamicNumLeds, 20);
}

void addGlitter(fract8 chanceOfGlitter) { 
  if (random8() < chanceOfGlitter) { 
    targetLeds[random16(dynamicNumLeds)] += CRGB::White;
  } 
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
  int sensor_value = analogRead(ANALOG_READ);
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
  fadeToBlackBy(targetLeds, MAX_LEDS, 255);
  uint8_t localHue = gHue;
  for (int i = noLed; i >= minLed; i--) {
    targetLeds[i] = CHSV(localHue, 255, 255);
    localHue += 3;
  }
  localHue = gHue;
  for (int i = noLed + 1; i < maxLed; i++) {
    targetLeds[i] = CHSV(localHue, 255, 255);
    localHue += 3;
  }
}

void visualize_music_solids() {
  int sensor_value = analogRead(ANALOG_READ);
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
  fadeToBlackBy(targetLeds, MAX_LEDS, 255);
  CRGB solidPickerColor = CRGB(pickerRed, pickerGreen, pickerBlue);
  for (int i = noLed; i >= minLed; i--) {
    targetLeds[i] = solidPickerColor;
  }
  for (int i = noLed + 1; i < maxLed; i++) {
    targetLeds[i] = solidPickerColor;
  }
}

void visualize_music_dual_tone() {
  int sensor_value = analogRead(ANALOG_READ);
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
  fadeToBlackBy(targetLeds, MAX_LEDS, 255);
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
  static byte heat[MAX_LEDS];
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
  fadeToBlackBy(targetLeds, MAX_LEDS, 255);
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

// ============================================================================
// ADVANCED MODULES WITH DYNAMIC COLOR PICKER INTEGRATION (28 to 36)
// ============================================================================

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

int const_of_d(int val, int low, int high) { 
  if (val < low) {
    return low;
  }
  if (val > high) {
    return high;
  }
  return val;
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