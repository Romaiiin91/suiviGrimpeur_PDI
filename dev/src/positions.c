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

extern camera_t camera1, camera2;

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

positionPTZ recupererPosition(char * voie, const camera_t *cameraActive){
    positionPTZ pos = {NULL, 0.0, 0.0, 0.0};

    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);  /* grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    // Url
    char url[256];
    sprintf(url, "%s@%s/%s?query=position", ENTETE_HTTP, cameraActive->ip, SCRIPT_PTZ);
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
        fprintf(stderr, "recupererPosition - curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else {
        DEBUG_PRINT("Data recupererPosition : %s\n", chunk.memory);
        pos.pan = recupererValeur(chunk.memory, "pan");
        pos.tilt = recupererValeur(chunk.memory, "tilt");
        pos.zoom = recupererValeur(chunk.memory, "zoom");

        pos.camera = cameraActive->id;
        pos.numVoie = voie;
    }
    
    // Nettoyage
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();

    return pos;


}

int addPositionFile(const positionPTZ pos){
    // Chargement pas propre du fichier en cas d'erreur

    json_error_t error;
    json_t *root = json_load_file(PATH_POSITIONS, 0, &error);
    
    if (!root) {
        root = json_object(); // Cree le fichier s'il n'existe pas
        if (!root) {
            fprintf(stderr, "addPositionFile - Erreur lors de la creation : %s\n", error.text);
            return -1;
        } 
    }

    json_t *position_array = json_array();
    json_array_append_new(position_array, json_real(pos.pan));
    json_array_append_new(position_array, json_real(pos.tilt));
    json_array_append_new(position_array, json_real(pos.zoom));
    json_array_append_new(position_array, json_integer(pos.camera));

    json_object_set_new(root, pos.numVoie, position_array);

    if (json_dump_file(root, PATH_POSITIONS, JSON_INDENT(4)) != 0) {
        json_decref(root);
        return -1;
    }
    
    json_decref(root);
    return 0;
}



int allerPosition(positionPTZ pos, const camera_t *cameraActive){
    int status = 0;

    // Url
    char url[256];
    sprintf(url, "%s@%s/%s?pan=%.4f&tilt=%.4f&zoom=%.4f", ENTETE_HTTP, cameraActive->ip, SCRIPT_PTZ, pos.pan, pos.tilt, pos.zoom);
    DEBUG_PRINT("%s\n", url);
    

    CURL *curl;
    CURLcode res;
 
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);  // URL de la requête
        res = curl_easy_perform(curl);  // Effectuer la requête

        // Vérification de la réussite de la requête
        if (res != CURLE_OK) {
            fprintf(stderr, "allerPosition - Erreur de requête : %s\n", curl_easy_strerror(res));
            status = -1;
        }

        // Nettoyage
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return status;
}


// Ajout d'une nouvelle voie dans le fichier json
int addRoute(char * voie, const camera_t *cameraActive){
    DEBUG_PRINT("Ajout de la voie n°%s\n", voie);
    positionPTZ pos = recupererPosition(voie, cameraActive);
    if (pos.numVoie == NULL) {
        fprintf(stderr, "addRoute - Erreur lors de la recuperation de valeurs de PTZ\n");
        return -1;
    }

    return addPositionFile(pos);
}

// Suppression d'une voie dans le fichier json
int removeRoute(char * voie){
    DEBUG_PRINT("Suppression de la voie n°%s\n", voie);

    int status = 0;
    
    // Charger le fichier JSON
    json_error_t error;
    json_t *root = json_load_file(PATH_POSITIONS, 0, &error);
    if (!root) {
        fprintf(stderr, "Erreur lors du chargement : %s\n", error.text);
        return -1;
    }

    // Supprimer une clé

    if (json_object_del(root, voie) != 0) {
        fprintf(stderr, "removeRoute - Erreur : impossible de supprimer la clé '%s'\n", voie);
        status = -1;
    } else {
        DEBUG_PRINT("Clé '%s' supprimée avec succès.\n", voie);
    }

    // Sauvegarder le fichier JSON modifié
    if (json_dump_file(root, PATH_POSITIONS, JSON_INDENT(4)) != 0) {
        fprintf(stderr, "removeRoute - Erreur lors de l'écriture dans le fichier\n");
        return -1;
    }

    // Nettoyer
    json_decref(root);
    DEBUG_PRINT("Fichier JSON mis à jour = %d.\n", status);
    return status;
}

// Affichage d'une voie depuis le fichier json
int showRoute(char * voie, const camera_t *cameraActive){
    DEBUG_PRINT("Affichage de la voie n°%s\n", voie);
    json_error_t error; 
    json_t *root = json_load_file(PATH_POSITIONS, 0, &error);
    
    if (!root) {
        fprintf(stderr, "showRoute - Erreur lors du chargement : %s\n", error.text);
        return -1;
    } 

    json_t *position_array = json_object_get(root, voie);
    if (!json_is_array(position_array)) {
        fprintf(stderr, "showRoute - Erreur : la clé '%s' n'est pas un tableau\n", voie);
        json_decref(root);
        return -1;
    }

    positionPTZ pos;
    pos.numVoie = voie;
    pos.pan = json_real_value(json_array_get(position_array, 0));
    pos.tilt = json_real_value(json_array_get(position_array, 1));
    pos.zoom = json_real_value(json_array_get(position_array, 2));
    pos.camera = json_integer_value(json_array_get(position_array, 3));

    json_decref(root);

    DEBUG_PRINT("Voie: %s, Pan: %.4f, Tilt: %.4f, Zoom: %.4f\n", pos.numVoie, pos.pan, pos.tilt, pos.zoom);

    if (strcmp(voie, "0") != 0 && cameraActive->id != pos.camera) {
        if (pos.camera == 1) setActiveCamera(&camera1);
        else if (pos.camera == 2) setActiveCamera(&camera2);
        else {
            fprintf(stderr, "showRoute - Erreur : la caméra %d n'existe pas\n", pos.camera);
            return -1;
        }
    }
    
    return allerPosition(pos, cameraActive);
}


