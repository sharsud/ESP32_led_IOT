#include <FastLED.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>

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
uint8_t savedBrightness = 100; 
bool systemPowerState = true;   
int dynamicNumLeds = 212;            
uint8_t audioSens = 5;

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

// --- Advanced Audio Infrastructure Registers ---
float dynamicMicHigh = 1500.0;
float smoothedLedCount = 0.0;

// --- Persistent Storage & Network Core Architecture ---
Preferences preferences;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;

// --- Security & API Authentication ---
String globalApiKey = "";

const byte DNS_PORT = 53;
bool isProvisionMode = false;
String scannedNetworksHTML = "";

// FreeRTOS Task Handle for Core 0 Visual Worker Thread
TaskHandle_t FastLEDTaskHandle = NULL;

// --- Core Helper Functions Used by Visual Engine ---
int const_of_d(int val, int low, int high) { 
  if (val < low) return low;
  if (val > high) return high;
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

// --- Forward Declarations ---
void blendFrames();
bool attemptWiFiConnection();
void launchCaptivePortal();
void sendSystemStatus(AsyncWebSocketClient *client = nullptr);
void scanNetworks();
void runSelectedPattern(uint8_t patternId);
void fastLEDCoreTask(void *pvParameters);

// --- WebSocket Event Processor (Delta Aware) ---
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("[WS] Client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    sendSystemStatus(client); 
  } 
  else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("[WS] Client #%u disconnected.\n", client->id());
  } 
  else if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      String jsonStr = String((char*)data).substring(0, len);
      
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, jsonStr);
      if (error) {
        Serial.printf("[WS] JSON Parsing Error: %s\n", error.c_str());
        return;
      }

      // Processes incoming fields atomically based on changes (Deltas)
      if (doc.containsKey("power")) {
        systemPowerState = doc["power"].as<bool>();
        if (!systemPowerState) {
          savedBrightness = globalBrightness;
          globalBrightness = 0;
        } else {
          globalBrightness = (savedBrightness == 0) ? 100 : savedBrightness;
        }
        FastLED.setBrightness(globalBrightness);
      }
      if (doc.containsKey("p")) {
        currentPatternIndex = doc["p"].as<uint8_t>();
      }
      if (doc.containsKey("br")) {
        globalBrightness = doc["br"].as<uint8_t>();
        if (globalBrightness > 0) systemPowerState = true;
        FastLED.setBrightness(globalBrightness);
      }
      if (doc.containsKey("num")) {
        dynamicNumLeds = doc["num"].as<int>();
      }
      if (doc.containsKey("s")) {
        globalSpeed = doc["s"].as<uint8_t>();
      }
      if (doc.containsKey("d")) {
        globalDensity = doc["d"].as<uint8_t>();
      }
      if (doc.containsKey("sens")) {
        audioSens = doc["sens"].as<uint8_t>();
      }
      if (doc.containsKey("xfade")) {
        crossfadeSpeed = doc["xfade"].as<uint8_t>();
      }
      if (doc.containsKey("r"))  pickerRed = doc["r"].as<uint8_t>();
      if (doc.containsKey("g"))  pickerGreen = doc["g"].as<uint8_t>();
      if (doc.containsKey("bl")) pickerBlue = doc["bl"].as<uint8_t>();

      sendSystemStatus(); // Update and synchronize all alternate interfaces
    }
  }
}

