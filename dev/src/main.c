/* ------------------------------------------------------------------------ */
/*                            Suivi grimpeur                                */
/*                        Auteur : CHEVALIER Romain                         */
/*                            Date : 26-10-2024                             */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */

#include <main.h>

/* ------------------------------------------------------------------------ */
/*              C O N S T A N T E S     S Y M B O L I Q U E S               */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*              D É F I N I T I O N S   D E   T Y P E S                     */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                 V A R I A B L E S    G L O B A L E S                     */
/* ------------------------------------------------------------------------ */

pid_t pidCapture, pidDetection, pidEcritureMemoire;
sem_t *semFichierOrdre;
sem_t *semReaders, *semWriter, *semMutex, *semNewFrame, *semActiveReaders;
int shmOrdre;
void* virtAddr;


/* ------------------------------------------------------------------------ */
/*            D E F I N I T I O N   D E    F O N C T I O N S                */
/* ------------------------------------------------------------------------ */


void init_semaphores() {
    // Créer ou ouvrir les sémaphores
    CHECK_NULL(semReaders = sem_open(SEM_READERS, O_CREAT, 0664, 0), "main: sem_open(semReaders)");  // Compter les lecteurs actifs
    CHECK_NULL(semWriter = sem_open(SEM_WRITER, O_CREAT, 0664, 1), "main: sem_open(semWriter)");   // Contrôler l'accès exclusif de l'écrivain
    CHECK_NULL(semFichierOrdre = sem_open("semFichierOrdre", O_CREAT, 0664, 1), "main: sem_open(semFichierOrdre)"); // Contrôler l'accès exclusif au fichier d'ordre
    CHECK_NULL(semMutex = sem_open(SEM_MUTEX, O_CREAT, 0664, 1), "main: sem_open(semMutex)"); // Contrôler l'accès exclusif à la mémoire partagée
    CHECK_NULL(semNewFrame = sem_open(SEM_NEW_FRAME, O_CREAT, 0664, 0), "main: sem_open(semNewFrame)"); // Signaler l'arrivée d'une nouvelle image
    CHECK_NULL(semActiveReaders = sem_open(SEM_ACTIVE_READERS, O_CREAT, 0664, 0), "main: sem_open(semActiveReaders)"); // Compter les lecteurs actifs
   
}

void processusEcritureMemoire(){
    DEBUG_PRINT("\t[%d] --> Début du processus fils de Ecriture Memoire [%d]\n", getppid(), getpid());

    char url[256];
    sprintf(url, "%s@%s/%s", ENTETE_HTTP, IP, SCRIPT_VIDEO);
    // sprintf(url, "%s", "/home/romain/Documents/suiviGrimpeur_PDI/dev/data/videos/20250109_2148_2_2_voie1.mp4");
    DEBUG_PRINT("Url de detection : %s\n", url);
  
#ifdef DEBUG
    const char * args[3] = {"./bin/ecritureMemoireDEBUG", url, NULL};
#else
    const char * args[3] = {"./bin/ecritureMemoire", url, NULL};
#endif
    DEBUG_PRINT("Affichage des argument d'écriture mémoire:\n");
    for (int i = 0; i<3;i++){
        DEBUG_PRINT("\targs[%d] = %s\n", i, args[i]);
    }
    // system("pwd");
    execvp(args[0], (char * const *) args);
}



