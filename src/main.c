/******************************************************************************
 * Name        : main.c
 * Description : Main programme for managing camera operations and inter-process
 *               communication using shared memory and semaphores.
 * Author      : Romain
 * Date        : 16/03/2025
 * Version     : 1.0
 *
 * This programme handles camera initialization, video recording, motion detection,
 * and communication with a CGI interface. It uses shared memory for data exchange
 * and semaphores for synchronisation. Signal handling is implemented for process
 * management and graceful termination.
 *
 * Compilation : make bin/main
 *
 * Usage       : ./main
 *
 * Notes       : 
 * - The programme uses JSON for camera configuration.
 * - Shared memory and semaphores are used for inter-process communication.
 * - Signal handling is implemented for managing child processes and termination.
 * 
 * - Need to be refactored and separated into files.
 *
 ******************************************************************************/


/* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */

#include <main.h>

/* ------------------------------------------------------------------------ */
/*              C O N S T A N T E S     S Y M B O L I Q U E S               */
/* ------------------------------------------------------------------------ */

#define NB_ARGS_ECRITURE_MEMOIRE 3
#define NB_ARGS_ENREGISTREMENT_VIDEO 6

/* ------------------------------------------------------------------------ */
/*              D É F I N I T I O N S   D E   T Y P E S                     */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                 V A R I A B L E S    G L O B A L E S                     */
/* ------------------------------------------------------------------------ */

pid_t pidCapture, pidDetection, pidEcritureMemoire;
sem_t *semReaders, *semWriter, *semMutex, *semNewFrame, *semActiveReaders;
int shmOrdre, shmImage;
void *virtAddrOrdre, *virtAddrImage;

camera_t camera1, camera2;
camera_t *cameraActive = NULL;


/* ------------------------------------------------------------------------ */
/*            D E F I N I T I O N   D E    F O N C T I O N S                */
/* ------------------------------------------------------------------------ */


void initSemaphores() {
    // Créer ou ouvrir les sémaphores
    CHECK_NULL(semReaders = sem_open(SEM_READERS, O_CREAT, 0664, 0), "main: sem_open(semReaders)");  // Compter les lecteurs actifs
    CHECK_NULL(semWriter = sem_open(SEM_WRITER, O_CREAT, 0664, 1), "main: sem_open(semWriter)");   // Contrôler l'accès exclusif de l'écrivain
    CHECK_NULL(semMutex = sem_open(SEM_MUTEX, O_CREAT, 0664, 1), "main: sem_open(semMutex)"); // Contrôler l'accès exclusif à la mémoire partagée
    CHECK_NULL(semNewFrame = sem_open(SEM_NEW_FRAME, O_CREAT, 0664, 0), "main: sem_open(semNewFrame)"); // Signaler l'arrivée d'une nouvelle image
    CHECK_NULL(semActiveReaders = sem_open(SEM_ACTIVE_READERS, O_CREAT, 0664, 0), "main: sem_open(semActiveReaders)"); // Compter les lecteurs actifs
   
}

void initSegmentMemoire(){
    // Initialisation segment memoire pour les ordre du cgi
	CHECK(shmOrdre = shm_open(SHM_ORDRE, O_CREAT | O_RDWR, 0666), "main: shm_open(SHM_ORDRE)");
    CHECK(ftruncate(shmOrdre, SHM_ORDRE_SIZE), "main: ftruncate(shmOrdre)");   
    CHECK_NULL(virtAddrOrdre = mmap(0, SHM_ORDRE_SIZE, PROT_WRITE, MAP_SHARED, shmOrdre, 0), "main: mmap(shmOrdre)");
    DEBUG_PRINT("Segment mémoire partagé pour les ordres du CGI créé\n");

    // Ouvrir la mémoire partagée
    CHECK(shmImage = shm_open(SHM_IMAGE, O_CREAT | O_RDWR, 0666), "main: shm_open(SHM_IMAGE)");
    CHECK(ftruncate(shmImage, SHM_FRAME_SIZE), "main: ftruncate(shmImage)");   
    CHECK_NULL(virtAddrImage = mmap(0, SHM_FRAME_SIZE, PROT_WRITE, MAP_SHARED, shmImage, 0), "main: mmap(virtAddr)");
    memset(virtAddrImage, 0, SHM_FRAME_SIZE);
    DEBUG_PRINT("Segment mémoire partagé pour les images créé\n");

}

