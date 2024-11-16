/* ------------------------------------------------------------------------ */
/*                       Suivi grimpeur - Positions                         */
/*                        Auteur : CHEVALIER Romain                         */
/*                            Date : 26-10-2024                             */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */

#include <positions.h>

/* ------------------------------------------------------------------------ */
/*                 V A R I A B L E S    G L O B A L E S                     */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*            D E F I N I T I O N   D E    F O N C T I O N S                */
/* ------------------------------------------------------------------------ */

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

void enregistrerPosition(){ //https://curl.se/libcurl/c/getinmemory.html
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);  /* grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    // Url
    char url[256];
    sprintf(url, "%s@%s/%s?query=position", ENTETE_HTTP, IP, SCRIPT_PTZ);
    DEBUG_PRINT("%s\n", url);

    curl_global_init(CURL_GLOBAL_ALL);
    
    
    curl_handle = curl_easy_init();                                             // init the curl session 
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);                 // adresse de la requete
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);  // send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);           // we pass our 'chunk' struct to the callback function */
    
    /* some servers do not like requests that are made without a user-agent
        field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    
    
    res = curl_easy_perform(curl_handle); // Do the request
    
    /* check for errors */
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else {
        positionPTZ pos;
        //pos = (positionPTZ *) malloc(sizeof(positionPTZ));
        pos.pan = recupererValeur(chunk.memory, "pan");
        pos.tilt = recupererValeur(chunk.memory, "tilt");
        pos.zoom = recupererValeur(chunk.memory, "zoom");

    
        printw("Entrer un numéro de voie :");

        // // Utiliser un buffer pour lire l'entrée utilisateur
        char input[5];
        echo();
        getnstr(input, sizeof(input) - 1);  // Lire la saisie utilisateur avec affichage
        noecho();  // Revenir au mode sans affichage automatique

        pos.numVoie = atoi(input); 
        addPositionFile(pos);
        
    }
    
    // Nettoyage
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();

}


double recupererValeur(const char *data, const char *key){
    char *deb = strstr(data, key);
    if (deb) return atof(deb+strlen(key)+1); // recuperer ce qu'il y a pres key=
    return 0.0;
}

void addPositionFile(const positionPTZ pos){
    FILE *fichier;
      CHECK_NULL(fichier = fopen(FILEPATH_POSITIONS, "a"), "fopen()");

    fprintf(fichier, "%02d;%.4f;%.4f;%.4f\n", pos.numVoie, pos.pan, pos.tilt, pos.zoom);
    fclose(fichier);

    char cmd[100];
    sprintf(cmd, "sort %s -o %s", FILEPATH_POSITIONS, FILEPATH_POSITIONS);
    system(cmd);
}

void supprimerPositionFile(){
    FILE *fichier_origine, *fichier_temp;
    int numVoieSuppr;

    printw("Entrer un numéro de voie :");

    // Utiliser un buffer pour lire l'entrée utilisateur
    char input[5];
    echo();
    getnstr(input, sizeof(input) - 1);  // Lire la saisie utilisateur avec affichage
    noecho();  // Revenir au mode sans affichage automatique

    numVoieSuppr = atoi(input);

    CHECK_NULL(fichier_origine = fopen(FILEPATH_POSITIONS, "r"), "fopen");
    CHECK_NULL(fichier_temp = fopen("temp.txt", "w"), "fopen");

    char line[LONGUEUR_LIGNE_FILE];
    char lineCopy[LONGUEUR_LIGNE_FILE];

    while (fgets(line, sizeof(line), fichier_origine) != NULL) {
        
        strcpy(lineCopy, line);

        if (atoi(strtok(lineCopy, ";")) != numVoieSuppr){
            fputs(line, fichier_temp);
        }
    }

    fclose(fichier_origine);
    fclose(fichier_temp);

    // Remplacer l'ancien fichier par le nouveau
    remove(FILEPATH_POSITIONS);
    rename("temp.txt", FILEPATH_POSITIONS);
}

