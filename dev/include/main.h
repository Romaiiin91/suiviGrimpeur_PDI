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

void bye();
void processusEcritureMemoire();
int enregistrerVideo(const char * donnees);
void processusDetection();
static void signalHandler(int numSig);
void gestionOrdres();
void ack(int status, pid_t pidCgi);
void initSemaphores();
void initCamera();
void initSegmentMemoire();
void setParamCamera(camera_t *camera);
int setActiveCamera(camera_t *camera);


/* ------------------------------------------------------------------------ */
/*                      M A C R O - F O N C T I O N S                       */
/* ------------------------------------------------------------------------ */



