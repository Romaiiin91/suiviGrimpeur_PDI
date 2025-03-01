/* ------------------------------------------------------------------------ */
/*                       Suivi grimpeur - Positions                         */
/*                        Auteur : CHEVALIER Romain                         */
/*                            Date : 26-10-2024                             */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */

#include <utils.h>
#include <jansson.h>


// Requetes http
#include <curl/curl.h> 

/* ------------------------------------------------------------------------ */
/*              C O N S T A N T E S     S Y M B O L I Q U E S               */
/* ------------------------------------------------------------------------ */

#define LONGUEUR_LIGNE_FILE     35

/* ------------------------------------------------------------------------ */
/*              D É F I N I T I O N S   D E   T Y P E S                     */
/* ------------------------------------------------------------------------ */

typedef struct{
    char * numVoie;
    int camera;
    double pan, tilt, zoom;
} positionPTZ;

struct MemoryStruct {
  char *memory;
  size_t size;
};



/* ------------------------------------------------------------------------ */
/*            P R O T O T Y P E S    D E    F O N C T I O N S               */
/* ------------------------------------------------------------------------ */

size_t writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);


int addPositionFile(const positionPTZ pos);
int allerPosition(positionPTZ pos, const camera_t *cameraActive);

double recupererValeur(const char *data, const char *key);
void supprimerPositionFile();

positionPTZ recupererPosition(char * voie, const camera_t *cameraActive);

int addRoute(char * voie, const camera_t *cameraActive);
int removeRoute(char * voie);
int showRoute(char * voie, const camera_t *cameraActive);

// Dans le main
int setActiveCamera(camera_t *camera);


