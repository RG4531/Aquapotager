# Projet Aqua-potager
### Collecte et visualisation de données environnementales par l'utilisation de micro-contrôleurs.

#### Le produit attendu est un ensemble de composantes formant un système complet permettant de collecter et visualiser des données de terrains (potager et aquaponie). Il pourra également dans une perspective d'amélioration, alerter lorsque certaines valeurs seuils seront dépassées (humidité excessive, …).

 ... Fonctions accomplies par le produit : 
 
**Collecter** : par capteurs (température/humidité de l’air, luminosité, température du sol)

**Transférer** : émetteur, gateway, MQTT

**Visualiser** : InfluxDB

Programmes Ardiuno
------

DHT22 = Programme pour le capteur de T° et humidité

Veml7700 = Programme pour le capteur de luminosité

LoRaWAN = Programme pour envoyer les données sur le LNS

Programme Node-Red
------

Pour Node-Red on a : 
- Un mqqt in permettant la réceptin des données depuis le serveur de Paul Sabatier
- Une fonction formater pour trier les informations pour récupérer seulement les informations de nos capteurs. De plus cette fonction récupère que la partie texte (c'est-à-dire les données) et pas les informations technque du serveur.
- Un json permettant de transformer le texte qui est en json, en texte exploitable par influxdb
- Une puce Debug pour vérifier le bon fonctionnement du flux.
- un influx db out permettant d'envoyer les dnnées sur influxdb pour la représentatin graphique

Pour InfluxDb
------

Pour InfluxDB on a :

