// Étape 2 — 2 LED RGB pilotées par MQTT (CONTRAT_MQTT v1.0, §4.1 et §4.2)
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
const uint32_t PWM_FREQ = 5000;
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
      mqtt.publish(T_STATUS, "{\"online\":true}", true, 0);
      // À chaque (re)connexion : état réel des 2 LED, sans requestId (§4.2)
      for (int i = 0; i < 2; i++) publishState(i, "");
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

  // Mesure factice (le vrai capteur arrive à l'étape 3)
  if (millis() - lastPub > 5000) {
    lastPub = millis();
    String msg = "{\"lux\":" + String(100 + (millis() / 1000) % 50) + ",\"alertActive\":false}";
    bool ok = mqtt.publish(T_LUX, msg, false, 0);
    Serial.println("[TX] " + String(T_LUX) + " -> " + msg + (ok ? "" : "  (ÉCHEC)"));
  }
}