void processusEcritureMemoire(){
    DEBUG_PRINT("\t[%d] --> Début du processus fils de Ecriture Memoire [%d]\n", getppid(), getpid());

    char url[256];
    sprintf(url, "%s@%s/%s", ENTETE_HTTP, cameraActive->ip, SCRIPT_VIDEO);
    DEBUG_PRINT("Url de detection : %s\n", url);

    const char * args[NB_ARGS_ECRITURE_MEMOIRE] = {PATH_EXE_ECRITURE_MEMOIRE, url,  NULL};


    DEBUG_PRINT("Affichage des argument d'écriture mémoire:\n");
    for (int i = 0; i<NB_ARGS_ECRITURE_MEMOIRE;i++){
        DEBUG_PRINT("\targs[%d] = %s\n", i, args[i]);
    }

    execvp(args[0], (char * const *) args);
}

void initCamera(){
    // Charger les paramètres depuis le fichier JSON
    json_error_t error;
    json_t *root;
    CHECK_NULL(root = json_load_file(PATH_CAMERAS, 0, &error), "main: initCamera - json_load_file(PATH_CAMERAS)");

    json_t *camera_json;
    // Charger les paramètres de la caméra 1
    CHECK_NULL(camera_json = json_object_get(root, "1"), "main: initCamera - json_object_get(camera1)");
    camera1.id = 1;
    strcpy(camera1.ip, json_string_value(json_object_get(camera_json, "IP")));
    camera1.orientation = json_integer_value(json_object_get(camera_json, "orientation"));
    setParamCamera(&camera1);
    

    // Charger les paramètres de la caméra 2
    camera2.id = 2;
    CHECK_NULL(camera_json = json_object_get(root, "2"), "main: initCamera - json_object_get(camera2)");
    strcpy(camera2.ip, json_string_value(json_object_get(camera_json, "IP")));
    camera2.orientation = json_integer_value(json_object_get(camera_json, "orientation"));
    setParamCamera(&camera2);
    

    // Définir la caméra active
    setActiveCamera(&camera1);
    showRoute("0", cameraActive);
}

void setParamCamera(camera_t *camera){
    switch (camera->orientation)
    {
    case 0:
        camera->height = 720;
        camera->width = 1280;
        camera->up = 1;
        camera->down = -1;
        camera->left = -1;
        camera->right = 1;
        strcpy(camera->cmdVertical, "rtilt");
        strcpy(camera->cmdHorizontal, "rpan");
        break;
    case 90:
        camera->height = 1280;
        camera->width = 720;
        camera->up = 1; // a bien definir
        camera->down = 1;
        camera->left = -1;
        camera->right = 1;
        strcpy(camera->cmdVertical, "rpan");
        strcpy(camera->cmdHorizontal, "rtilt");
        break;
    case 180:
        camera->height = 720;
        camera->width = 1280;
        camera->up = -1;
        camera->down = 1;
        camera->left = -1;
        camera->right = 1;
        strcpy(camera->cmdVertical, "rtilt");
        strcpy(camera->cmdHorizontal, "rpan");
        break;
    case 270:    
        camera->height = 1280;
        camera->width = 720;
        camera->up = -1;
        camera->down = 1;
        camera->left = -1;
        camera->right = 1;
        strcpy(camera->cmdVertical, "rpan");
        strcpy(camera->cmdHorizontal, "rtilt");
        break;
    default:
        perror("main: setParamCamera - Orientation de la camera non reconnue");
        break;
    }
}

