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

int shmOrdre;
void* virtAddr;


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
 
// void retourHTTP(){
// 	int status = -1;

// 	// Lecture du status dans le segment mémoire partagé
// 	sscanf((char*) virtAddr, "%d", &status);
// 	DEBUG_CGI_PRINT("action: Status : %d\n", status);

//     // Envoyer la réponse HTTP
//     printf("Content-type: text/html\n\n");
//     if (status == 0) {
//         printf("<html><body>Action exécutée avec succès</body>\n"); 
//     } else if (status == -2){
// 		// Je veux retourner une erreur ici pour la renvoyer a la page html qui devra mettre une popup disant que l'espace disque n'est pas suffisant
// 		printf("<html><body><script>alert('Erreur: Espace disque insuffisant');</script></body></html>\n");
// 	} 
// 	else {
//         printf("<html><body>Erreur lors de l'exécution de l'action</body></html>\n");
//     }

// }

void retourHTTP(){
    int status = -1;

    // Lecture du status dans le segment mémoire partagé
    sscanf((char*) virtAddr, "%d", &status);
    DEBUG_CGI_PRINT("action: Status : %d\n", status);

    // Envoyer la réponse HTTP en JSON
    // printf("Content-type: application/json\n\n");
    if (status == 0) {
        printf("{\"success\": true, \"message\": \"Action exécutée avec succès\"}\n");
    } else if (status == -2) {
        printf("{\"success\": false, \"error\": \"Espace disque insuffisant, veuillez supprimer des vidéos\"}\n");
    } else {
        printf("{\"success\": false, \"error\": \"Une erreur est survenue lors de l’exécution du CGI\"}\n");
    }
}


void bye(){
    DEBUG_CGI_PRINT("[%d] --> Fin du programme CGI action\n", getpid());

	// Fermeture du segment mémoire partagé
    if (virtAddr != MAP_FAILED) {
        munmap(virtAddr, SHM_ORDRE_SIZE);
    }
    if (shmOrdre != -1) {
        close(shmOrdre);
    }
}

/* ------------------------------------------------------------------------ */
/*           				 M A I N   P R O G R A M   		       		    */
/* ------------------------------------------------------------------------ */

int main(void) {
	DEBUG_CGI_PRINT("[%d] Démarrage du programme CGI\n", getpid());
	
	
	//Installation du gestionnaire de fin d'exécution du programme
	atexit(bye);


	char buffer[256];

	struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask ), " sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGUSR1, &newAction, NULL), "sigaction (SIGUSR1)");
	CHECK(sigaction(SIGALRM, &newAction, NULL), "sigaction (SIGALRM)");


	printf("Content-type:text/html\n");
	printf("\n"); 

	

	if (getenv("QUERY_STRING") != NULL) strcpy(buffer, getenv("QUERY_STRING")); 
	else {
		printf("Aucun caractere reçu\n");
		return EXIT_FAILURE;
	}
	   

	DEBUG_CGI_PRINT("Recu : %s\n",buffer);

	
	// Utilisation d'un segment mémoire partagé pour l'ordre
    CHECK(shmOrdre = shm_open(SHM_ORDRE, O_RDWR, 0666), "action: shm_open(SHM_ORDRE)");
    CHECK(ftruncate(shmOrdre, SHM_ORDRE_SIZE), "action: ftruncate(shmOrdre)");   
    CHECK_NULL(virtAddr = mmap(0, SHM_ORDRE_SIZE, PROT_WRITE, MAP_SHARED, shmOrdre, 0), "action: mmap(virtAddr)");

	// Ecriture de l'ordre dans le segment mémoire partagé
	sprintf((char*) virtAddr, "%d-%s", getpid(), buffer);
	
	// Envoi du signal SIGUSR1 au programme principal
	FILE *fpid;
	int pid;

	DEBUG_CGI_PRINT("Lecture du PID\n");
	
	CHECK_NULL(fpid = fopen(PATH_FPID, "r"), "fopen(fpid)");
	fscanf(fpid, "%d", &pid);                                                   
	fclose(fpid); 

	DEBUG_CGI_PRINT("PID : %d\n", pid); 

	CHECK(kill(pid, SIGUSR1), "kill(pid, SIGUSR1)");

	alarm(5);
	pause(); // attendre le signal SIGUSR1 ou SIGALARM


	
	return 0;
	
}

