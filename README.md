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

Temp_Hyg_Lum = Fusion des programmes DHT22 et Veml7700

envoie_serveur = Programme pour envoyer les données sur le LNS

Programme Node-Red
------

Pour Node-Red on a : 
- Un mqqt in permettant la réceptin des données depuis le serveur de Paul Sabatier
- Une fonction formater pour trier les informations pour récupérer seulement les informations de nos capteurs. De plus cette fonction récupère que la partie texte (c'est-à-dire les données) et pas les informations technque du serveur.
- Un json permettant de transformer le texte qui est en json, en texte exploitable par influxdb.
- Une fonction qui permet de formater les données reçues en différentes catégories.
- Une puce Debug pour vérifier le bon fonctionnement du flux.
- un influx db out permettant d'envoyer les dnnées sur influxdb pour la représentatin graphique. Pour cela on créer un bucket permettant de stocker ces données.
  
![image](https://github.com/RG4531/Aquapotager/assets/160113818/d617a77d-3ab2-40ee-a4fb-4a8265b8f04a)


Pour InfluxDb
------

Pour InfluxDB on a :
- Le bucket avec toutes les données

![image](https://github.com/RG4531/Aquapotager/assets/160113818/1ac4d720-f0cb-474e-b3e2-9e025496f5af)

- Une interface graphique représentant les données.

![image](https://github.com/RG4531/Aquapotager/assets/160113818/6db58d45-abaa-4e0b-addb-c1ccba135bf0)


Pour Python
------

- Ecriture d'un programme permettant l'apparition d'une fenêtre tableau de bord sur laquelle s'affiche une alerte au cas ou des valeurs ne soient pas conformes aux seuils.

![image](https://github.com/RG4531/Aquapotager/assets/160113818/b49b6d7f-22bb-43af-93ea-0c0c6d482af3)
