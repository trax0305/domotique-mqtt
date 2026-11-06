# Firmware ESP32 simulé Wokwi (responsable : développeur firmware/cloud)

Créer ici le projet Wokwi : code Arduino/ESP32 et diagramme des 2 LED RGB + capteur lumière.

Étapes :
1. Définir les broches et les états initiaux des 2 LED.
2. Valider le Wi-Fi virtuel Wokwi-GUEST.
3. Connexion MQTT/TLS à AWS IoT Core via certificat client X.509.
4. Souscription exacte aux topics LED /set et config/set.
5. Appliquer les commandes, publier les /state avec retain=true.
6. Publier les mesures `lux` et les alertes.
7. Configurer LWT sur status offline et publier online au démarrage.

**Sécurité** : conserver les clés privées et les certificats hors du dépôt. Ne jamais rendre public un projet Wokwi contenant une clé privée réelle.
