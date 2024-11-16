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

char *choices[] = {
    "Lancer l'enregistrement", 
    "Stopper l'enregistrement", 
    "Mouvement",
    "Enregistrer cette position pour une voie",
    "Supprimer une voie",
    "Allez a une voie", 
    "Lancer la detection", 
    (char *) NULL,
}; 



/* ------------------------------------------------------------------------ */
/*            P R O T O T Y P E S    D E    F O N C T I O N S               */
/* ------------------------------------------------------------------------ */

void bye();
void processusCapture(const char *outputVideoFile);
void processusDetection();
static void signalHandler(int numSig);
void displayMenu(MENU *myMenu);
void resetMenu(MENU *myMenu);


/* ------------------------------------------------------------------------ */
/*            D E F I N I T I O N   D E    F O N C T I O N S                */
/* ------------------------------------------------------------------------ */

int main(int argc, char const *argv[])
{
    DEBUG_PRINT("[%d] Démarrage du programme\n", getpid());
    //Installation du gestionnaire de fin d'exécution du programme
	atexit(bye);

	// Installation du gestionnaire de signaux pour géré le ctrl c et la fin d'un fils.
    struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask ), " sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGINT, &newAction, NULL), "sigaction (SIGINT)");
    CHECK(sigaction(SIGCHLD, &newAction, NULL), "sigaction (SIGCHLD)");


 
    char outputVideoFile[22]="./videos/test.mp4";
    pid_t pidFils;
    
	
     // Initialisation de ncurses
    ITEM **my_items;
    MENU *myMenu;
    int c;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Création des items du menu
    int n_choices = ARRAY_SIZE(choices) - 1; // -1 pour exclure NULL

    my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *)); // +1 pour le NULL final

    for (int i = 0; i < n_choices; i++) my_items[i] = new_item(choices[i], "");
    my_items[n_choices] = (ITEM *)NULL; // Marquer la fin des éléments

    // Créer le menu
    myMenu = new_menu((ITEM **)my_items);
    mvprintw(LINES - 2, 0, "Appuyez sur 'q' pour quitter");
    post_menu(myMenu);
    refresh();

   while ((c = getch()) != 'q') {
        switch (c) {
            case KEY_DOWN:
                menu_driver(myMenu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(myMenu, REQ_UP_ITEM);
                break;
            case 10: // Touche 'Enter'
                {
                    int itemSelectIndex = item_index(current_item(myMenu));

                    switch (itemSelectIndex)
                    {
                    case 0:
                        CHECK(pidFils = fork(), "fork(pidCapture)");
                        if (pidFils == 0) {
                            processusCapture(outputVideoFile);
                        } else {
                            pidCapture = pidFils;
                        }
                        mvprintw(LINES - 3, 0, "Enregistrement en cours");      
                        break;
                    case 1:
                        if (pidCapture > 0) {
                            kill(pidCapture, SIGTERM); // Arrête le processus de capture
                            pidCapture = -1;
                        }
                        mvprintw(LINES - 3, 0, "                                          ");
                        break;
                    case 2:
                        mvprintw(LINES - 4, 0, "Option de déplacement sélectionnée,(q pour quitter)");
                        mvprintw(LINES - 5, 0, "Flèches pour naviguer et page UP/DOWN pour le zoom");
                        deplacement();
                        mvprintw(LINES - 4, 0, "                                                         ");
                        mvprintw(LINES - 5, 0, "                                                         ");
                        break;
                    case 3:
                        resetMenu(myMenu);
                        enregistrerPosition();
                        displayMenu(myMenu);

                        break;
                    case 4:
                        resetMenu(myMenu);
                        supprimerPositionFile();
                        displayMenu(myMenu);

                        break;
                    case 5:
                        resetMenu(myMenu);
                        choixPosition();
                        displayMenu(myMenu);

                        break;
                    case 6:
                        CHECK(pidFils = fork(), "fork(pidDetection)");
                        if (pidFils == 0) {
                            processusDetection();
                        } else {
                            pidDetection = pidFils;
                        }    
                        break;
                    default:
                        break;
                    }
                }
                break;
        }
        refresh();
    }

    // Nettoyage
    unpost_menu(myMenu);
    free_menu(myMenu);
    for (int i = 0; i < n_choices; ++i) {
        free_item(my_items[i]);
    }
    free(my_items);
    
    


    return 0;
}


void processusCapture(const char *outputVideoFile){
    DEBUG_PRINT("\t[%d] --> Début du processus fils de capture [%d]\n", getppid(), getpid());
    DEBUG_PRINT("\t[%d] --> Début de la capture\n", getpid());
    char url[256];
    sprintf(url, "%s@%s/%s", ENTETE_HTTP, IP, SCRIPT_VIDEO);
    DEBUG_PRINT("%s\n", url);

    // Url du flux video; -y force overwrite
    const char * args[NB_ARGS_VIDEO+1] = {"ffmpeg", "-y", "-loglevel", "0",  "-i", url, "-c", "copy", outputVideoFile, NULL};

#ifdef DEBUG
    DEBUG_PRINT("Affichage des argument de ffmpeg:\n");
    for (int i = 0; i<NB_ARGS_VIDEO;i++){
        DEBUG_PRINT("\targs[%d] = %s\n", i, args[i]);
    }
#endif

    execvp("ffmpeg", (char * const *) args);

}

void processusDetection(){
    DEBUG_PRINT("\t[%d] --> Début du processus fils de Detection [%d]\n", getppid(), getpid());
    DEBUG_PRINT("\t[%d] --> Début de la detection\n", getpid());

    char url[256];
    sprintf(url, "%s@%s/%s", ENTETE_HTTP, IP, SCRIPT_VIDEO);
    DEBUG_PRINT("%s\n", url);

    // Url du flux video; -y force overwrite
    
    //const char * args[3] = {"/home/romain/Documents/PDI/dev/bin/detection", url, NULL};

#ifdef DEBUG
    const char * args[3] = {"/home/romain/Documents/PDI/dev/bin/detectionDEBUG", url, NULL};
    DEBUG_PRINT("Affichage des argument de detection:\n");
    for (int i = 0; i<3;i++){
        DEBUG_PRINT("\targs[%d] = %s\n", i, args[i]);
    }
#else 
    const char * args[3] = {"/home/romain/Documents/PDI/dev/bin/detection", url, NULL};
#endif
 
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
                while ((pid = waitpid(-1, &status, WNOHANG)) > 0) DEBUG_PRINT("\t[%d] --> Arrêt du fils de PID: %d\n", getpid(), pid);


            }
            break;
        default :
            printf (" Signal %d non traité \n", numSig );
            break ;
    }
}


void bye(){

	if (pidCapture>0){ // Si le fils est encore en exécution
		kill(pidCapture, SIGTERM); // Arret du fils 
		pause(); // Attente de la fin du fils et que le signal SIGCHLD soit levé.
	}
    if (pidDetection>0){ // Si le fils est encore en exécution
		kill(pidCapture, SIGTERM); // Arret du fils 
		pause(); // Attente de la fin du fils et que le signal SIGCHLD soit levé.
	}

    // Fin de ncurses
    endwin();
	
}

void resetMenu(MENU *myMenu){
    unpost_menu(myMenu);
    clear();
    refresh();
}

void displayMenu(MENU *myMenu){
    clear();
    post_menu(myMenu);
    mvprintw(LINES - 2, 0, "Appuyez sur 'q' pour quitter");
    refresh();
}
