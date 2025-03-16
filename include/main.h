/* ------------------------------------------------------------------------ */
/*                            Suivi grimpeur                                */
/*                        Auteur : CHEVALIER Romain                         */
/*                            Date : 26-10-2024                             */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */

// Utils
#include <utils.h>
#include <positions.h>
#include <ptz.h>


// Processus
#include <sys/wait.h>


// Requetes http
#include <curl/curl.h>



#include <sys/stat.h>
#include <errno.h>

#include <sys/statvfs.h>



/* ------------------------------------------------------------------------ */
/*              C O N S T A N T E S     S Y M B O L I Q U E S               */
/* ------------------------------------------------------------------------ */

#define MIN_DISK_SPACE 300000000// 300 Mo car 1s ~= 0.6Mo et on veut 6 minutes minimum


/* ------------------------------------------------------------------------ */
/*              D É F I N I T I O N S   D E   T Y P E S                     */
/* ------------------------------------------------------------------------ */



/* ------------------------------------------------------------------------ */
/*            P R O T O T Y P E S    D E    F O N C T I O N S               */
/* ------------------------------------------------------------------------ */

/**
 * @brief Clean up and terminate the program.
 */
void bye();

/**
 * @brief Handle the process of writing data to memory.
 */
void processusEcritureMemoire();

/**
 * @brief Record a video using the provided data.
 * 
 * @param donnees Data used for video recording.
 * @return int 0 if successful, -1 in case of error.
 */
int enregistrerVideo(const char * donnees);

/**
 * @brief Handle the process of detection.
 */
void processusDetection();

/**
 * @brief Handle signals received by the program.
 * 
 * @param numSig Signal number.
 */
static void signalHandler(int numSig);

/**
 * @brief Manage incoming orders or commands.
 */
void gestionOrdres();

/**
 * @brief Send an acknowledgment to a CGI process.
 * 
 * @param status Status code to acknowledge.
 * @param pidCgi Process ID of the CGI process.
 */
void ack(int status, pid_t pidCgi);

/**
 * @brief Initialize semaphores used in the program.
 */
void initSemaphores();

/**
 * @brief Initialize the camera for use.
 */
void initCamera();

/**
 * @brief Initialize the shared memory segment.
 */
void initSegmentMemoire();

/**
 * @brief Set parameters for the camera.
 * 
 * @param camera Pointer to the camera structure.
 */
void setParamCamera(camera_t *camera);

/**
 * @brief Activate a specific camera.
 * 
 * @param camera Pointer to the camera structure to activate.
 * @return int 0 if successful, -1 in case of error.
 */
int setActiveCamera(camera_t *camera);


/* ------------------------------------------------------------------------ */
/*                      M A C R O - F O N C T I O N S                       */
/* ------------------------------------------------------------------------ */



