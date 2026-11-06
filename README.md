# Domotique MQTT — Projet en binôme

**Stack :** React Native / Expo Go + AWS IoT Core + ESP32 simulé sur Wokwi + InfluxDB.

**Périmètre V1 :** 2 LED RGB indépendantes, luminosité ambiante, alertes de seuil, scénarios d'éclairage et historique des mesures.

## Responsabilités

- `mobile/` : interface Expo Go, commande des LED, scénarios, affichage du capteur et des alertes.
- `firmware/wokwi/` : simulation ESP32, application des commandes, mesures et publication MQTT.
- `infra/aws/` : configuration AWS IoT Core, politiques d'accès, règles de routage.
- `infra/influxdb/` : historique des mesures et validation de la rétention.
- `docs/CONTRAT_MQTT.txt` : **source de vérité commune** pour les topics et formats JSON.
- `examples/mqtt/` : exemples de payloads de référence.

## Première étape à réaliser ensemble

1. Créer un dépôt GitHub **vide et privé** (sans README initial) ; initialiser ce dossier avec Git.
2. Lire ensemble `docs/CONTRAT_MQTT.txt` et confirmer la V1.
3. Tester depuis le client MQTT AWS l'envoi d'une commande et la réception d'un état sur Wokwi.
4. Vérifier très tôt la connexion du mobile via MQTT/WebSocket sécurisé avec identifiants AWS temporaires.
5. Développer ensuite le mobile et le firmware en parallèle.

## Démarrer le dépôt (PowerShell / terminal, à la racine du dossier)

```bash
git init
git add .
git commit -m "chore: initialiser le projet domotique"
git branch -M main
git remote add origin URL_DU_DEPOT_GITHUB
git push -u origin main
```

Le dépôt distant doit être créé au préalable. Remplacez `URL_DU_DEPOT_GITHUB` par son adresse réelle ; ne saisissez pas ce texte tel quel.

### Branches conseillées

- `main` : version commune fonctionnelle.
- `feature/mobile` : développement mobile.
- `feature/wokwi-aws` : ESP32, AWS et InfluxDB.

Fusionnez les changements via des pull requests. Évitez de modifier simultanément le contrat MQTT ; toute modification demande l'accord des deux membres.

## Sécurité

Ne placez **jamais** de certificats privés ESP32, clés AWS permanentes, tokens InfluxDB ou secrets dans Git, y compris dans un dépôt privé. La présence d'un `.gitignore` ne retire pas un secret déjà commité. L'application doit utiliser des identifiants temporaires pour l'accès AWS.

## Statut du dépôt

Ce ZIP fournit **la structure et la documentation**, pas une application Expo initialisée ou un sketch Wokwi déjà fonctionnel. Les projets effectifs seront créés dans `mobile/` et `firmware/wokwi/`.
