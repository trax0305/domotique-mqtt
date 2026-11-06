# Architecture technique — V1

```text
             Téléphone / Expo Go
             React Native
                  | MQTT / WSS + SigV4
                  v
             AWS IoT Core
             /          \
            / MQTT/TLS   \ Règle IoT (télémétrie)
           v              v
    ESP32 Wokwi        InfluxDB
    - LED RGB 1        - Historique lux
    - LED RGB 2        - Horodatage AWS
    - Capteur lumière
```

## Flux

1. L'app publie la commande LED via un topic `/set`.
2. L'ESP32 applique la commande puis publie son état via `/state`.
3. L'ESP32 publie périodiquement `lux` et `alertActive`.
4. AWS IoT Rules est chargé de transférer les mesures vers InfluxDB selon la destination choisie et testée.
5. Les scénarios restent dans l'application ; ils produisent des commandes LED ordinaires.

## Risques techniques à lever rapidement

- Wokwi vers AWS IoT Core : connectivité Wi-Fi simulée, TLS et certificats.
- Expo Go vers AWS : génération d'URL WebSocket signée avec des identifiants temporaires.
- Règles AWS vers InfluxDB : compatibilité entre la destination choisie, la version InfluxDB et les coûts.

Ne pas déployer de services facturés sans avoir vérifié les prix et configuré des alertes de budget.
