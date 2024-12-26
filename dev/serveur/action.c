#include <action.h>

sem_t *semFichierOrdre;

int main(void) {
	char buffer[200];

	struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask ), " sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGUSR1, &newAction, NULL), "sigaction (SIGUSR1)");

	CHECK_NULL(semFichierOrdre = sem_open("semFichierOrdre", 0), "sem_open");


	printf("Content-type:text/html\n");
	//printf("Location:form.html\n"); // redirection 
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

	
	FILE *fOrder;

	sem_wait(semFichierOrdre);

	CHECK_NULL(fOrder = fopen(PATH_FILE_ORDRE, "w+"), "fopen(fichierOrdre)");
	fprintf(fOrder, "%d-%s\n", getpid(), buffer);
	fflush(fOrder);
	fclose(fOrder);

	sem_post(semFichierOrdre);

	DEBUG_PRINT("Recu : %s\n",buffer);
	DEBUG_PRINT("PID action.c %d\n", getpid());


	FILE *fpid;
	int i = 0; 
	char c;

	CHECK_NULL(fpid = fopen(PATH_FPID, "r"), "fopen(fpid)");
	while((c = fgetc(fpid)) != EOF) buffer[i++]= c; 
	buffer[i] = '\0';                                                             
	fclose(fpid); 

	int pid = atoi(buffer);
	DEBUG_PRINT("PID : %d\n", pid); 

	CHECK(kill(pid, SIGUSR1), "kill(pid, SIGUSR1)");

	pause(); // attendre le signal SIGUSR1
	
	return 0;
	
}

static void signalHandler(int numSig)
{ 
    switch(numSig) {
        case SIGUSR1:
            DEBUG_PRINT("\t[%d] --> SIGUSR1 recu... \n", getpid());
            retourHTTP();
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
		// Ici je veux raffraichir la liste si besoin mais je comprends pas comment faire
        printf("<html><body>Action exécutée avec succès</body></html>\n");
    } else {
        printf("<html><body>Erreur lors de l'exécution de l'action</body></html>\n");
    }

}