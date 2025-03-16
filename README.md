# Suivi de Grimpeurs

## Description

**Suivi de Grimpeurs** est un projet conçu pour suivre et analyser les performances des grimpeurs à l'aide de caméras connectées et de traitements vidéo. Ce système utilise des caméras PTZ (Pan-Tilt-Zoom) Axis pour capturer les mouvements des grimpeurs, les enregistrer, et fournir des outils d'analyse en temps réel ou différé.

Le projet est développé en C/C++ avec des bibliothèques comme OpenCV pour le traitement vidéo, libcurl pour les requêtes HTTP, et Jansson pour la gestion des fichiers JSON.

---

## Fonctionnalités

- **Détection et suivi vidéo** :
  - Capture vidéo en temps réel via des caméras PTZ.
  - Rotation automatique des images en fonction de l'orientation de la caméra.
  - Enregistrement des vidéos pour une analyse ultérieure.

- **Gestion des positions PTZ** :
  - Récupération et enregistrement des positions des caméras.
  - Déplacement automatique des caméras vers des positions prédéfinies.

- **Interface CGI** :
  - Serveur CGI pour interagir avec les caméras et les vidéos via des scripts.

- **Logs et débogage** :
  - Système de logs pour suivre les actions et les erreurs.
  - Mode débogage activable pour une analyse approfondie.

---

## Prérequis

Avant de commencer, assurez-vous d'avoir les éléments suivants installés sur votre système :

- **Système d'exploitation** : Linux (testé sur Raspberry Pi 4 et Ubuntu LTS 22.04 et 24.04).
- **Compilateurs** :
    - GCC pour le code C.
    - G++ pour le code C++.
- **Bibliothèques** :
    - OpenCV (version 4 ou supérieure).
    - libcurl.
    - Jansson.
    - pthread.
    - librt.
- **Outils supplémentaires** :
    - `make` pour la compilation.
    - `ffmpeg` pour le traitement vidéo.

---

## Installation

Pour les instructions détaillées d'installation, veuillez consulter le fichier [INSTALL](INSTALL.md).

## Utilisation 

Pour exécuter ce programme, vous devez le lancer en tant qu'utilisateur www-data (pour la mémoire partagée et les sémaphores, problème à corriger). Exécutez la commande suivante selon votre cas :

- Pour une utilisation normale :
```sh
sudo -u www-data env LD_LIBRARY_PATH=/usr/local/lib ./bin/main
```

- Pour une utilisation avec détection visuelle (uniquement sur un environnement de bureau) :
```sh
xhost +SI:localuser:www-data
sudo -u www-data env LD_LIBRARY_PATH=/usr/local/lib ./bin/main
```

## Structure du projet
Voici un aperçu de la structure des fichiers et dossiers du projet :

```
suiviGrimpeur_PDI/
├── bin/                    # Fichiers exécutables générés
├── src/                # Code source principal
│   ├── main.c          # Point d'entrée principal
│   ├── detection.cpp   # Détection vidéo
│   ├── positions.c     # Gestion des positions PTZ
│   ├── ptz.c           # Contrôle des caméras PTZ
│   ├── enregistrementVideo.c # Enregistrement vidéo
│   ├── ecritureMemoire.c     # Gestion de la mémoire partagée
│   ├── action.c    # Gestion des actions CGI
│   └── video.cpp   # Gestion des vidéos CGI
│
├── include/            # Fichiers d'en-tête
└── server/
    ├── html/        # Pages html et fonctions json pour l'affichage
    └── 010-suiviGrimpeur.conf # Fichier de config pour le serveur apache 
├── log/                    # Fichiers de logs
├── Makefile                # Fichier de compilation
├── README               # Documentation du projet
└── data/                   # Fichiers de configuration et données
    ├── paramDetection.json # Paramètres de détection
    └── positions.json      # Positions PTZ enregistrées
```

## Développement
### Ajout de nouvelles fonctionnalités
1. Ajouter le code source dans ```dev/src/```.
2. Inclure les fichiers d'en-tête nécessaires dans dev/include/.
3. Mettre à jour le Makefile pour inclure les nouvelles cibles.

### Débogage
Activez le mode débogage en ajoutant ```-DDEBUG``` dans les options de compilation ou en ajoutant ```DEBUG=1``` près une commande ```make```

# Contributions

Les contributions sont les bienvenues ! Si vous souhaitez contribuer :
1. Forkez le dépôt.
2. Créez une branche pour vos modifications :
   ```bash
   git checkout -b feature/nom_de_la_fonctionnalite
   ```
3. Soumettez une pull request avec une description claire de vos modifications.

---

## Licence

Ce projet est sous licence MIT. Consultez le fichier [LICENSE](LICENSE.md) pour plus d'informations.