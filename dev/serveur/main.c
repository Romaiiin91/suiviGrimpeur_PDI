#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
	char buffer[200];

	printf("Content-type:text/html\n");
	//printf("Location:form.html\n"); // redirection 
	printf("\n"); 

	if (getenv("QUERY_STRING") != NULL)
	{
		strcpy(buffer, getenv("QUERY_STRING"));
		printf("Recu : %s\n",buffer);
	}
	else {
		printf("Aucun caractere reçu\n");
	}
	  
	// traitements à terminer ! 
	
	
	return 0 ;
	
}
