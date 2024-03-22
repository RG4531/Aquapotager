from influxdb_client import InfluxDBClient
from datetime import datetime
import time
import tkinter as tk

# Configuration de la connexion à InfluxDB
url = "http://localhost:8086"
bucket = 'Value'
token = "tOQv4Ctp06BEAFTA9pnNOjjyKL8h4yb5nT2Ylam-blD2MNdBtRtqrOOA5bagU_GmUlgwveL-z4lWppynKskDiA=="
org = "Aquapotager"

# Seuils à surveiller
Seuilmin = [0, 30, 100]
Seuilmax = [40, 80, 50000]
list = ["temp", "hygro", "lux"]

# Fonction pour récupérer les données
def recup_donnee(client):
    Valeur = [[], [], []]
    for i, valeurlist in enumerate(list):
        query = f'from(bucket: "Value")\
        |> range(start: -3h)\
        |> filter(fn:(r)=>r._measurement=="ENSAT" and r._field=="{valeurlist}" and r.loc=="ensat")'
        tables = client.query_api().query(query, org=org)
        for table in tables:
            valeur_list = []
            for record in table.records:
                valeur_list.append(record['_value'])
            Valeur[i] = valeur_list
    return Valeur

# Fonction pour alerter si le seuil est dépassé
def Seuil(value, seuilmin, seuilmax):
    return value < seuilmin or value > seuilmax

# Fonction principale pour vérifier les seuils et afficher les alertes
def check_and_alert():
    global client  # Accès à la variable client globale
    liste_valeur = recup_donnee(client)
    alert_messages = []
    for i, valeurlist in enumerate(list):
        value = liste_valeur[i][-1]
        seuilmin = Seuilmin[i]
        seuilmax = Seuilmax[i]
        if Seuil(value, seuilmin, seuilmax):
            alert_messages.append(f"Alerte {valeurlist} !!! : La valeur {value} n'est pas comprise dans l'intervalle viable de [{seuilmin};{seuilmax}].\n",)
    if alert_messages:
        affichagealerte(alert_messages)
    # Planifier l'appel de la fonction après un certain délai
    root.after(10000, check_and_alert)  # Vérifier toutes les 10 secondes

# Fonction pour afficher une fenêtre d'alerte
def affichagealerte(alert_messages):
    error_text.delete('1.0', tk.END)
    for message in alert_messages:
        error_text.insert(tk.END, message,"alert")
        error_text.insert(tk.END, "\n----------------------------------\n")  # Ajouter une séparation entre les messages
    error_text.configure(font=("Arial", 20))
    error_text.see(tk.END)  # Faire défiler vers le bas pour voir le dernier message
    root.lift()

# Fonction principale
if __name__ == "__main__":
    client = InfluxDBClient(url=url, token=token)
    root = tk.Tk()
    root.title("Tableau de bord")

    # Widget Text pour afficher les messages d'alerte
    error_text = tk.Text(root, wrap="word", height=20, width=75)
    error_text.pack(fill="both", expand=True, padx=10, pady=10)
    error_text.tag_config("alert", foreground="red")  # Configurer le tag pour que le texte soit rouge

    # Démarrer la vérification et l'affichage des alertes
    check_and_alert()

    # Démarrer la boucle principale Tkinter
    root.mainloop()