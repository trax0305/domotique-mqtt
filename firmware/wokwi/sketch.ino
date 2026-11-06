// Étape 3 — LED RGB + capteur de luminosité + alerte au seuil + configuration
// CONTRAT_MQTT v1.0 : §4.1/4.2 (LED), §4.3 (mesure), §4.4 (alerte), §4.5/4.6 (config)
// ESP32 Wokwi -> AWS IoT Core (MQTT/TLS mutuel, port 8883)
// Bibliothèques (libraries.txt) : MQTT (Joel Gaehwiler), ArduinoJson

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include "secrets.h"   // endpoint + certificats (NE PAS committer)

// ---------- Réseau / MQTT ----------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const char* CLIENT_ID = "esp32-01";           // = nom du Thing AWS
#define PREFIX "domotique/v1/esp32-01/"

const char* T_STATUS = PREFIX "status";
const char* T_LUX    = PREFIX "capteur/luminosite";
const char* T_ALERT  = PREFIX "alertes";
const char* T_CFG_SET   = PREFIX "config/set";
const char* T_CFG_STATE = PREFIX "config/state";
const char* T_LED_SET[2]   = { PREFIX "led/1/set",   PREFIX "led/2/set" };
const char* T_LED_STATE[2] = { PREFIX "led/1/state", PREFIX "led/2/state" };

WiFiClientSecure net;
MQTTClient mqtt(1024);

// ---------- LED RGB ----------
// Cathode commune : COM -> GND. Broches PWM (R, G, B).
const uint8_t LED_PINS[2][3] = {
  { 25, 26, 27 },   // LED 1
  { 18, 19, 21 }    // LED 2
};
const uint32_t PWM_FREQ = 1000;
const uint8_t  PWM_BITS = 8;                  // 0..255

struct LedState {
  bool on = false;
  uint8_t r = 255, g = 255, b = 255;           // couleur mémorisée
  uint8_t brightness = 100;                    // 0..100 %
};
LedState leds[2];

// Publication différée : on ne publie pas depuis le callback MQTT
bool   pendingState[2] = { false, false };
String pendingReqId[2];

unsigned long lastPub = 0;

// ---------- Capteur de luminosité (photorésistance Wokwi) ----------
const uint8_t LDR_PIN = 34;                    // AO du module -> GPIO 34 (ADC1)
const float   LDR_VCC = 3.3;                   // module alimenté en 3V3
const float   GAMMA = 0.7, RL10 = 50;          // constantes de la doc Wokwi
const unsigned long SAMPLE_MS  = 1000;         // lecture + contrôle du seuil
const unsigned long PUBLISH_MS = 1000;         // publication de la mesure (§4.3)
const float   HYSTERESIS_LUX = 10;             // sortie d'alerte à seuil + 10 (§4.4)

float alertThresholdLux = 100;                 // valeur initiale (§4.5)
bool  alertActive = false;
float lastLux = 0;
unsigned long lastSample = 0;

bool pendingCfgState = false;
bool pendingAlert = false;

// ---------- PWM (compatible cœur ESP32 Arduino 2.x et 3.x) ----------
void pwmInit() {
  for (int i = 0; i < 2; i++)
    for (int c = 0; c < 3; c++) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
      ledcAttach(LED_PINS[i][c], PWM_FREQ, PWM_BITS);
#else
      ledcSetup(i * 3 + c, PWM_FREQ, PWM_BITS);
      ledcAttachPin(LED_PINS[i][c], i * 3 + c);
#endif
    }
}

void pwmWrite(int led, int c, uint32_t duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(LED_PINS[led][c], duty);
#else
  ledcWrite(led * 3 + c, duty);
#endif
}

void applyLed(int i) {
  const LedState& s = leds[i];
  uint32_t k = s.on ? s.brightness : 0;        // 0..100
  pwmWrite(i, 0, s.r * k / 100);
  pwmWrite(i, 1, s.g * k / 100);
  pwmWrite(i, 2, s.b * k / 100);
}

// ---------- Utilitaires couleur ----------
bool parseHexColor(const char* str, uint8_t& r, uint8_t& g, uint8_t& b) {
  if (!str || strlen(str) != 7 || str[0] != '#') return false;
  for (int i = 1; i < 7; i++) if (!isxdigit((unsigned char)str[i])) return false;
  uint32_t v = strtoul(str + 1, nullptr, 16);
  r = (v >> 16) & 0xFF; g = (v >> 8) & 0xFF; b = v & 0xFF;
  return true;
}

