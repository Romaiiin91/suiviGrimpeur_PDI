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



double recupererValeur(const char *data, const char *key){
    char *deb = strstr(data, key);
    if (deb) return atof(deb+strlen(key)+1); // recuperer ce qu'il y a pres key=
    return 0.0;
}

positionPTZ recupererPosition(char * voie){
    positionPTZ pos = {NULL, 0.0, 0.0, 0.0};

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
    
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    
    
    res = curl_easy_perform(curl_handle); // Do the request
    
    /* check for errors */
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else {
        DEBUG_PRINT("Data recupererPosition : %s\n", chunk.memory);
        pos.pan = recupererValeur(chunk.memory, "pan");
        pos.tilt = recupererValeur(chunk.memory, "tilt");
        pos.zoom = recupererValeur(chunk.memory, "zoom");


        pos.numVoie = voie; // risque de planter avec l'affectation des char *
    }
    
    // Nettoyage
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();

    return pos;


}

void addPositionFile(const positionPTZ pos){
    json_t *root = json_load_file(FILEPATH_POSITIONS, 0, NULL);
    if (!root) {
        root = json_object();
    }

    json_t *position_array = json_array();
    json_array_append_new(position_array, json_real(pos.pan));
    json_array_append_new(position_array, json_real(pos.tilt));
    json_array_append_new(position_array, json_real(pos.zoom));

    json_object_set_new(root, pos.numVoie, position_array);
    json_dump_file(root, FILEPATH_POSITIONS, JSON_INDENT(4));
    json_decref(root);
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



int addRoute(char * voie){
    DEBUG_PRINT("Ajout de la voie n°%s\n", voie);
    positionPTZ pos = recupererPosition(voie);
    if (pos.numVoie == NULL) {
        fprintf(stderr, "Erreur lors de la recuperation de valeurs de PTZ\n");
        return 1;
    }
    addPositionFile(pos);

    return 0;
}
int removeRoute(char * voie){
    DEBUG_PRINT("Suppression de la voie n°%s\n", voie);
    
    // Charger le fichier JSON
    json_error_t error;
    json_t *root = json_load_file(FILEPATH_POSITIONS, 0, &error);
    if (!root) {
        fprintf(stderr, "Erreur lors du chargement : %s\n", error.text);
        return 1;
    }

    // Supprimer une clé

    if (json_object_del(root, voie) != 0) {
        fprintf(stderr, "Erreur : impossible de supprimer la clé '%s'\n", voie);
        return 1;
    } else {
        DEBUG_PRINT("Clé '%s' supprimée avec succès.\n", voie);
    }

    // Sauvegarder le fichier JSON modifié
    if (json_dump_file(root, FILEPATH_POSITIONS, JSON_INDENT(4)) != 0) {
        fprintf(stderr, "Erreur lors de l'écriture dans le fichier\n");
        json_decref(root);
        return 1;
    }

    // Nettoyer
    json_decref(root);
    DEBUG_PRINT("Fichier JSON mis à jour avec succès.\n");
    return 0;

}
int showRoute(char * voie){
    DEBUG_PRINT("Affichage de la voie n°%s\n", voie);
    json_error_t error;
    json_t *root = json_load_file(FILEPATH_POSITIONS, 0, &error);
    if (!root) {
        fprintf(stderr, "Erreur lors du chargement : %s\n", error.text);
        return 1;
    }

    json_t *position_array = json_object_get(root, voie);
    if (!json_is_array(position_array)) {
        fprintf(stderr, "Erreur : la clé '%s' n'est pas un tableau\n", voie);
        json_decref(root);
        return 1;
    }

    positionPTZ pos;
    pos.numVoie = voie;
    pos.pan = json_real_value(json_array_get(position_array, 0));
    pos.tilt = json_real_value(json_array_get(position_array, 1));
    pos.zoom = json_real_value(json_array_get(position_array, 2));

    json_decref(root);

    DEBUG_PRINT("Voie: %s, Pan: %.4f, Tilt: %.4f, Zoom: %.4f\n", pos.numVoie, pos.pan, pos.tilt, pos.zoom);
    allerPosition(pos);
    return 0;
}


