/* ------------------------------------------------------------------------ */
/*                            Suivi grimpeur                                */
/*                        Auteur : CHEVALIER Romain                         */
/*                            Date : 26-10-2024                             */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */

#include <utils.h>
#include <positions.h>
#include <ptz.h>


/* ------------------------------------------------------------------------ */
/*              C O N S T A N T E S     S Y M B O L I Q U E S               */
/* ------------------------------------------------------------------------ */

#define NB_ARGS_VIDEO       10

/* ------------------------------------------------------------------------ */
/*              D É F I N I T I O N S   D E   T Y P E S                     */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                 V A R I A B L E S    G L O B A L E S                     */
/* ------------------------------------------------------------------------ */

pid_t pidCapture, pidDetection;
sem_t *semFichierOrdre;

/* ------------------------------------------------------------------------ */
/*            P R O T O T Y P E S    D E    F O N C T I O N S               */
/* ------------------------------------------------------------------------ */

void bye();
void processusCapture(char * outputVideoFile);
void processusDetection();
static void signalHandler(int numSig);
void gestionOrdres();
void ack(int status, pid_t pidCgi);



/* ------------------------------------------------------------------------ */
/*            D E F I N I T I O N   D E    F O N C T I O N S                */
/* ------------------------------------------------------------------------ */

int main(int argc, char const *argv[])
{
    DEBUG_PRINT("[%d] Démarrage du programme\n", getpid());

    // Ouverture du sémaphore nommé pour la communication avec le CGI

    CHECK_NULL(semFichierOrdre = sem_open("semFichierOrdre", O_CREAT, 0644, 1), "sem_open");

    
    
    // Sauvegarde du numero de PID
    FILE *fpid;
    CHECK_NULL(fpid = fopen(PATH_FPID, "w+"), "fopen(fpid)"); 
    fprintf(fpid, "%d\n", getpid());                              
    fflush(fpid);                                                   
    fclose(fpid); 

    //Installation du gestionnaire de fin d'exécution du programme
	atexit(bye);

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


void processusCapture(char * outputVideoFile){
    DEBUG_PRINT("\t[%d] --> Début du processus fils de capture [%d]\n", getppid(), getpid());
    char url[256];
    sprintf(url, "%s@%s/%s", ENTETE_HTTP, IP, SCRIPT_VIDEO);
    DEBUG_PRINT("Url de capture : %s\n", url);

    // Url du flux video; -y force overwrite
    const char * args[NB_ARGS_VIDEO+1] = {"ffmpeg", "-y", "-loglevel", "0",  "-i", url, "-c", "copy", outputVideoFile, NULL};

    DEBUG_PRINT("Affichage des argument de ffmpeg:\n");
    for (int i = 0; i<NB_ARGS_VIDEO;i++){
        DEBUG_PRINT("\targs[%d] = %s\n", i, args[i]);
    }

    execvp("ffmpeg", (char * const *) args);

    

}

void processusDetection(){
    DEBUG_PRINT("\t[%d] --> Début du processus fils de Detection [%d]\n", getppid(), getpid());

    char url[256];
    sprintf(url, "%s@%s/%s", ENTETE_HTTP, IP, SCRIPT_VIDEO);
    DEBUG_PRINT("Url de detection : %s\n", url);
  

    const char * args[3] = {"/home/romain/Documents/PDI/dev/bin/detectionDEBUG", url, NULL};
    DEBUG_PRINT("Affichage des argument de detection:\n");
    for (int i = 0; i<3;i++){
        DEBUG_PRINT("\targs[%d] = %s\n", i, args[i]);
    }
 
    execvp(args[0], (char * const *) args);

}

static void signalHandler(int numSig)
{ 
    switch(numSig) {
        case SIGINT : // traitement de SIGINT
            printf("\t[%d] --> Arrêt du programme en cours...\n", getpid());
            exit(0); // Terminer le programme proprement
			break;
		case SIGCHLD: /* traitement de SIGCHLD */
            {
                pid_t pid;
                int status;

                // Boucle pour récupérer tous les fils terminés, afin d'éviter les processus zombies
                while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
                    DEBUG_PRINT("\t[%d] --> Arrêt du fils de PID: %d\n", getpid(), pid);
                    if (pid == pidCapture) pidCapture = -1;
                    if (pid == pidDetection) pidDetection = -1;
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
    FILE *fOrder;
    char champs1[5]="";
    char champs2[100] ="";
    int status, pidCgi;


    sem_wait(semFichierOrdre);

    CHECK_NULL(fOrder = fopen(PATH_FILE_ORDRE, "r"), "fopen(fichierOrdre)");
    fscanf(fOrder, "%d-%[^=]=%s", &pidCgi, champs1, champs2);
    fclose(fOrder);

    sem_post(semFichierOrdre);

    DEBUG_PRINT("PID du CGI : %d, Champs1 : %s et champs2 : %s\n", pidCgi, champs1, champs2);

    if (strcmp(champs1, "move")==0){
        requetePTZ(champs1, champs2);
    }

    pid_t pidFils = -1;

    if (strcmp(champs1, "reco")==0){ // For record
        char outputVideoFile[50] = "./videos/test.mp4"; // Recuperer le nom du fichier de la requete


        if (strcmp(champs2, "on") == 0){
            CHECK(pidFils = fork(), "fork(pidCapture)");
            if (pidFils == 0) {
                processusCapture(outputVideoFile);
            }
            else pidCapture = pidFils;
        }
        else {
            if (pidCapture > 0) {
                status = kill(pidCapture, SIGTERM); // Arrête le processus de capture
                pidCapture = -1;
            }
        }
    }

    if (strcmp(champs1, "detc")==0){
        if (strcmp(champs2, "on")){
            CHECK(pidFils = fork(), "fork(pidDetection)");
            if (pidFils == 0) processusDetection();
            else pidDetection = pidFils;
                        
        }
        else {
            if (pidDetection > 0) {
                status = kill(pidDetection, SIGTERM); // Arrête le processus de capture
                pidDetection = -1;
            }
        }
    }

    // si je ne ctrl + s le .json, je ne peux pas voir les changements dans le fichier malgré que le cgi fasse des modifs ca ne se met pas à jour sur le serveur appache

    if (strcmp(champs1, "rout")==0){
        char action[4], voie[10];
        sscanf(champs2, "%[^&]&id=%s", action, voie);
        DEBUG_PRINT("Route - Action : %s, Voie : %s\n", action, voie);
        
        

        if (strcmp(action, "add") == 0) status = addRoute(voie);
        else if (strcmp(action, "rem")==0) status = removeRoute(voie);
        else status = showRoute(voie);
    }

    DEBUG_PRINT("Status : %d\n", status);
    ack(status, pidCgi);
    
}

void ack(int status, pid_t pidCgi){
    FILE *fOrder;

    sem_wait(semFichierOrdre);

    CHECK_NULL(fOrder = fopen(PATH_FILE_ORDRE, "w+"), "fopen(fichierOrdre)");
    fprintf(fOrder, "%d\n", status);
    fflush(fOrder);
    fclose(fOrder);

    sem_post(semFichierOrdre);

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

    sem_close(semFichierOrdre);

    DEBUG_PRINT("[%d] --> Fin du programme\n", getpid());
}

