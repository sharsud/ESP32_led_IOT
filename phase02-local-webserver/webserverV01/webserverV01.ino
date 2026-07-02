#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

// --- Hardware Constraints ---
#define MAX_LEDS          300        
#define LED_PIN           2          
#define ANALOG_READ       34         
#define LED_TYPE          WS2811
#define COLOR_ORDER       GRB

// --- System Global State Registers ---
uint8_t currentPatternIndex = 0;
uint8_t gHue = 0;
uint8_t globalBrightness = 100;
int dynamicNumLeds = 212;            

// --- Advanced Interactive Modifier Registers ---
uint8_t globalSpeed = 120;   
uint8_t globalDensity = 30;

// --- Dynamic Color Picker Targets ---
uint8_t pickerRed = 255;   
uint8_t pickerGreen = 140; 
uint8_t pickerBlue = 0;

// --- Dual-Array Crossfade Architecture ---
CRGB leds[MAX_LEDS];       
CRGB targetLeds[MAX_LEDS]; 
uint8_t crossfadeSpeed = 15;

// --- Advanced Audio Infrastructure Registers ---
float dynamicMicHigh = 1500.0;
float smoothedLedCount = 0.0;

// --- Persistent Storage & Network Core Architecture ---
Preferences preferences;
WebServer server(80);
DNSServer dnsServer;

const byte DNS_PORT = 53;
bool isProvisionMode = false;
String scannedNetworksHTML = "";

// --- Core Helper Functions Used by Visual Engine ---
int const_of_d(int val, int low, int high) { 
  if (val < low) {
    return low;
  }
  if (val > high) {
    return high;
  }
  return val;
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

// --- Import Isolated Visual Presets Block ---
#include "Effects.h"

// --- Forward Declarations for Infrastructure Flow ---
void handleSerialCommunication();
void handleWebUpdateRequest();
void handleWebRootRequest();
void handlePortalRootRequest();
void handlePortalSaveRequest();
void blendFrames();
bool attemptWiFiConnection();
void launchCaptivePortal();

void setup() {
  Serial.begin(115200);
  while (!Serial) { 
    ; 
  }
  delay(500); 
  Serial.setTimeout(5); 

  Serial.println("==================================================================");
  Serial.println("System Core Architecture Online. Initializing Storage Engine.");
  Serial.println("==================================================================");

  // Initialize FastLED Hardware First to show startup indicators
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, MAX_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 450); 
  FastLED.setBrightness(globalBrightness);
  FastLED.clear(true);

  // Open storage namespace "wifi-config" in Read/Write mode
  preferences.begin("wifi-config", false);

  // Attempt connection with saved credentials
  if (attemptWiFiConnection()) {
    // Standard Operations Mode
    server.on("/", HTTP_GET, handleWebRootRequest);
    server.on("/update", HTTP_GET, handleWebUpdateRequest);
    server.begin();
    Serial.println("Asynchronous Local HTTP Web Server Layer Online.");
  } else {
    // Provisioning Mode (Captive Portal)
    launchCaptivePortal();
  }
}