void sendSystemStatus(AsyncWebSocketClient *client) {
  JsonDocument doc;
  doc["status"] = "ONLINE";
  doc["power"] = systemPowerState;
  doc["pattern"] = currentPatternIndex;
  doc["brightness"] = globalBrightness;
  doc["speed"] = globalSpeed;
  doc["density"] = globalDensity;
  doc["numLeds"] = dynamicNumLeds;
  doc["audioSens"] = audioSens;
  doc["xfade"] = crossfadeSpeed;
  doc["r"] = pickerRed;
  doc["g"] = pickerGreen;
  doc["bl"] = pickerBlue;

  String output;
  serializeJson(doc, output);
  
  if (client != nullptr) {
    client->text(output);
  } else {
    ws.textAll(output);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500); 

  Serial.println("==================================================================");
  Serial.println("PsudoGlow Engine Initializing storage mapping layout.");
  Serial.println("==================================================================");

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, MAX_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 100000); 
  FastLED.setBrightness(globalBrightness);
  FastLED.clear(true);

  preferences.begin("wifi-config", false);
  globalApiKey = preferences.getString("apiKey", "");
  
  if (globalApiKey == "") {
    const char hexChars[] = "0123456789ABCDEF";
    for (int i = 0; i < 32; i++) {
      globalApiKey += hexChars[esp_random() % 16];
    }
    preferences.putString("apiKey", globalApiKey);
    Serial.println("▶ NEW RETROFITTED API KEY GENERATED: " + globalApiKey);
  } else {
    Serial.println("▶ LOADED SECURITY API KEY: " + globalApiKey);
  }

  if (attemptWiFiConnection()) {
    ws.onEvent(onWsEvent);
    
    ws.setFilter([](AsyncWebServerRequest *request) -> bool {
      if (request->hasParam("apiKey") && request->getParam("apiKey")->value() == globalApiKey) {
        return true; 
      }
      return false; 
    });

    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "PsudoGlow Core Online. Please use the dashboard wrapper application.");
    });

    server.on("/get-key", HTTP_GET, [](AsyncWebServerRequest *request){
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->addHeader("Access-Control-Allow-Origin", "*");
      JsonDocument doc;
      doc["apiKey"] = globalApiKey;
      serializeJson(doc, *response);
      request->send(response);
    });

    server.begin();
    Serial.println("Asynchronous Local Web Server Running on Core 1.");
  } else {
    launchCaptivePortal();
  }

  if (MDNS.begin("psudoglow")) {
    Serial.println("▶ mDNS responder started! Target URL: http://psudoglow.local/");
  }

  // --- Pin Blending and FastLED Execution to Core 0 Task ---
  xTaskCreatePinnedToCore(
    fastLEDCoreTask,        /* Task function */
    "FastLEDTask",          /* Name of task */
    4096,                   /* Stack size in words */
    NULL,                   /* Task input parameter */
    3,                      /* Priority of the task (High) */
    &FastLEDTaskHandle,     /* Task handle to keep track of created task */
    0                       /* Pin task to Core 0 */
  );
  Serial.println("▶ FastLED Engine Isolated and Assigned to Core 0 Thread context.");
}

// Core 1 loop is now kept totally lean to handle networking & clients
void loop() {
  if (isProvisionMode) {
    dnsServer.processNextRequest();
  } else {
    ws.cleanupClients();
  }
  vTaskDelay(pdMS_TO_TICKS(10)); // Give breathing space to Core 1 OS operations
}

// Separate Core 0 runtime engine task loop
void fastLEDCoreTask(void *pvParameters) {
  for(;;) {
    if (isProvisionMode) {
      uint8_t pulse = beatsin8(15, 20, 120);
      fill_solid(targetLeds, dynamicNumLeds, CRGB(0, 0, pulse));
      blendFrames();
      FastLED.show();
    } else {
      if (systemPowerState && globalBrightness > 0) {
        runSelectedPattern(currentPatternIndex);
      } else {
        fill_solid(targetLeds, MAX_LEDS, CRGB::Black);
      }
      
      blendFrames();
      FastLED.show();
      
      uint8_t scaledHueSpeed = map(globalSpeed, 0, 255, 1, 10);
      gHue += scaledHueSpeed;
    }
    
    // Locks processing rate cleanly to ~120 FPS inside Core 0 without blocking network stack
    vTaskDelay(pdMS_TO_TICKS(8)); 
  }
}

// --- Network Connectivity & Provisioning Captive Portal Engines ---
bool attemptWiFiConnection() {
  String savedSSID = preferences.getString("ssid", "");
  String savedPassword = preferences.getString("password", "");

  if (savedSSID == "") {
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

  int connectionAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
    delay(500);
    connectionAttempts++;
    fill_solid(leds, 10, (connectionAttempts % 2 == 0) ? CRGB(255, 100, 0) : CRGB::Black);
    FastLED.show();
  }

  if (WiFi.status() == WL_CONNECTED) {
    fill_solid(leds, dynamicNumLeds, CRGB::Green);
    FastLED.show();
    delay(500);
    return true;
  }
  return false;
}

void scanNetworks() {
  int n = WiFi.scanNetworks();
  scannedNetworksHTML = "";
  for (int i = 0; i < n; ++i) {
    scannedNetworksHTML += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
  }
}

