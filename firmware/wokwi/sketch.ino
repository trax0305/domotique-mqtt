// PoC Wokwi ESP32 -> AWS IoT Core (MQTT/TLS mutuel, port 8883)
// Aligné sur CONTRAT_MQTT v1.0 (préfixe domotique/v1/esp32-01/)
// Bibliothèque Wokwi : "MQTT" (Joel Gaehwiler / 256dpi) dans libraries.txt
//   -> contrairement à PubSubClient, elle sait PUBLIER en QoS 1 avec RETAIN

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include "secrets.h"   // endpoint + certificats (NE PAS committer)

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const char* CLIENT_ID = "esp32-01";           // = nom du Thing AWS
#define PREFIX "domotique/v1/esp32-01/"

const char* T_STATUS = PREFIX "status";
const char* T_LUX    = PREFIX "capteur/luminosite";
const char* T_LED1_SET = PREFIX "led/1/set";

WiFiClientSecure net;
MQTTClient mqtt(1024);
unsigned long lastPub = 0;

void onMessage(String &topic, String &payload) {
  Serial.println("[RX] " + topic + " -> " + payload);
}

void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS, 6);
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(250); Serial.print("."); }
  Serial.println(" OK");
}

void syncTime() {
  // TLS vérifie la validité des certificats : il faut l'heure
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("NTP");
  while (time(nullptr) < 1700000000) { delay(250); Serial.print("."); }
  Serial.println(" OK");
}

void connectMqtt() {
  // Last Will : publié par AWS si l'ESP32 disparaît sans prévenir (contrat §4.7)
  mqtt.setWill(T_STATUS, "{\"online\":false}", true, 1);
  while (!mqtt.connected()) {
    Serial.print("MQTT AWS...");
    if (mqtt.connect(CLIENT_ID)) {
      Serial.println(" connecté");
      mqtt.subscribe(T_LED1_SET, 1);
      mqtt.publish(T_STATUS, "{\"online\":true}", true, 1);
    } else {
      char err[128];
      net.lastError(err, sizeof(err));
      Serial.printf(" échec (lastError=%d, returnCode=%d) TLS: %s\n",
                    mqtt.lastError(), mqtt.returnCode(), err);
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
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
    Serial.printf("Déconnecté (lastError=%d, returnCode=%d)\n",
                  mqtt.lastError(), mqtt.returnCode());
    connectMqtt();
  }

  if (millis() - lastPub > 5000) {
    lastPub = millis();
    // Valeur factice pour le PoC ; le vrai capteur arrive à l'étape suivante
    String msg = "{\"lux\":" + String(100 + (millis() / 1000) % 50) + ",\"alertActive\":false}";
    bool ok = mqtt.publish(T_LUX, msg, false, 0);   // QoS 0, pas de RETAIN (contrat §3)
    Serial.println("[TX] " + String(T_LUX) + " -> " + msg + (ok ? "" : "  (ÉCHEC)"));
  }
}
