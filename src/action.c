/******************************************************************************
 * Name        : action.c
 * Description : CGI programme to manage interaction between apache serveur 
 * and main programme.
 * Author      : Romain
 * Date        : 16/03/2025
 * Version     : 1.0
 *
 * This programme receives CGI requests, processes the requested actions and 
 * synchronises access to shared files using semaphores.
 *
 * Compilation : make server/cgi-bin/action.cgi
 *
 * Usage       : This programme is called by the web server to handle users'
 *               requests.
 *
 * Notes       : 
 * - The programme uses semaphores to synchronise access to files.
 * - The SIGUSR1 signals are handled for to notify the main.
 * - The SIGALRM signals are handled for timeout.
 *
 ******************************************************************************/

/* ------------------------------------------------------------------------ */
/*                   S T A N D A R D S   H E A D E R S                      */
/* ------------------------------------------------------------------------ */

#include <utilsCgi.h>
#define TIMEOUT 1 // Time to wait SIGALARM in seconds, Average time to execute request is less than 100ms according to web browser

/* ------------------------------------------------------------------------ */
/*                 V A R I A B L E S    G L O B A L E S                     */
/* ------------------------------------------------------------------------ */

int shmOrdre;
void* virtAddrOrdre;


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
		case SIGALRM:
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
    int status = -1;

    // Read shared memory segment
    sscanf((char*) virtAddrOrdre, "%d", &status);
    DEBUG_CGI_PRINT("action: Status : %d\n", status);

    // Return appropriate HTTP response (JSON format)
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

	// Closing shared memory segment
    if (virtAddrOrdre != MAP_FAILED) {
        munmap(virtAddrOrdre, SHM_ORDRE_SIZE);
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
	
	// Installing the exit handler
	atexit(bye);

    // Installation of the signal handler
	struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask ), " sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGUSR1, &newAction, NULL), "sigaction (SIGUSR1)");
	CHECK(sigaction(SIGALRM, &newAction, NULL), "sigaction (SIGALRM)");
    
    // Variables
    char buffer[256];

    // Read the CGI request
	printf("Content-type:text/html\n");
	printf("\n"); 

	if (getenv("QUERY_STRING") != NULL) strcpy(buffer, getenv("QUERY_STRING")); 
	else {
		printf("Aucun caractere reçu\n");
		return EXIT_FAILURE;
	}
	   

	DEBUG_CGI_PRINT("Recu : %s\n",buffer);

	
	// Write the order in the shared memory segment
    CHECK(shmOrdre = shm_open(SHM_ORDRE, O_RDWR, 0666), "action: shm_open(SHM_ORDRE)");
    CHECK(ftruncate(shmOrdre, SHM_ORDRE_SIZE), "action: ftruncate(shmOrdre)");   
    CHECK_NULL(virtAddrOrdre = mmap(0, SHM_ORDRE_SIZE, PROT_WRITE, MAP_SHARED, shmOrdre, 0), "action: mmap(virtAddrOrdre)");

	sprintf((char*) virtAddrOrdre, "%d-%s", getpid(), buffer);
	

	// Getting the PID of the main processus
	FILE *fpid;
	int pid;
	
	CHECK_NULL(fpid = fopen(PATH_FPID, "r"), "fopen(fpid)");
	fscanf(fpid, "%d", &pid);                                                   
	fclose(fpid); 

	DEBUG_CGI_PRINT("PID : %d\n", pid); 

    // Send the SIGUSR1 signal to the main process to notify the arrival of a new order
	CHECK(kill(pid, SIGUSR1), "kill(pid, SIGUSR1)");

	alarm(TIMEOUT);
	pause(); // waiting for a signal

    return EXIT_SUCCESS;	
}