void processusDetection(){
    DEBUG_PRINT("\t[%d] --> Début du processus fils de Detection [%d]\n", getppid(), getpid());

    
  
#ifdef DEBUG
    const char * args[2] = {"./bin/detectionDEBUG",  NULL};
#else
    const char * args[2] = {"./bin/detection",  NULL};
#endif
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
                        else if (pid == pidDetection && received == 1){ // Si detection s'arrete, on arrete la capture
                            pidDetection = -1;
                            DEBUG_PRINT("main: Arret de la capture demande\n"); 
                            if (pidCapture > 0) kill(pidCapture, SIGTERM);
                        } 
                        else if (pid == pidEcritureMemoire){
                            pidEcritureMemoire = -1;
                        }
                    }

                    // // La capture nee renvoie pas de status
                    // if (pid == pidCapture){
                    //     pidCapture = -1;
                    // }
            
                    
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
    sscanf((char*) virtAddr, "%d-%[^=]=%s", &pidCgi, champs1, champs2);
    DEBUG_PRINT("PID du CGI : %d, Champs1 : %s et champs2 : %s\n", pidCgi, champs1, champs2);
    
    // Mouvements de camera 
    if (strcmp(champs1, "move")==0){
        char dir[4], angle[5], newAngle[6];
        sscanf(champs2, "%[^&]&prec=%s", dir, angle);
        DEBUG_PRINT("Move - dir : %s, angle : %s\n", dir, angle);
        
        if (strcmp(dir, "up")==0) status = requetePTZ("rtilt", angle);
        else if (strcmp(dir, "down")==0) {
            sprintf(newAngle, "-%s", angle);
            status =  requetePTZ("rtilt", newAngle);
        }
        
        else if (strcmp(dir, "right")==0) status = requetePTZ("rpan", angle);
        else if (strcmp(dir, "left")==0){
            sprintf(newAngle, "-%s", angle);
            status = requetePTZ("rpan", newAngle);
        }
    }

    // Zoom de la camera
    if (strcmp(champs1, "zoom")==0){
        float zoom = atof(champs2);
        zoom = 588 * zoom - 587; // min de zoom = 1 et max = 9999 et zoom = 0.1 à 18
        sprintf(champs2, "%.0f", zoom);
        
        status = requetePTZ("zoom", champs2);
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
        
        

        if (strcmp(action, "add") == 0) status = addRoute(voie);
        else if (strcmp(action, "rem")==0) status = removeRoute(voie);
        else status = showRoute(voie);
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

        #ifdef DEBUG
            const char * args[3] = {"./bin/enregistrementVideoDEBUG", outputVideoFile, NULL};
        #else
            const char * args[3] = {"./bin/enregistrementVideo", outputVideoFile, NULL};
        #endif
        
        DEBUG_PRINT("Affichage des argument de capture video:\n");
        for (int i = 0; i<3;i++) DEBUG_PRINT("\targs[%d] = %s\n", i, args[i]);

        execvp(args[0], (char * const *) args);

    } 
    else pidCapture = pidFils;

    return 0;
}

void ack(int status, pid_t pidCgi){
    // Ecriture du status dans le segment mémoire partagée
    memset(virtAddr, 0, SHM_ORDRE_SIZE);
    sprintf((char*) virtAddr, "%d", status);

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
		kill(pidEcritureMemoire, SIGTERM); // Arret du fils 
		pause(); // Attente de la fin du fils et que le signal SIGCHLD soit levé.
	} 


    munmap(virtAddr, SHM_ORDRE_SIZE);
    close(shmOrdre);
    shm_unlink(SHM_ORDRE);

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

    // Ouverture du sémaphore nommé pour la communication avec le CGI
    init_semaphores();

    //Installation du gestionnaire de fin d'exécution du programme
	atexit(bye);

    // Sauvegarde du numero de PID
    FILE *fpid;
    CHECK_NULL(fpid = fopen(PATH_FPID, "w"), "fopen(fpid)"); 
    fprintf(fpid, "%d\n", getpid());                              
    fflush(fpid);                                                   
    fclose(fpid); 
 


    int pidFils;
    CHECK(pidFils = fork(), "fork(pidEcritureMemoire)");
    if (pidFils == 0) {
        processusEcritureMemoire();
    }
    else pidEcritureMemoire = pidFils;



	// Initialisation segment memoire pour les ordre du cgi
    // Utilisation d'un segment mémoire partagé pour l'ordre
	CHECK(shmOrdre = shm_open(SHM_ORDRE, O_CREAT | O_RDWR, 0666), "main: shm_open(SHM_ORDRE)");
    CHECK(ftruncate(shmOrdre, SHM_ORDRE_SIZE), "main: ftruncate(shmOrdre)");   
    CHECK_NULL(virtAddr = mmap(0, SHM_ORDRE_SIZE, PROT_WRITE, MAP_SHARED, shmOrdre, 0), "main: mmap(shmOrdre)");
    
   

    

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

