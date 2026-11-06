# Répartition de travail

## Développeur mobile

- `mobile/` : Expo Go, composants UI LED, scénarios, affichage lux et alertes.
- Client MQTT WSS, gestion connexion/déconnexion, états confirmés.
- Tests mobiles sur téléphone réel.

## Développeur firmware/cloud

- `firmware/wokwi/` : ESP32, deux LED RGB, capteur simulé, MQTT/TLS.
- `infra/aws/` : compte IoT, politique restreinte, certificats, règles.
- `infra/influxdb/` : historisation de la mesure.

## À faire ensemble

- Validation de `docs/CONTRAT_MQTT.txt` avant l'écriture de code.
- Preuve de concept MQTT AWS/Wokwi/Expo.
- Tests d'intégration et de reconnexion.
- Validation des politiques et des coûts AWS.
- Démonstration des cinq gestes demandés par le professeur.

## Convention Git

- `main` stable et partagée.
- `feature/mobile` et `feature/wokwi-aws` en parallèle.
- PR vers `main` après test et relecture par l'autre membre.
- Éviter les commits de secrets ; même un secret supprimé ensuite reste présent dans l'historique Git.