int setActiveCamera(camera_t *camera){
    // Définir la caméra active
    cameraActive = camera;

    DEBUG_PRINT("main: setActiveCamera - Camera active : %d\n", cameraActive->id);

    // Ecrire les parametres de la cam active dans le fichier data/cameraActive.json
    json_error_t error;
    json_t *root = json_load_file(PATH_CAMERA_ACTIVE, 0, &error);
    
    if (!root) {
        root = json_object(); // Cree le fichier s'il n'existe pas
        if (!root) {
            perror("main: setActiveCamera - Erreur lors de la creation du fichier json");
            return -1;
        } 
    }

    // Vider le fichier json avant d'ecrire
    json_object_clear(root);
    json_object_set_new(root, "id", json_integer(cameraActive->id));
    json_object_set_new(root, "IP", json_string(cameraActive->ip));
    json_object_set_new(root, "orientation", json_integer(cameraActive->orientation));
    json_object_set_new(root, "height", json_integer(cameraActive->height));
    json_object_set_new(root, "width", json_integer(cameraActive->width));
    json_object_set_new(root, "up", json_integer(cameraActive->up));
    json_object_set_new(root, "down", json_integer(cameraActive->down));
    json_object_set_new(root, "left", json_integer(cameraActive->left));
    json_object_set_new(root, "right", json_integer(cameraActive->right));
    json_object_set_new(root, "cmdVertical", json_string(cameraActive->cmdVertical));
    json_object_set_new(root, "cmdHorizontal", json_string(cameraActive->cmdHorizontal));
    

    if (json_dump_file(root, PATH_CAMERA_ACTIVE, JSON_INDENT(0)) != 0) {
        json_decref(root);
        perror("main: setActiveCamera - Erreur lors de l'écriture dans le fichier json");
        return -1;
    }
    json_decref(root);

    if (pidEcritureMemoire > 0) kill(pidEcritureMemoire, SIGTERM); // Arreter le processus d'ecriture memoire
    
    // Lancer le processus d'ecriture memoire
    pid_t pidFils;
    CHECK(pidFils = fork(), "fork(pidEcritureMemoire)");
    if (pidFils == 0) {
        processusEcritureMemoire();
    }
    else pidEcritureMemoire = pidFils;

    // Mettre la camera sur la position d'origine
    return 0;
}

void processusDetection(){
    DEBUG_PRINT("\t[%d] --> Début du processus fils de Detection [%d]\n", getppid(), getpid());

    const char * args[2] = {PATH_EXE_DETECTION,  NULL};

    DEBUG_PRINT("Affichage des argument de detection:\n");
    for (int i = 0; i<2;i++){
        DEBUG_PRINT("\targs[%d] = %s\n", i, args[i]);
    }
    // system("pwd");
    execvp(args[0], (char * const *) args);

}

static void signalHandler(int numSig)
{ 
    switch(numSig) {
        case SIGINT : // traitement de SIGINT
            printf("\t[%d] --> Interruption du programme en cours...\n", getpid());
            exit(0); // Terminer le programme proprement
			break;
		case SIGCHLD: /* traitement de SIGCHLD */
            {
                pid_t pid;
                int status;

                // Boucle pour récupérer tous les fils terminés, afin d'éviter les processus zombies
                while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
                    // DEBUG_PRINT("\t[%d] --> Arrêt du fils de PID: %d\n", getpid(), pid);
                    DEBUG_PRINT("pidCapture : %d, pidDetection : %d, pidEcritureMemoire : %d, pidEnTraitement : %d\n", pidCapture, pidDetection, pidEcritureMemoire, pid);
                    
                    if (WIFEXITED(status)) {
                        int received = WEXITSTATUS(status);
                        DEBUG_PRINT("\t[%d] --> Arrêt du fils de PID: %d, status : %d\n", getpid(), pid, received);
                        
                        
                        
                        if (pid == pidCapture){
                            pidCapture = -1;
                        }
                        else if (pid == pidDetection){ // Si detection s'arrete, on arrete la capture
                            pidDetection = -1;
                            if (received == 1){
                                DEBUG_PRINT("main: Arret de la capture demande\n"); 
                                if (pidCapture > 0) kill(pidCapture, SIGTERM);
                            }
                        } 
                        else if (pid == pidEcritureMemoire){
                            pidEcritureMemoire = -1;
                        }
                    }

                
            
                    
                }
            }
            break;
        case SIGUSR1:
            DEBUG_PRINT("\t[%d] --> SIGUSR1 recu... \n", getpid());
            gestionOrdres();
            break;  
        default :
            printf (" Signal %d non traité \n", numSig );
            break ;
    }
}

