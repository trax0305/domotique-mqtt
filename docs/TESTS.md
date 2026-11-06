# Scénario de validation de la première intégration

- [ ] AWS IoT Core est configuré dans la bonne région avec budget/alertes de coût.
- [ ] Wokwi se connecte à AWS via MQTT/TLS.
- [ ] AWS MQTT Test Client reçoit un `/status` online.
- [ ] Un message `/led/1/set` envoyé par AWS change LED 1 dans Wokwi.
- [ ] `/led/1/state` est bien publié après l'application.
- [ ] LED 2 reste inchangée si seule LED 1 est commandée.
- [ ] Un nouvel abonné exact à `/led/1/state` reçoit le dernier état retenu.
- [ ] Les mesures de luminosité arrivent régulièrement.
- [ ] Le changement de seuil génère et résout une alerte.
- [ ] Expo Go se connecte avec une authentification temporaire et pilote LED 1.
- [ ] Une règle AWS historise les mesures dans la destination Influx choisie.
- [ ] Le scénario « Cinéma » pilote les deux LED.
- [ ] L'application gère un ESP32 hors ligne et évite d'afficher un état retenu comme nécessairement actuel.
- [ ] Les secrets, certificats et tokens sont absents du dépôt.