void launchCaptivePortal() {
  isProvisionMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP("PsudoGlow-Setup");
  
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  scanNetworks();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>body{font-family:sans-serif; background:#f0f2f5; padding:20px; text-align:center;} ";
    html += ".card{background:white; padding:30px; border-radius:12px; box-shadow:0 4px 10px rgba(0,0,0,0.1); max-width:400px; margin:auto;} ";
    html += "select, input, button{width:100%; padding:12px; margin:10px 0; border-radius:6px; border:1px solid #ccc; box-sizing:border-box;} ";
    html += "button{background:#6c5ce7; color:white; font-weight:bold; border:none; cursor:pointer;}</style></head><body>";
    html += "<div class='card'><h2>✨ PsudoGlow Provisioning Engine</h2>";
    html += "<form action='/save' method='POST'>";
    html += "<label>Choose Network:</label><select name='ssid'>" + scannedNetworksHTML + "</select>";
    html += "<label>Password:</label><input type='password' name='password' placeholder='••••••••'>";
    html += "<button type='submit'>Save Configuration</button></form></div></body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
    String reqSsid = "";
    String reqPass = "";
    
    if(request->hasParam("ssid", true)) reqSsid = request->getParam("ssid", true)->value();
    if(request->hasParam("password", true)) reqPass = request->getParam("password", true)->value();

    if (reqSsid != "") {
      preferences.putString("ssid", reqSsid);
      preferences.putString("password", reqPass);
      request->send(200, "text/html", "<h3>Configuration Saved. Rebooting to establish connection...</h3>");
      delay(2000);
      ESP.restart();
    } else {
      request->send(400, "text/plain", "Bad Request: SSID missing.");
    }
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    request->redirect("/");
  });

  server.begin();
}

void blendFrames() {
  nblend(leds, targetLeds, MAX_LEDS, crossfadeSpeed);
  for (int i = 0; i < MAX_LEDS; i++) {
    if (!targetLeds[i]) leds[i] = CRGB::Black;
  }
  if (dynamicNumLeds < MAX_LEDS) {
    fill_solid(leds + dynamicNumLeds, MAX_LEDS - dynamicNumLeds, CRGB::Black);
    fill_solid(targetLeds + dynamicNumLeds, MAX_LEDS - dynamicNumLeds, CRGB::Black);
  }
}

void runSelectedPattern(uint8_t patternId) {
  switch (patternId) {
    case 0:  solidColorPicker();          break; 
    case 1:  rainbow();                   break; 
    case 2:  rainbowVortex();             break; 
    case 3:  rainbowWithGlitter();        break; 
    case 4:  confetti();                  break; 
    case 5:  sinelon();                   break; 
    case 6:  bpm();                       break; 
    case 7:  juggle();                    break; 
    case 8:  Fire2012();                  break; 
    case 9:  pacmanGhostChase();          break; 
    case 10: oceanWaves();                break; 
    case 11: plasmaNoise();               break; 
    case 12: strobeParty();               break; 
    case 13: meteorRain();                break; 
    case 14: cyberpunkNeon();             break; 
    case 15: auroraBorealis();            break; 
    case 16: customPulseBreathing();      break; 
    case 17: customColorScanner();        break; 
    case 18: warmFireflyShimmer();        break; 
    case 19: cozyFireplaceCrackle();      break; 
    case 20: movingPastelWave();          break; 
    case 21: organicNebulaDrift();        break; 
    case 22: loungeWineBreathing();       break; 
    case 23: kineticSandGlass();          break; 
    case 24: theatreMarquee();            break; 
    case 25: cosmicDustSupernova();       break; 
    case 26: glacialIceMelt();            break; 
    case 27: matrixDigitalRain();         break; 
    case 28: interstellarWarpDrive();     break; 
    case 29: diamondPrismShimmer();       break; 
    case 30: magmaChamberFissure();       break; 
    case 31: zenBambooForest();           break; 
    case 32: electricTeslaArc();          break; 
    case 33: tokyoNeonRain();             break; 
    case 34: visualize_music();           break; 
    case 35: visualize_music_solids();    break; 
    case 36: visualize_music_dual_tone(); break; 
    default: rainbow();                   break;
  }
}