void gestionOrdres(){
    char champs1[5]="";
    char champs2[100] ="";
    int status=-1, pidCgi=-1;
    pid_t pidFils = -1;
    

    // Lecture du segment mémoire partagée
    sscanf((char*) virtAddrOrdre, "%d-%[^=]=%s", &pidCgi, champs1, champs2);
    DEBUG_PRINT("PID du CGI : %d, Champs1 : %s et champs2 : %s\n", pidCgi, champs1, champs2);
    
    // Mouvements de camera 
    if (strcmp(champs1, "move")==0){
        char dir[4];
        float angle;
        sscanf(champs2, "%[^&]&prec=%f", dir, &angle);
        DEBUG_PRINT("Move - dir : %s, angle : %f\n", dir, angle);
    
        status = requetePTZ(dir, angle, cameraActive);
    }

    // Zoom de la camera
    if (strcmp(champs1, "zoom")==0){
        float zoom = atof(champs2);
        zoom = 588 * (zoom - 1); // min de zoom Camera = 1 et max = 9999 et zoomHumain = 0.1 à 18
        
        status = requetePTZ("zoom", zoom, cameraActive);
    }

    
    // Enregistrement seul de la video
    if (strcmp(champs1, "reco")==0){ // For record
        
        char action[4], donnees[255];
        sscanf(champs2, "%[^&]&%s", action, donnees);
        DEBUG_PRINT("main: gestionOrdre - Record - Action : %s, Données : %s\n", action, donnees);

        // Enregistrement de la video
        if (strcmp(action, "on") == 0){
            status = enregistrerVideo(donnees);
        }
        else {
            if (pidCapture > 0) {
                status = kill(pidCapture, SIGTERM); // Arrête le processus de capture
                // pidCapture = -1; // Se met à jour a la reception du signal SIGCHLD
            }
        }
    }

    // Detection seule
    if (strcmp(champs1, "detc")==0){
        if (strcmp(champs2, "on") == 0 ){
            status = 0;

            pidFils = fork();
            if (pidFils < 0) {
                perror("main: gestionOrdre - detc - fork(pidDetection)");
                status =  -1;
            } 
            else if (pidFils == 0) processusDetection();
            else pidDetection = pidFils;
                        
        } else {
            if (pidDetection > 0) {
                status = kill(pidDetection, SIGTERM); // Arrête le processus de detection
                // pidDetection = -1; // Se met à jour a la reception du signal SIGCHLD
            }
        }
    }

    // Enregistrement et detection
    if (strcmp(champs1, "enrg") == 0){
        char action[4], donnees[255];
        sscanf(champs2, "%[^&]&%s", action, donnees);
        DEBUG_PRINT("Record - Action : %s, Données : %s\n", action, donnees);

        if (strcmp(action, "on") == 0){

            // Enregistrement de la video
            status = enregistrerVideo(donnees);

            if (status == 0){
                
                // Lancement de la detection
                pidFils = fork();
                if (pidFils < 0) {
                    perror("main: gestionOrdre - enrg - fork(pidDetection)");
                    status =  -1;
                } 
                else if (pidFils == 0) processusDetection();
                else pidDetection = pidFils;
            }
        }
    }

    
    // Voies (afficahge, ajout, suppression)
    if (strcmp(champs1, "rout")==0){
        char action[4], voie[10];
        sscanf(champs2, "%[^&]&id=%s", action, voie);
        DEBUG_PRINT("Route - Action : %s, Voie : %s\n", action, voie);
        
        

        if (strcmp(action, "add") == 0) status = addRoute(voie, cameraActive);
        else if (strcmp(action, "rem")==0) status = removeRoute(voie);
        else status = showRoute(voie, cameraActive);
    }
    

    // Suppression de video
    if (strcmp(champs1, "supp")==0){
        char filepath[255];
        // sécurité sur le nom du fichier pour éviter la suppression de fichier en dehors du dossier PATH_VIDEOS
        if (strstr(champs2, "..") != NULL || strchr(champs2, '/') != NULL) {
            DEBUG_PRINT("Tentative de suppression d'un fichier en dehors du dossier PATH_VIDEOS : %s\n", champs2);
            status = -1;
        } else {
            sprintf(filepath, "%s/%s", PATH_VIDEOS, champs2);
            
            DEBUG_PRINT("Suppression du fichier : %s\n", filepath);
            status = remove(filepath);
        }
    }
        
    // Changement de camera
    if (strcmp(champs1, "cam")==0){
        int id = atoi(champs2);
        DEBUG_PRINT("Changement de camera - id : %d\n", id);
        if (id == 1) status = setActiveCamera(&camera1);
        else if (id == 2) status = setActiveCamera(&camera2);
        else status = -1;
    }

    DEBUG_PRINT("Status : %d\n", status);
    ack(status, pidCgi); 
}

