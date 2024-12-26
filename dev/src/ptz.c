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


void requetePTZ(const char * cmd, const char *val){

    // URL
    char url[256];
    sprintf(url, "%s@%s/%s?%s=%s",ENTETE_HTTP, IP, SCRIPT_PTZ, cmd, val);
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