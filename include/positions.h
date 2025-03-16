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

typedef struct PositionPTZ {
    char * numVoie;
    int camera;
    double pan, tilt, zoom;
} PositionPTZ;

struct MemoryStruct {
  char *memory;
  size_t size;
};



/* ------------------------------------------------------------------------ */
/*            P R O T O T Y P E S    D E    F O N C T I O N S               */
/* ------------------------------------------------------------------------ */

/**
 * @brief Callback function to write data received in memory.
 * 
 * @param contents Pointer to the data received.
 * @param size Size of each data element.
 * @param nmemb Number of data elements.
 * @param userp Pointer to user-defined data.
 * @return size_t Number of bytes written.
 */
size_t writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

/**
 * @brief Add a PTZ position to a file.
 * 
 * @param pos PTZ position to add.
 * @return int 0 if successful, -1 in case of error.
 */
int addPositionFile(const PositionPTZ pos);

/**
 * @brief Move the camera to a specific PTZ position.
 * 
 * @param pos PTZ position to move to.
 * @param cameraActive Pointer to the active camera.
 * @return int 0 if successful, -1 in case of error.
 */
int allerPosition(PositionPTZ pos, const camera_t *cameraActive);

/**
 * @brief Retrieve a value from a data string using a key.
 * 
 * @param data Data string to search.
 * @param key Key to look for.
 * @return double Retrieved value.
 */
double recupererValeur(const char *data, const char *key);

/**
 * @brief Delete all PTZ positions stored in the file.
 */
void supprimerPositionFile();

/**
 * @brief Retrieve a PTZ position based on a route and active camera.
 * 
 * @param voie Route identifier.
 * @param cameraActive Pointer to the active camera.
 * @return PositionPTZ Retrieved PTZ position.
 */
PositionPTZ recupererPosition(char * voie, const camera_t *cameraActive);

/**
 * @brief Add a route associated with a PTZ position.
 * 
 * @param voie Route identifier.
 * @param cameraActive Pointer to the active camera.
 * @return int 0 if successful, -1 in case of error.
 */
int addRoute(char * voie, const camera_t *cameraActive);

/**
 * @brief Remove a route associated with a PTZ position.
 * 
 * @param voie Route identifier.
 * @return int 0 if successful, -1 in case of error.
 */
int removeRoute(char * voie);

/**
 * @brief Display a route associated with a PTZ position.
 * 
 * @param voie Route identifier.
 * @param cameraActive Pointer to the active camera.
 * @return int 0 if successful, -1 in case of error.
 */
int showRoute(char * voie, const camera_t *cameraActive);


/**
 * @brief Set the active camera for PTZ operations.
 * 
 * @param camera Pointer to the camera to set as active.
 * @return int 0 if successful, -1 in case of error.
 */
int setActiveCamera(camera_t *camera);