int enregistrerVideo(const char * donnees){
    pid_t pidFils;
    char outputVideoFile[255] = "./videos/output.mp4";
    char prenom[25], nom[25], voie[10];

    struct statvfs stat;

    // Obtenir les informations sur le système de fichiers
    CHECK(statvfs(PATH_VIDEOS, &stat), "main: enregistrerVideo - statvfs(PATH_VIDEOS)");

    // Calculer l'espace disponible en octets
    unsigned long long free_space = stat.f_bsize * stat.f_bavail;

    // Afficher l'espace disponible
    DEBUG_PRINT("Espace disque disponible dans PATH_VIDEO: %llu octets\n", free_space);

    // Vérifier si l'espace disque est suffisant
    if (free_space < MIN_DISK_SPACE) {
        DEBUG_PRINT("Espace disque insuffisant pour l'enregistrement vidéo\n");
        return -2;
    }


    sscanf(donnees, "nom=%[^&]&prenom=%[^&]&voie=%s", nom, prenom, voie);
    DEBUG_PRINT("main: enregistrerVideo - Nom : %s, Prenom : %s, Voie : %s\n", nom, prenom, voie);
    
    // Création de la date et de l'heure
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char dateTime[15];

    strftime(dateTime, sizeof(dateTime)-1, "%Y%m%d_%H%M", t);

    // Création du nom du fichier de sortie
    sprintf(outputVideoFile, "%s/%s_%s_%s_voie%s.mp4", PATH_VIDEOS, dateTime, nom, prenom, voie);
    DEBUG_PRINT("main: enregistrerVideo - Nom du fichier de sortie : %s\n", outputVideoFile); 
    
    pidFils = fork();
    if (pidFils < 0) {
        perror("main: enregistrerVideo - fork(pidCapture)");
        return -1;
    } else if (pidFils == 0) {
        DEBUG_PRINT("\t[%d] --> Début du processus fils de capture [%d]\n", getppid(), getpid());

        char orientationStr[4], widthStr[5], heightStr[5];
        snprintf(orientationStr, sizeof(orientationStr), "%d", cameraActive->orientation);
        snprintf(widthStr, sizeof(widthStr), "%d", cameraActive->width); // a supprimer
        snprintf(heightStr, sizeof(heightStr), "%d", cameraActive->height);

        
        const char * args[NB_ARGS_ENREGISTREMENT_VIDEO] = {PATH_EXE_ENREGISTREMENT_VIDEO, outputVideoFile, orientationStr, widthStr, heightStr, NULL};
        
        DEBUG_PRINT("Affichage des argument de capture video:\n");
        for (int i = 0; i<NB_ARGS_ENREGISTREMENT_VIDEO;i++) DEBUG_PRINT("\targs[%d] = %s\n", i, args[i]);

        execvp(args[0], (char * const *) args);
        DEBUG_PRINT("main: enregistrerVideo - execvp\n");

    } 
    else pidCapture = pidFils;

    return 0;
}

