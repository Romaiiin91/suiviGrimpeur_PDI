/* ------------------------------------------------------------------------ */
/*                            Suivi grimpeur                                */
/*                        Auteur : CHEVALIER Romain                         */
/*                            Date : 26-10-2024                             */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */

#include <ptz.h>


/* ------------------------------------------------------------------------ */
/*                 V A R I A B L E S    G L O B A L E S                     */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*            D E F I N I T I O N   D E    F O N C T I O N S                */
/* ------------------------------------------------------------------------ */


// int requetePTZ(const char * cmd, const char *val){

//     // URL
//     char url[256];
//     sprintf(url, "%s@%s/%s?%s=%s",ENTETE_HTTP, IP, SCRIPT_PTZ, cmd, val);
//     DEBUG_PRINT("%s\n", url);

//     int status = 0;

//     CURL *curl;
//     CURLcode res;
 
//     curl_global_init(CURL_GLOBAL_DEFAULT);
//     curl = curl_easy_init();

//     if (curl) {
//         curl_easy_setopt(curl, CURLOPT_URL, url);  // URL de la requête
//         res = curl_easy_perform(curl);  // Effectuer la requête

//         // Vérification de la réussite de la requête
//         if (res != CURLE_OK) {
//             fprintf(stderr, "Erreur de requête : %s\n", curl_easy_strerror(res));
//             status = -1;
//         }

//         // Nettoyage
//         curl_easy_cleanup(curl);
//     }
//     curl_global_cleanup();

    
//     return status;
// }

int requetePTZ(const char *dir, const float angle, const camera_t *cameraActive){
    char cmd[15];

    if (strcmp(dir, "up")==0)           sprintf(cmd, "%s=%.1f", cameraActive->cmdVertical, angle*cameraActive->up);
    else if (strcmp(dir, "down")==0)    sprintf(cmd, "%s=%.1f", cameraActive->cmdVertical, angle*cameraActive->down);
    else if (strcmp(dir, "left")==0)    sprintf(cmd, "%s=%.1f", cameraActive->cmdHorizontal, angle*cameraActive->left);
    else if (strcmp(dir, "right")==0)   sprintf(cmd, "%s=%.1f", cameraActive->cmdHorizontal, angle*cameraActive->right);
    else if (strcmp(dir, "zoom")==0)    sprintf(cmd, "%s=%.1f", "zoom", angle);
    else {
        perror("requetePTZ: Direction non reconnue");
        return -1;
    }

    // URL
    char url[256];
    sprintf(url, "%s@%s/%s?%s",ENTETE_HTTP, cameraActive->ip, SCRIPT_PTZ, cmd);
    DEBUG_PRINT("requetePTZ: %s\n", url);

    int status = 0;

    CURL *curl;
    CURLcode res;
 
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);  // URL de la requête
        res = curl_easy_perform(curl);  // Effectuer la requête

        // Vérification de la réussite de la requête
        if (res != CURLE_OK) {
            fprintf(stderr, "Erreur de requête : %s\n", curl_easy_strerror(res));
            status = -1;
        }

        // Nettoyage
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    
    return status;
}