String toHexColor(const LedState& s) {
  char buf[8];
  snprintf(buf, sizeof(buf), "#%02X%02X%02X", s.r, s.g, s.b);
  return String(buf);
}

// ---------- Lecture du capteur ----------
float readLux() {
  int raw = analogRead(LDR_PIN);               // 0..4095
  float v = raw / 4095.0 * LDR_VCC;
  if (v >= LDR_VCC - 0.001) return 0;          // obscurité totale
  if (v <= 0.001) v = 0.001;                   // évite une division par zéro
  float resistance = 10000 * v / (LDR_VCC - v);
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, 1 / GAMMA);
  return round(lux * 10) / 10.0;               // 1 décimale
}

// Alerte avec hystérésis : entrée sous le seuil, sortie à seuil + 10 lux (§4.4)
void checkThreshold(float lux) {
  if (!alertActive && lux < alertThresholdLux) {
    alertActive = true;  pendingAlert = true;
  } else if (alertActive && lux >= alertThresholdLux + HYSTERESIS_LUX) {
    alertActive = false; pendingAlert = true;
  }
}

void publishAlert() {
  JsonDocument doc;
  doc["type"] = "LOW_LUMINOSITY";
  doc["lux"] = lastLux;
  doc["thresholdLux"] = alertThresholdLux;
  doc["active"] = alertActive;
  String out; serializeJson(doc, out);
  bool ok = mqtt.publish(T_ALERT, out, false, 0);          // pas de RETAIN (§4.4)
  Serial.println("[TX] " + String(T_ALERT) + " -> " + out + (ok ? "" : "  (ÉCHEC)"));
}

void publishMeasure() {
  JsonDocument doc;
  doc["lux"] = lastLux;
  doc["alertActive"] = alertActive;
  String out; serializeJson(doc, out);
  bool ok = mqtt.publish(T_LUX, out, false, 0);            // QoS 0, pas de RETAIN (§3)
  Serial.println("[TX] " + String(T_LUX) + " -> " + out + (ok ? "" : "  (ÉCHEC)"));
}

void publishConfigState() {
  JsonDocument doc;
  doc["alertThresholdLux"] = alertThresholdLux;
  String out; serializeJson(doc, out);
  bool ok = mqtt.publish(T_CFG_STATE, out, true, 0);       // RETAIN (§4.6)
  Serial.println("[TX] " + String(T_CFG_STATE) + " -> " + out + (ok ? "" : "  (ÉCHEC)"));
}

// ---------- Réception d'une configuration (§4.5) ----------
void handleConfigSet(const String& payload) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) { Serial.printf("[ERR] config : JSON invalide (%s)\n", err.c_str()); return; }
  if (!doc["alertThresholdLux"].is<float>() || doc["alertThresholdLux"].as<float>() < 0) {
    Serial.println("[ERR] config : 'alertThresholdLux' doit être un nombre >= 0"); return;
  }
  alertThresholdLux = doc["alertThresholdLux"].as<float>();
  Serial.printf("[CFG] nouveau seuil : %.1f lux\n", alertThresholdLux);
  checkThreshold(lastLux);                     // réévalue l'alerte avec le nouveau seuil
  pendingCfgState = true;                      // confirmation publiée dans loop()
}

// ---------- Publication de l'état réellement appliqué (§4.2) ----------
void publishState(int i, const String& requestId) {
  JsonDocument doc;
  if (requestId.length()) doc["requestId"] = requestId;   // omis au démarrage
  doc["on"] = leds[i].on;
  doc["color"] = toHexColor(leds[i]);
  doc["brightness"] = leds[i].brightness;
  String out;
  serializeJson(doc, out);
  // QoS 0 côté ESP32 : dans Wokwi, l'attente du PUBACK (QoS 1) fait tomber la connexion
  bool ok = mqtt.publish(T_LED_STATE[i], out, true, 0);   // RETAIN, QoS 0
  Serial.println("[TX] " + String(T_LED_STATE[i]) + " -> " + out + (ok ? "" : "  (ÉCHEC)"));
}