void ack(int status, pid_t pidCgi){
    // Ecriture du status dans le segment mémoire partagée
    memset(virtAddrOrdre, 0, SHM_ORDRE_SIZE);
    sprintf((char*) virtAddrOrdre, "%d", status);

    kill(pidCgi, SIGUSR1);
}

void bye(){
	if (pidCapture>0){ // Si le fils est encore en exécution
		kill(pidCapture, SIGTERM); // Arret du fils 
		pause(); // Attente de la fin du fils et que le signal SIGCHLD soit levé.
	}
    if (pidDetection>0){ // Si le fils est encore en exécution
		kill(pidDetection, SIGTERM); // Arret du fils 
		pause(); // Attente de la fin du fils et que le signal SIGCHLD soit levé.
	} 
    if (pidEcritureMemoire>0){ // Si le fils est encore en exécution
        DEBUG_PRINT("main: bye - pidEcritureMemoire : %d\n", pidEcritureMemoire);
		kill(pidEcritureMemoire, SIGTERM); // Arret du fils 
		pause(); // Attente de la fin du fils et que le signal SIGCHLD soit levé.
	} 


    munmap(virtAddrOrdre, SHM_ORDRE_SIZE);
    close(shmOrdre);
    shm_unlink(SHM_ORDRE);

    munmap(virtAddrImage, SHM_FRAME_SIZE);
    close(shmImage);
    shm_unlink(SHM_IMAGE);

    sem_close(semReaders);
    sem_unlink(SEM_READERS);
    sem_close(semWriter);
    sem_unlink(SEM_WRITER);
    sem_close(semMutex);
    sem_unlink(SEM_MUTEX);
    sem_close(semNewFrame);
    sem_unlink(SEM_NEW_FRAME);  
    sem_close(semActiveReaders);
    sem_unlink(SEM_ACTIVE_READERS);
    

    DEBUG_PRINT("[%d] --> Fin du programme\n", getpid());
}


int main(int argc, char const *argv[])
{
    DEBUG_PRINT("[%d] Démarrage du programme\n", getpid());
    
    //Installation du gestionnaire de fin d'exécution du programme
	atexit(bye);

    // Ouverture du sémaphore nommé pour la communication avec le CGI
    initSemaphores();
    initSegmentMemoire();

    // Initialisation des cameras
    initCamera();

   

    // Sauvegarde du numero de PID
    FILE *fpid;
    CHECK_NULL(fpid = fopen(PATH_FPID, "w"), "fopen(fpid)"); 
    fprintf(fpid, "%d\n", getpid());                              
    fflush(fpid);                                                   
    fclose(fpid); 
 
    

	// Installation du gestionnaire de signaux pour géré le ctrl c et la fin d'un fils.
    struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask ), " sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGINT, &newAction, NULL), "sigaction (SIGINT)");
    CHECK(sigaction(SIGCHLD, &newAction, NULL), "sigaction (SIGCHLD)");
    CHECK(sigaction(SIGUSR1, &newAction, NULL), "sigaction (SIGUSR1)");
    
    while(1) pause();

    return 0;
}

