# Historique InfluxDB

Objectif V1 : stocker les messages `domotique/v1/esp32-01/capteur/luminosite`.

Champs métier :
- deviceId = esp32-01
- lux (nombre)
- alertActive (booléen)
- receivedAt (timestamp UTC, calculable dans AWS IoT Rules via timestamp())

À valider :
1. Version et hébergement exacts d'InfluxDB.
2. Coût réel de l'hébergement choisi.
3. Compatibilité avec l'action AWS IoT InfluxDB ou besoin d'un adaptateur (p. ex. Lambda).
4. Durée de rétention configurée et preuves de stockage.

Note : l'action AWS annoncée en août 2026 vise des clusters InfluxDB gérés par Amazon Timestream ou auto-hébergés ; ne pas présumer que n'importe quelle offre gratuite InfluxDB Cloud est directement compatible.

Référence :
https://aws.amazon.com/about-aws/whats-new/2026/08/aws-iot-core-influxdb/
