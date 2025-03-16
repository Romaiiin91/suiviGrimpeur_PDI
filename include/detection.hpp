/* ------------------------------------------------------------------------ */
/*                       Suivi grimpeur - Detection                         */
/*                        Auteur : CHEVALIER Romain                         */
/*                            Date : 26-10-2024                             */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */

#include <opencv4/opencv2/opencv.hpp>
#include <cstdio>
#include <iostream>
#include <opencv2/objdetect.hpp>



#include <ptz.h>
#include <utils.h>
#include <jansson.h>

/* ------------------------------------------------------------------------ */
/*              C O N S T A N T E S     S Y M B O L I Q U E S               */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*              D É F I N I T I O N S   D E   T Y P E S                     */
/* ------------------------------------------------------------------------ */



/* ------------------------------------------------------------------------ */
/*              C O N S T A N T E S     S Y M B O L I Q U E S               */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*              D É F I N I T I O N S   D E   T Y P E S                     */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                      M A C R O - F O N C T I O N S                       */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*            P R O T O T Y P E S    D E    F O N C T I O N S               */
/* ------------------------------------------------------------------------ */


/**
 * @brief Callback function to handle data received during an HTTP request
 * 
 * @param contents Pointer to the data received
 * @param size Size of each data element
 * @param nmemb Number of data elements
 * @param output Pointer to the string where the received data will be appended
 * @return size_t Total size of the data processed (size * nmemb)
 */
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);

/**
 * @brief Retrieve the current zoom level of the active camera
 * 
 * @param cameraActive Pointer to the active camera
 * @return float The current zoom level of the camera
 */
float getActualZoom(const camera_t* cameraActive);

/**
 * @brief Handle a signal received by the program
 * 
 * @param numSig Signal number that was received
 */
static void signalHandler(int numSig);