// ---------- Réception d'une commande /set (§4.1) ----------
void handleLedSet(int i, const String& payload) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) { Serial.printf("[ERR] LED %d : JSON invalide (%s)\n", i + 1, err.c_str()); return; }

  // Tous les champs sont requis
  if (!doc["requestId"].is<const char*>() || strlen(doc["requestId"] | "") == 0) {
    Serial.printf("[ERR] LED %d : requestId manquant ou vide\n", i + 1); return;
  }
  if (!doc["on"].is<bool>()) {
    Serial.printf("[ERR] LED %d : 'on' doit être un booléen\n", i + 1); return;
  }
  uint8_t r, g, b;
  if (!doc["color"].is<const char*>() || !parseHexColor(doc["color"], r, g, b)) {
    Serial.printf("[ERR] LED %d : 'color' doit être #RRGGBB\n", i + 1); return;
  }
  if (!doc["brightness"].is<int>() || doc["brightness"].as<int>() < 0 || doc["brightness"].as<int>() > 100) {
    Serial.printf("[ERR] LED %d : 'brightness' doit être un entier 0..100\n", i + 1); return;
  }

  // Commande valide : on applique (idempotent : même commande => même état)
  leds[i].on = doc["on"].as<bool>();
  leds[i].r = r; leds[i].g = g; leds[i].b = b;
  leds[i].brightness = doc["brightness"].as<int>();
  applyLed(i);

  pendingReqId[i] = doc["requestId"].as<String>();
  pendingState[i] = true;                       // confirmation publiée dans loop()
}

void onMessage(String &topic, String &payload) {
  Serial.println("[RX] " + topic + " -> " + payload);
  for (int i = 0; i < 2; i++)
    if (topic == T_LED_SET[i]) { handleLedSet(i, payload); return; }
  if (topic == T_CFG_SET) { handleConfigSet(payload); return; }
}

// ---------- Connexions ----------
void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS, 6);
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(250); Serial.print("."); }
  Serial.println(" OK");
}

void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("NTP");
  while (time(nullptr) < 1700000000) { delay(250); Serial.print("."); }
  Serial.println(" OK");
}

void connectMqtt() {
  mqtt.setWill(T_STATUS, "{\"online\":false}", true, 1);
  while (!mqtt.connected()) {
    Serial.print("MQTT AWS...");
    if (mqtt.connect(CLIENT_ID)) {
      Serial.println(" connecté");
      for (int i = 0; i < 2; i++) mqtt.subscribe(T_LED_SET[i], 1);
      mqtt.subscribe(T_CFG_SET, 1);
      mqtt.publish(T_STATUS, "{\"online\":true}", true, 0);
      // À chaque (re)connexion : état réel des 2 LED, sans requestId (§4.2)
      for (int i = 0; i < 2; i++) publishState(i, "");
      publishConfigState();                    // seuil courant, retenu (§4.6)
    } else {
      char err[128];
      net.lastError(err, sizeof(err));
      Serial.printf(" échec (lastError=%d, returnCode=%d) TLS: %s\n",
                    mqtt.lastError(), mqtt.returnCode(), err);
      delay(3000);
    }
  }
}

// ---------- Programme ----------
void setup() {
  Serial.begin(115200);
  pwmInit();
  for (int i = 0; i < 2; i++) applyLed(i);     // LED éteintes au démarrage

  connectWifi();
  syncTime();

  net.setCACert(AWS_ROOT_CA);
  net.setCertificate(DEVICE_CERT);
  net.setPrivateKey(DEVICE_KEY);

  mqtt.begin(AWS_ENDPOINT, 8883, net);
  mqtt.onMessage(onMessage);
  mqtt.setOptions(60, true, 10000);   // keepalive 60 s, session propre, timeout 10 s (TLS Wokwi lent)
  connectMqtt();
  lastLux = readLux();
  checkThreshold(lastLux);
}

void loop() {
  mqtt.loop();
  if (!mqtt.connected()) {
    char err[128];
    net.lastError(err, sizeof(err));
    Serial.printf("Déconnecté (lastError=%d, returnCode=%d) TLS: %s | heap libre: %u\n",
                  mqtt.lastError(), mqtt.returnCode(), err, ESP.getFreeHeap());
    connectMqtt();
  }

  // Confirmations des commandes reçues
  for (int i = 0; i < 2; i++)
    if (pendingState[i]) { pendingState[i] = false; publishState(i, pendingReqId[i]); }

  if (pendingCfgState) { pendingCfgState = false; publishConfigState(); }

  // Lecture du capteur et contrôle du seuil chaque seconde
  if (millis() - lastSample >= SAMPLE_MS) {
    lastSample = millis();
    lastLux = readLux();
    checkThreshold(lastLux);
  }
  if (pendingAlert) { pendingAlert = false; publishAlert(); }

  // Publication de la mesure toutes les 5 s (§4.3)
  if (millis() - lastPub >= PUBLISH_MS) {
    lastPub = millis();
    publishMeasure();
  }
}
