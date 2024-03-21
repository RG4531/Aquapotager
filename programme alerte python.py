from influxdb_client import InfluxDBClient
from datetime import datetime
import time

# Configuration de la connexion à InfluxDB
url = "http://localhost:8086"
bucket = 'Value'
token = "tOQv4Ctp06BEAFTA9pnNOjjyKL8h4yb5nT2Ylam-blD2MNdBtRtqrOOA5bagU_GmUlgwveL-z4lWppynKskDiA=="
org = "Aquapotager"

# Seuil à surveiller
seuil = 1000

# Fonction pour récupérer les données

def fetch_data(client):
    fieldlist=["temp","hygro","lux"]
    for fieldvalue in fieldlist:
        query = f'from(bucket: "Value")\
        |> range(start: -1d)\
        |> filter(fn:(r)=>r._measurement=="ENSAT" and r._field=="{fieldvalue}" and r.loc=="ensat")'
        tables = client.query_api().query(query, org=org)
        for table in tables:
            print(table)
            for record in table.records:
                print(record)
                print(record['_value'])



# Fonction pour alerter si le seuil est dépassé
def check_threshold(value):
    if value!= None:
        if value > seuil :
            return True
        else:
            return False
    else:
        print("erreur")

# Fonction principale
def main():
    client = InfluxDBClient(url=url, token=token)

    while True:
        print("Connect")
        value = fetch_data(client)


        if check_threshold(value) :
            print(f"[{datetime.now()}] Alerte : La valeur {value} dépasse le seuil de {seuil}")
        time.sleep(10)

            #Ici, vous pouvez ajouter un mécanisme d'alerte supplémentaire, comme envoyer un e-mail ou une notification

        #time.sleep(60)  # Attendre 60 secondes avant de vérifier à nouveau

if __name__ == "__main__":
    main()




