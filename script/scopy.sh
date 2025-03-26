#!/bin/bash

# Variables
# LOCAL_DIR="/home/romain/Documents/suiviGrimpeur_PDI"
LOCAL_DIR="."
SRC_DIR="src"
INCLUDE_DIR="include"
HTML_DIR="server/html"
PARAM_FILE="data/paramDetection.json"
CAMERAS_FILE="data/cameras.json"
MAKEFILE="makefile"
START_SCRIPT="script/startSuiviGrimpeur.sh"

REMOTE_USER="pi"
REMOTE_HOST="192.168.10.20"  # Remplacez par l'adresse IP de votre Raspberry Pi
REMOTE_DIR="/home/pi/suiviGrimpeur/"

if [[ "$(basename "$(pwd)")" == "suiviGrimpeur_PDI" ]]; then
    echo "Le répertoire courant est suiviGrimpeur_PDI."
else
    echo "Attention : Vous n'êtes pas dans le répertoire suiviGrimpeur_PDI. La copie ne peut pas être effectuée."
    exit 1
fi

# Copier le dossier src
rsync -avz --info=stats0 --update "$LOCAL_DIR/$SRC_DIR/" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/src/"

# Copier le dossier include
rsync -avz --info=stats0 --update "$LOCAL_DIR/$INCLUDE_DIR/" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/include/"


# Copier le dossier html
rsync -avz --info=stats0 --update "$LOCAL_DIR/$HTML_DIR/" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/server/html/"

# Copier le fichier paramDetection.json
rsync -avz --info=stats0 --update "$LOCAL_DIR/$PARAM_FILE" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/data/"
rsync -avz --info=stats0 --update "$LOCAL_DIR/$CAMERAS_FILE" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/data/"

# Copier le Makefile
rsync -avz --info=stats0 --update "$LOCAL_DIR/$MAKEFILE" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/"

# Copier le script de démarrage
rsync -avz --info=stats0 --update "$LOCAL_DIR/$START_SCRIPT" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/"


echo "Les dossiers src, include, html, et les fichiers spécifiques de cgi-bin et paramDetection.json ont été copiés avec succès sur le Raspberry Pi."