void allerPosition(positionPTZ pos){
    // Url
    char url[256];
    sprintf(url, "%s@%s/%s?pan=%.4f&tilt=%.4f&zoom=%.4f", ENTETE_HTTP, IP, SCRIPT_PTZ, pos.pan, pos.tilt, pos.zoom);
    DEBUG_PRINT("%s\n", url);
    

    CURL *curl;
    CURLcode res;
 
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);  // URL de la requête
        res = curl_easy_perform(curl);  // Effectuer la requête

        // Vérification de la réussite de la requête
        if (res != CURLE_OK) fprintf(stderr, "Erreur de requête : %s\n", curl_easy_strerror(res));

        // Nettoyage
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void choixPosition(){
    positionPTZ *positions;
    int nbPositions=0;
    char line[LONGUEUR_LIGNE_FILE];

    CHECK_NULL(positions = (positionPTZ *) malloc(1), "malloc(positions)"); // Initialisation de la mémoire

    FILE *fichier;
    CHECK_NULL(fichier = fopen(FILEPATH_POSITIONS, "r"), "fopen()");

    while (fgets(line, sizeof(line), fichier) != NULL)
    {
        CHECK_NULL(positions = (positionPTZ *) realloc(positions, sizeof(positionPTZ)*(nbPositions+1)), "realloc(positions)");

        positions[nbPositions].numVoie = atoi(strtok(line, ";"));
        positions[nbPositions].pan = atof(strtok(NULL, ";"));
        positions[nbPositions].tilt = atof(strtok(NULL, ";"));
        positions[nbPositions].zoom = atof(strtok(NULL, ";"));
        nbPositions++;
    }

    fclose(fichier);

#ifdef DEBUG
    DEBUG_PRINT("Affichage des voies disponibles:\n");
    for (int i = 0; i<nbPositions; i++){
        DEBUG_PRINT("\tnumVoie=%d: pan=%.4f, tilt=%.4f, zoom=%.4f\n", positions[i].numVoie, positions[i].pan, positions[i].tilt, positions[i].zoom);
    }
#endif


    char choix[nbPositions][10];
    for (int i =0; i <nbPositions; i++){
        sprintf(choix[i], "Voie %d", positions[i].numVoie);
    }



     // Initialisation de ncurses
    ITEM **my_items;
    MENU *my_menu;
    int c;

    

    my_items = (ITEM **)calloc(nbPositions + 1, sizeof(ITEM *)); // +1 pour le NULL final

    for (int i = 0; i < nbPositions; i++) my_items[i] = new_item(choix[i], "");
    my_items[nbPositions] = (ITEM *)NULL; // Marquer la fin des éléments

    // Créer le menu
    my_menu = new_menu((ITEM **)my_items);
    mvprintw(LINES - 2, 0, "Appuyez sur 'q' pour quitter");
    post_menu(my_menu);
    refresh();

   while ((c = getch()) != 'q') {
        switch (c) {
            case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
            case 10: // Touche 'Enter'
                allerPosition(positions[item_index(current_item(my_menu))]);
                break;
        }
        refresh();


    }

    // Nettoyage
    unpost_menu(my_menu);
    free_menu(my_menu);
    for (int i = 0; i < nbPositions; ++i) free_item(my_items[i]);
    free(my_items);
    


    free(positions);
}




// int main(int argc, char const *argv[])
// {

//     // // Initialisation de ncurses
//     // initscr();
//     // cbreak();
//     // noecho();
//     // keypad(stdscr, TRUE);
//     // //mvprintw(LINES - 2, 0, "Appuyez sur 'q' pour quitter");
//     // refresh();

//     //enregistrerPosition();
//     //supprimerPositionFile();
//     // choixPosition();
    
//     // // Fin de ncurses
//     // endwin();
    

//     return 0;
// }
