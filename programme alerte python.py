from influxdb_client import InfluxDBClient
from datetime import datetime
import time
import tkinter as tk

# Configuration de la connexion à InfluxDB
url = "http://localhost:8086"
bucket = 'Value'
token = "tOQv4Ctp06BEAFTA9pnNOjjyKL8h4yb5nT2Ylam-blD2MNdBtRtqrOOA5bagU_GmUlgwveL-z4lWppynKskDiA=="
org = "Aquapotager"

# Seuil à surveiller
seuil = 20

# Fonction pour récupérer les données

def recup_donnee (client):
    list=["temp","hygro","lux"]
    for valeurlist in list:
        query = f'from(bucket: "Value")\
        |> range(start: -1d)\
        |> filter(fn:(r)=>r._measurement=="ENSAT" and r._field=="{valeurlist}" and r.loc=="ensat")'
        tables = client.query_api().query(query, org=org)
        Valeur=[]
        for table in tables:
            print(table)
            for record in table.records:
                Valeur.append(record['_value'])
                print(record)
                print(Valeur[-1])
        return Valeur


# Fonction pour afficher une fenêtre d'alerte
def affichagealerte(value, threshold):
    fenetre = tk.Toplevel()
    fenetre.title("Alerte!")
    alert_label = tk.Label(fenetre, text=f"La valeur {value} dépasse le seuil de {threshold}.", font=("Arial", 50), fg="red")
    alert_label.pack(padx=400, pady=300)
    dismiss_button = tk.Button(fenetre, text="Fermer", command=fenetre.destroy)
    dismiss_button.pack(pady=10)


# Fonction pour alerter si le seuil est dépassé
def Seuil(value):
    if value[-1] > seuil :
        return True
    else:
        return False

# Fonction principale
def main():
    client = InfluxDBClient(url=url, token=token)
    while True:
        print("Connect")
        value = recup_donnee(client)
        if Seuil(value) :
            print(f"[{datetime.now()}] Alerte : La valeur {value} dépasse le seuil de {seuil}")
            root = tk.Tk()
            affichagealerte(value[-1], seuil)
            root.withdraw()# Cacher la fenêtre principale
            root.after(10000, main)
            root.mainloop()
    time.sleep(300)



if __name__ == "__main__":
    main()

