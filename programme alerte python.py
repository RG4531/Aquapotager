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
Seuilmin = [0,10, 10 ]
Seuilmax=[25, 50, 100]
list=["temp","hygro","lux"]

# Fonction pour récupérer les données

def recup_donnee (client):
    Valeur=[[],[],[]]
    i=0
    for valeurlist in list:
        query = f'from(bucket: "Value")\
        |> range(start: -2h)\
        |> filter(fn:(r)=>r._measurement=="ENSAT" and r._field=="{valeurlist}" and r.loc=="ensat")'
        tables = client.query_api().query(query, org=org)
        for table in tables:
            valeur_list=[]
            for record in table.records:
                valeur_list.append(record['_value'])
            Valeur[i]=valeur_list
            i+=1
                #print(record)
    print(Valeur)
    return Valeur


# Fonction pour afficher une fenêtre d'alerte
def affichagealerte(valeurlist, value ):
    fenetre = tk.Toplevel()
    fenetre.title("Alerte!")
    alert_label = tk.Label(fenetre, text=f"{valeurlist} : La valeur {value} dépasse le seuil.", font=("Arial", 40), fg="red")
    alert_label.pack(padx=400, pady=300)
    dismiss_button = tk.Button(fenetre, text="Fermer", command=fenetre.destroy)
    dismiss_button.pack(pady=10)


# Fonction pour alerter si le seuil est dépassé
def Seuil(value,seuilmin, seuilmax):
    if value[-1] < seuilmin or value[-1] > seuilmax :
        return True
    else:
        return False

# Fonction principale
def main():
    client = InfluxDBClient(url=url, token=token)
    while True:
        print("Connect")
        liste_valeur = recup_donnee(client)
        for i in range(0,3):
            value=liste_valeur[i]
            seuilmin=Seuilmin[i]
            seuilmax=Seuilmax[i]
            valeurlist=list[i]
            if Seuil(value,seuilmin, seuilmax) :
                root = tk.Tk()
                affichagealerte(valeurlist, value[-1])
                root.withdraw()# Cacher la fenêtre principale
                root.after(10000, main)
                root.mainloop()



if __name__ == "__main__":
    main()