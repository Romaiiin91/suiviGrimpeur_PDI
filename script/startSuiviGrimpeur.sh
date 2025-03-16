#!/bin/bash

JSON_FILE="/home/pi/suiviGrimpeur/data/cameras.json"

cd /home/pi/suiviGrimpeur

# Attendre que le tty soit log
sleep 2

# clear
clear

# Affichage du titre
echo "Démarrage du programme de suivi de grimpeur"
echo "-------------------------------------------"
echo ""

# Affichage de l'IP de la machine
echo ""
MACHINE_IP=$(hostname -I )
echo "My address IP : $MACHINE_IP"
echo ""

# Lire les IPs depuis le fichier JSON
IP1=$(jq -r '."1".IP' "$JSON_FILE")
IP2=$(jq -r '."2".IP' "$JSON_FILE")

echo "Attente de la réponse des deux caméras :"
echo " - $IP1"
echo " - $IP2"

# Vérifier que les deux ping répondent
while true; do
    ping -c 1 -W 1 "$IP1" &> /dev/null && P1=1 || P1=0
    ping -c 1 -W 1 "$IP2" &> /dev/null && P2=1 || P2=0
    if [[ $P1 -eq 1 && $P2 -eq 1 ]]; then
        echo "Les deux caméras sont accessibles."
        break
    fi
    sleep 1
done

# Attente de 30 secondes avec barre de chargement
echo -n "Attente de 30 secondes avant le démarrage du programme : ["
for i in $(seq 1 3); do
    echo -n "#"
    sleep 1
done
echo "]"

# Lancer le programme en arrière-plan
echo "Démarrage du programme..."
sudo -u www-data env LD_LIBRARY_PATH=/usr/local/lib /home/pi/suiviGrimpeur/bin/mainDEBUG & 

sleep 3



# Lancer top 
echo "Lancement de 'top'..."
script -q -c "top" /dev/null