void loop() {
  if (isProvisionMode) {
    dnsServer.processNextRequest();
    server.handleClient();
    
    // Cycle a soft pulsing blue shimmer on the strip to physically signal setup mode
    EVERY_N_MILLISECONDS(20) {
      uint8_t pulse = beatsin8(15, 20, 120);
      fill_solid(targetLeds, dynamicNumLeds, CRGB(0, 0, pulse));
      blendFrames();
      FastLED.show();
    }
  } else {
    // Normal Runtime Framework Execution
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
}

// --- Network Connectivity Engines ---

bool attemptWiFiConnection() {
  String savedSSID = preferences.getString("ssid", "");
  String savedPassword = preferences.getString("password", "");

  if (savedSSID == "") {
    Serial.println("No local Wi-Fi credentials found in persistent memory storage.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
  
  Serial.print("Connecting to saved Wi-Fi Network: ");
  Serial.println(savedSSID);

  int connectionAttempts = 0;
  // Orange flashing status bar across first 10 LEDs showing connection progress
  while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
    delay(500);
    Serial.print(".");
    connectionAttempts++;
    fill_solid(leds, 10, (connectionAttempts % 2 == 0) ? CRGB(255, 100, 0) : CRGB::Black);
    FastLED.show();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("==================================================================");
    Serial.println("Wi-Fi Connection successfully verified!");
    Serial.print("Local Target IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("==================================================================");
    
    // Green confirmation pulse on successful startup
    fill_solid(leds, dynamicNumLeds, CRGB::Green);
    FastLED.show();
    delay(500);
    return true;
  }

  Serial.println("\nFailed to connect to configured network. Timed out.");
  return false;
}

void launchCaptivePortal() {
  isProvisionMode = true;
  Serial.println("==================================================================");
  Serial.println("LAUNCHING CAPTIVE PROVISIONING PORTAL");
  Serial.println("==================================================================");

  // Initialize soft Access Point network
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-LED-Panel-Setup");
  delay(100); // Give radio hardware time to stabilize

  // Intercept every inbound web request and map it back to the softAP IP address
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  Serial.print("Access Point Broadcast Active: [ESP32-LED-Panel-Setup]\nLocal Portal Base URL: ");
  Serial.println(WiFi.softAPIP());

  // Scan local 2.4GHz airwaves to compile clean UI dropdown options
  Serial.println("Scanning available Wi-Fi airwaves...");
  int networkCount = WiFi.scanNetworks();
  scannedNetworksHTML = ""; // Clear old buffer
  if (networkCount == 0) {
    scannedNetworksHTML = "<option value=\"\">No Networks Found</option>";
  } else {
    for (int i = 0; i < networkCount; ++i) {
      scannedNetworksHTML += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
    }
  }

  // Bind server portal endpoints
  server.on("/", HTTP_GET, handlePortalRootRequest);
  server.on("/save", HTTP_POST, handlePortalSaveRequest);
  
  // CRITICAL CAPTIVE PORTAL LINE:
  // If a phone asks for random apple/android test files, redirect them straight to our configuration UI
  server.onNotFound(handlePortalRootRequest);
  
  server.begin();
}

// --- Web Server Request Handlers ---

void handlePortalRootRequest() {
  String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<title>LED Controller Setup</title>";
  html += "<style>body{font-family:sans-serif;background:#1a1a2e;color:#fff;text-align:center;padding:30px 10px;}";
  html += ".box{max-width:360px;margin:0 auto;background:#162447;padding:25px;border-radius:12px;box-shadow:0 6px 15px rgba(0,0,0,0.4);}";
  html += "h2{margin-bottom:20px;color:#e43f5a;}select,input[type='password']{width:100%;padding:12px;margin:12px 0;box-sizing:border-box;border-radius:6px;border:none;font-size:1em;}";
  html += "input[type='submit']{width:100%;padding:12px;background:#e43f5a;border:none;color:#fff;font-size:1.1em;font-weight:bold;border-radius:6px;cursor:pointer;margin-top:10px;}</style></head><body>";
  html += "<div class=\"box\"><h2>Wi-Fi Provisioning Panel</h2>";
  html += "<form method=\"POST\" action=\"/save\">";
  html += "<label>Select Targeted Local Network:</label>";
  html += "<select name=\"ssid\">" + scannedNetworksHTML + "</select>";
  html += "<label>Network Security Password:</label>";
  html += "<input type=\"password\" name=\"password\" placeholder=\"Enter Wi-Fi Password\">";
  html += "<input type=\"submit\" value=\"Save and Connect Device\">";
  html += "</form></div></body></html>";
  
  server.send(200, "text/html", html);
}

void handlePortalSaveRequest() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String netSSID = server.arg("ssid");
    String netPass = server.arg("password");

    // Commit parameters to permanent Flash Memory
    preferences.putString("ssid", netSSID);
    preferences.putString("password", netPass);

    String html = "<html><body style=\"background:#1a1a2e;color:#fff;font-family:sans-serif;text-align:center;padding-top:50px;\">";
    html += "<h2>Credentials Saved Securely!</h2><p>The hardware is rebooting now to authenticate on your network.</p>";
    html += "<p>Reconnect your device to <b>" + netSSID + "</b> and look at your Serial Monitor for the runtime IP console address.</p></body></html>";
    
    server.send(200, "text/html", html);
    Serial.println("New Wi-Fi credentials written to Flash Storage. Executing System Restart...");
    delay(2000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Bad Request: Missing SSID or Password field parameters.");
  }
}

void handleWebRootRequest() {
//   String htmlPayload = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
//   htmlPayload += "<title>ESP32 LED System Dashboard</title>";
//   htmlPayload += "<style>";
//   htmlPayload += "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#0d1117;color:#c9d1d9;text-align:center;padding:20px 10px;}";
//   htmlPayload += ".container{max-width:440px;margin:0 auto;background:#161b22;padding:25px;border-radius:14px;box-shadow:0 8px 24px rgba(0,0,0,0.6);border:1px solid #30363d;}";
//   htmlPayload += "h2{margin:0 0 25px 0;color:#58a6ff;font-size:1.6em;border-bottom:1px solid #21262d;padding-bottom:15px;}";
//   htmlPayload += ".control-group{margin-bottom:22px;text-align:left;background:#21262d;padding:14px;border-radius:8px;border:1px solid #30363d;}";
//   htmlPayload += "label{display:block;font-size:1.05em;margin-bottom:8px;font-weight:600;color:#f0f6fc;}";
//   htmlPayload += "input[type='color']{-webkit-appearance:none;border:none;width:100%;height:50px;border-radius:6px;cursor:pointer;background:none;}";
//   htmlPayload += "input[type='color']::-webkit-color-swatch-wrapper{padding:0;}";
//   htmlPayload += "input[type='color']::-webkit-color-swatch{border:1px solid #30363d;border-radius:6px;}";
//   htmlPayload += "input[type='range']{width:100%;height:6px;background:#484f58;border-radius:3px;-webkit-appearance:none;outline:none;margin:10px 0;}";
//   htmlPayload += "input[type='range']::-webkit-slider-thumb{-webkit-appearance:none;width:18px;height:18px;background:#58a6ff;border-radius:50%;cursor:pointer;transition:0.1s;}";
//   htmlPayload += "input[type='range']::-webkit-slider-thumb:hover{transform:scale(1.2);background:#79c0ff;}";
//   htmlPayload += ".val-display{font-size:0.9em;color:#8b949e;text-align:right;font-family:monospace;}";
//   htmlPayload += ".val-display span{color:#58a6ff;font-weight:bold;}";
//   htmlPayload += "</style></head><body>";
//   htmlPayload += "<div class=\"container\"><h2>System Control Console</h2>";
  
//   // 1. Pattern ID Selection UI (0 to 36)
//   htmlPayload += "<div class=\"control-group\"><label>Active Animation Preset</label>";
//   htmlPayload += "<input type=\"range\" id=\"pattern\" min=\"0\" max=\"36\" value=\"" + String(currentPatternIndex) + "\" oninput=\"dispatchParameters()\">";
//   htmlPayload += "<div class=\"val-display\">Pattern ID: <span id=\"patternVal\">" + String(currentPatternIndex) + "</span></div></div>";
  
//   // 2. Global Brightness Slider UI (0 to 255)
//   htmlPayload += "<div class=\"control-group\"><label>Global System Brightness</label>";
//   htmlPayload += "<input type=\"range\" id=\"brightness\" min=\"0\" max=\"255\" value=\"" + String(globalBrightness) + "\" oninput=\"dispatchParameters()\">";
//   htmlPayload += "<div class=\"val-display\">Duty Cycle: <span id=\"brightnessVal\">" + String(globalBrightness) + "</span> / 255</div></div>";
  
//   // 3. Truncation Range Slider UI (1 to MAX_LEDS)
//   htmlPayload += "<div class=\"control-group\"><label>Active Strip Boundary Limit</label>";
//   htmlPayload += "<input type=\"range\" id=\"numLeds\" min=\"1\" max=\"" + String(MAX_LEDS) + "\" value=\"" + String(dynamicNumLeds) + "\" oninput=\"dispatchParameters()\">";
//   htmlPayload += "<div class=\"val-display\">Pixel Limit: <span id=\"numLedsVal\">" + String(dynamicNumLeds) + "</span> / " + String(MAX_LEDS) + "</div></div>";
  
//   // 4. Global Speed Slider UI (0 to 255)
//   htmlPayload += "<div class=\"control-group\"><label>Kinetic Pacing Speed</label>";
//   htmlPayload += "<input type=\"range\" id=\"speed\" min=\"0\" max=\"255\" value=\"" + String(globalSpeed) + "\" oninput=\"dispatchParameters()\">";
//   htmlPayload += "<div class=\"val-display\">Speed Register: <span id=\"speedVal\">" + String(globalSpeed) + "</span> / 255</div></div>";
  
//   // 5. Global Density Slider UI (0 to 255)
//   htmlPayload += "<div class=\"control-group\"><label>Spatial Density / Compaction</label>";
//   htmlPayload += "<input type=\"range\" id=\"density\" min=\"0\" max=\"255\" value=\"" + String(globalDensity) + "\" oninput=\"dispatchParameters()\">";
//   htmlPayload += "<div class=\"val-display\">Density Register: <span id=\"densityVal\">" + String(globalDensity) + "</span> / 255</div></div>";
  
//   // 6, 7, 8. Hex Color Picker UI Target Triad
//   char hexBuffer[8];
//   sprintf(hexBuffer, "#%02X%02X%02X", pickerRed, pickerGreen, pickerBlue);
//   htmlPayload += "<div class=\"control-group\"><label>Hardware Color Register (RGB Triad)</label>";
//   htmlPayload += "<input type=\"color\" id=\"picker\" value=\"" + String(hexBuffer) + "\" onchange=\"dispatchParameters()\">";
//   htmlPayload += "<div class=\"val-display\">HEX Target: <span id=\"colorVal\">" + String(hexBuffer) + "</span></div></div>";
  
//   // Client-Side Asynchronous Dispatch Engine Block
//   htmlPayload += "</div><script>function dispatchParameters(){";
//   htmlPayload += "var pStr = document.getElementById('pattern').value;";
//   htmlPayload += "var bStr = document.getElementById('brightness').value;";
//   htmlPayload += "var nStr = document.getElementById('numLeds').value;";
//   htmlPayload += "var sStr = document.getElementById('speed').value;";
//   htmlPayload += "var dStr = document.getElementById('density').value;";
//   htmlPayload += "var colorHex = document.getElementById('picker').value;";
  
//   // Instantly push values to front-end labels for zero-latency interactive feel
//   htmlPayload += "document.getElementById('patternVal').innerText = pStr;";
//   htmlPayload += "document.getElementById('brightnessVal').innerText = bStr;";
//   htmlPayload += "document.getElementById('numLedsVal').innerText = nStr;";
//   htmlPayload += "document.getElementById('speedVal').innerText = sStr;";
//   htmlPayload += "document.getElementById('densityVal').innerText = dStr;";
//   htmlPayload += "document.getElementById('colorVal').innerText = colorHex.toUpperCase();";
  
//   // Convert Hex Triad back to standard C++ independent integers 
//   htmlPayload += "var r = parseInt(colorHex.substring(1,3),16);";
//   htmlPayload += "var g = parseInt(colorHex.substring(3,5),16);";
//   htmlPayload += "var b = parseInt(colorHex.substring(5,7),16);";
  

// // Build non-blocking background fetch pipeline request with unique parameter keys
//   htmlPayload += "var targetUrl = '/update?p=' + pStr + '&br=' + bStr + '&num=' + nStr + '&s=' + sStr + '&d=' + dStr + '&r=' + r + '&g=' + g + '&bl=' + b;";
//   htmlPayload += "fetch(targetUrl);}</script></body></html>";
  
//   server.send(200, "text/html", htmlPayload);
String fallbackMsg = "ESP32 LED Controller Core is Online.\n\n";
  fallbackMsg += "Please open your local 'index.html' dashboard control panel to interact with this device.";
  
  server.send(200, "text/plain", fallbackMsg);
}

void handleWebUpdateRequest() {
  // Allow requests originating from external hosted browser dashboards
  server.sendHeader("Access-Control-Allow-Origin", "*");

  // 1. Parse Pattern ID Selection
  if (server.hasArg("p")) {
    int parsedPattern = server.arg("p").toInt();
    if (parsedPattern >= 0 && parsedPattern <= 36) {
      currentPatternIndex = parsedPattern;
    }
  }
  
  // 2. Parse Global System Brightness
  if (server.hasArg("br")) {
    int parsedBrightness = server.arg("br").toInt();
    if (parsedBrightness >= 0 && parsedBrightness <= 255) {
      globalBrightness = parsedBrightness;
      FastLED.setBrightness(globalBrightness);
    }
  }
  
  // 3. Parse Active Strip Boundary Limit
  if (server.hasArg("num")) {
    int parsedTruncation = server.arg("num").toInt();
    if (parsedTruncation >= 1 && parsedTruncation <= MAX_LEDS) {
      dynamicNumLeds = parsedTruncation;
    }
  }
  
  // 4. Parse Kinetic Pacing Speed
  if (server.hasArg("s")) {
    int parsedSpeed = server.arg("s").toInt();
    if (parsedSpeed >= 0 && parsedSpeed <= 255) {
      globalSpeed = parsedSpeed;
    }
  }
  
  // 5. Parse Spatial Density / Particle Compaction
  if (server.hasArg("d")) {
    int parsedDensity = server.arg("d").toInt();
    if (parsedDensity >= 0 && parsedDensity <= 255) {
      globalDensity = parsedDensity;
    }
  }
  
  // 6, 7, 8. Parse RGB Triad Color Targets
  if (server.hasArg("r")) {
    pickerRed = constrain(server.arg("r").toInt(), 0, 255);
  }
  
  if (server.hasArg("g")) {
    pickerGreen = constrain(server.arg("g").toInt(), 0, 255);
  }
  
  if (server.hasArg("bl")) {
    pickerBlue = constrain(server.arg("bl").toInt(), 0, 255);
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