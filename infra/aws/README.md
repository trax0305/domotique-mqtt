# Infrastructure AWS IoT Core

Configuration prévue : un Thing `esp32-01`, un certificat et une politique IoT restreinte.

- ESP32 : MQTT/TLS sur port 8883 avec certificat X.509.
- Mobile : MQTT/WebSocket sécurisé via SigV4, identité AWS temporaire (p. ex. Cognito Identity Pool), permissions limitées.
- Un identifiant client MQTT unique par connexion.
- Prévoir `iot:Connect`, `iot:Publish`, `iot:Subscribe`, `iot:Receive`, et `iot:RetainPublish` **uniquement pour les topics retenus publiés**.
- Les ressources `topic/` et `topicfilter/` diffèrent dans les politiques AWS.
- Le mobile ne doit publier que sur `/set` et recevoir `/state`, mesures, alertes et status.
- L'ESP32 ne doit publier que ses `/state`, mesures, alertes et status, et recevoir ses `/set`.
- Le broker accepte les QoS 0 et 1, pas QoS 2.
- Configurer les alertes de budget AWS avant déploiement.

Ne pas créer de politique `iot:*` ouverte pour les appareils ou l'application en production.

Références :
https://docs.aws.amazon.com/iot/latest/developerguide/mqtt.html
https://docs.aws.amazon.com/iot/latest/developerguide/iot-policy-actions.html
https://docs.aws.amazon.com/iot/latest/developerguide/pub-sub-policy.html
