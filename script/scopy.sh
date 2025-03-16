#!/bin/bash

# Variables
LOCAL_SRC_DIR="/home/romain/Documents/suiviGrimpeur_PDI/src"
LOCAL_INCLUDE_DIR="/home/romain/Documents/suiviGrimpeur_PDI/include"
LOCAL_CGI_BIN_DIR="/home/romain/Documents/suiviGrimpeur_PDI/server/cgi-bin"
LOCAL_HTML_DIR="/home/romain/Documents/suiviGrimpeur_PDI/server/html"
LOCAL_PARAM_FILE="/home/romain/Documents/suiviGrimpeur_PDI/data/paramDetection.json"
LOCAL_CAMERAS_FILE="/home/romain/Documents/suiviGrimpeur_PDI/data/cameras.json"
LOCAL_MAKEFILE="/home/romain/Documents/suiviGrimpeur_PDI/makefile"
LOCAL_START_SCRIPT="/home/romain/Documents/suiviGrimpeur_PDI/script/startSuiviGrimpeur.sh"

REMOTE_USER="pi"
REMOTE_HOST="192.168.1.14"  # Remplacez par l'adresse IP de votre Raspberry Pi
#REMOTE_HOST="192.168.0.196"
REMOTE_DIR="/home/pi/suiviGrimpeur/"


# Copier le dossier src
rsync -avz --info=stats0 --update "$LOCAL_SRC_DIR/" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/src/"

# Copier le dossier include
rsync -avz --info=stats0 --update "$LOCAL_INCLUDE_DIR/" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/include/"

# Copier les fichiers spécifiques du dossier cgi-bin
rsync -avz --info=stats0 --update "$LOCAL_CGI_BIN_DIR/action.c" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/server/cgi-bin/"
rsync -avz --info=stats0 --update "$LOCAL_CGI_BIN_DIR/video.cpp" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/server/cgi-bin/"
rsync -avz --info=stats0 --update "$LOCAL_CGI_BIN_DIR/utilsCgi.h" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/server/cgi-bin/"

# Copier le dossier html
rsync -avz --info=stats0 --update "$LOCAL_HTML_DIR/" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/server/html/"

# Copier le fichier paramDetection.json
rsync -avz --info=stats0 --update "$LOCAL_PARAM_FILE" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/data/"
rsync -avz --info=stats0 --update "$LOCAL_CAMERAS_FILE" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/data/"

# Copier le Makefile
rsync -avz --info=stats0 --update "$LOCAL_MAKEFILE" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/"

# Copier le script de démarrage
rsync -avz --info=stats0 --update "$LOCAL_START_SCRIPT" "$REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/"


echo "Les dossiers src, include, html, et les fichiers spécifiques de cgi-bin et paramDetection.json ont été copiés avec succès sur le Raspberry Pi."