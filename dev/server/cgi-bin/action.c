/******************************************************************************
 * Name        : action.c
 * Description : CGI programme to manage interaction between apache serveur 
 * and main programme.
 * Author      : Romain
 * Date        : [Date]
 * Version     : 1.0
 *
 * This programme receives CGI requests, processes the requested actions and 
 * synchronises access to shared files using semaphores.
 *
 * Compilation : gcc -o action.cgi action.c -lpthread -lrt
 *
 * Usage       : This programme is called by the web server to handle users'
 *               requests.
 *
 * Notes       : 
 * - The programme uses semaphores to synchronise access to files.
 * - The SIGUSR1 and SIGALRM signals are handled for inter-process communication.
 *
 ******************************************************************************/

/* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */

#include <utilsCgi.h>


/* ------------------------------------------------------------------------ */
/*                 V A R I A B L E S    G L O B A L E S                     */
/* ------------------------------------------------------------------------ */

sem_t *semFichierOrdre;


/* ------------------------------------------------------------------------ */
/*            P R O T O T Y P E S    D E    F O N C T I O N S               */
/* ------------------------------------------------------------------------ */

static void signalHandler(int numSig);
void retourHTTP();
void bye();


/* ------------------------------------------------------------------------ */
/*            D E F I N I T I O N   D E    F O N C T I O N S                */
/* ------------------------------------------------------------------------ */

static void signalHandler(int numSig){ 
    switch(numSig) {
        case SIGUSR1:
            DEBUG_CGI_PRINT("\t[%d] --> SIGUSR1 recu... \n", getpid());
            retourHTTP();
            break;  
		case SIGALRM:// A faire 
			DEBUG_CGI_PRINT("\t[%d] --> SIGALRM recu... \n", getpid());
			printf("Content-Type: text/plain\n\n");
            printf("Timeout: Aucun signal SIGUSR1 reçu dans les 5 secondes.\n");
			break;
        default :
            printf (" Signal %d non traité \n", numSig );
            break ;
    }
}
 
void retourHTTP(){
    FILE *fOrder;
	int status;

	sem_wait(semFichierOrdre);
    CHECK_NULL(fOrder = fopen(PATH_FILE_ORDRE, "r"), "fopen(fichierOrdre)");
    fscanf(fOrder, "%d", &status);
    fclose(fOrder);
	sem_post(semFichierOrdre);

    // Envoyer la réponse HTTP
    printf("Content-type: text/html\n\n");
    if (status == 0) {
		// Ici je veux rafraîchir la liste si besoin mais je comprends pas comment faire et le tableau
        printf("<html><body>Action exécutée avec succès</body>\n");
    } else {
        printf("<html><body>Erreur lors de l'exécution de l'action</body></html>\n");
    }

}


void bye(){
    sem_close(semFichierOrdre);

    DEBUG_CGI_PRINT("[%d] --> Fin du programme CGI\n", getpid());
}

/* ------------------------------------------------------------------------ */
/*           				 M A I N   P R O G R A M   		       		    */
/* ------------------------------------------------------------------------ */

int main(void) {
	DEBUG_CGI_PRINT("[%d] Démarrage du programme CGI\n", getpid());
	
	
	//Installation du gestionnaire de fin d'exécution du programme
	atexit(bye);


	char buffer[200];

	struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask ), " sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGUSR1, &newAction, NULL), "sigaction (SIGUSR1)");

	// Ouverture du sémaphore nommé pour synchroniser l'accès au fichier de copie des ordres
	CHECK_NULL(semFichierOrdre = sem_open("semFichierOrdre", 0), "sem_open");


	printf("Content-type:text/html\n");
	// printf("Location:form.html\n"); // redirection 
	printf("\n"); 

	

	if (getenv("QUERY_STRING") != NULL)
	{
		strcpy(buffer, getenv("QUERY_STRING")); 
		
	}
	else {
		printf("Aucun caractere reçu\n");
		return EXIT_FAILURE;
	}
	   
	// traitements à terminer ! 

	DEBUG_CGI_PRINT("Recu : %s\n",buffer);
	
	DEBUG_CGI_PRINT("PID action.c %d\n", getpid());

	int value;
    sem_getvalue(semFichierOrdre, &value);
	DEBUG_CGI_PRINT("Valeur du sémaphore %d\n", value); 
	
	FILE *fOrder;

	sem_wait(semFichierOrdre);
	CHECK_NULL(fOrder = fopen(PATH_FILE_ORDRE, "w"), "fopen(fichierOrdre)");
	fprintf(fOrder, "%d-%s\n", getpid(), buffer);
	fflush(fOrder);
	fclose(fOrder);

	sem_post(semFichierOrdre);


	FILE *fpid;
	int pid;

	DEBUG_CGI_PRINT("Lecture du PID\n");
	
	CHECK_NULL(fpid = fopen(PATH_FPID, "r"), "fopen(fpid)");
	// DEBUG_CGI_PRINT("Fichier ouvert FPDI\n");
	fscanf(fpid, "%d", &pid);                                                   
	fclose(fpid); 

	DEBUG_CGI_PRINT("PID : %d\n", pid); 

	CHECK(kill(pid, SIGUSR1), "kill(pid, SIGUSR1)");

	alarm(5);
	pause(); // attendre le signal SIGUSR1
	
	return 0;
	
}

