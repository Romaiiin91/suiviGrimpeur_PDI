/* ------------------------------------------------------------------------ */
/*                            Suivi grimpeur                                */
/*                        Auteur : CHEVALIER Romain                         */
/*                            Date : 26-10-2024                             */
/* ------------------------------------------------------------------------ */


#ifndef __UTIL_H

#define __UTIL_H

/* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>

// Semaphore
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

// Signaux
#include <signal.h>
#include <bits/sigaction.h> // Include pour evider les erreurs sur vscode 
#include <bits/types/sigset_t.h>


// Chemins
#include <chemin.h>


/* ------------------------------------------------------------------------ */
/*              C O N S T A N T E S     S Y M B O L I Q U E S               */
/* ------------------------------------------------------------------------ */

#define ENTETE_HTTP         "http://serveur:serveur"
#define IP                  "192.168.1.13" // 1.13
#define SCRIPT_VIDEO        "axis-cgi/mjpg/video.cgi?resolution=1280x720&fps=25&compression=25"
#define SCRIPT_PTZ          "axis-cgi/com/ptz.cgi"





/* ------------------------------------------------------------------------ */
/*              D É F I N I T I O N S   D E   T Y P E S                     */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                      M A C R O - F O N C T I O N S                       */
/* ------------------------------------------------------------------------ */



#define CHECK_T(status, msg)                                                 \
  if (0 != (status))   {                                                     \
    fprintf(stderr, "pthread erreur : %s avec erreur n°%d\n", msg, status);  \
    exit (EXIT_FAILURE);                                                     \
  }

#define CHECK(status, msg)                                                   \
    if (-1 == (status)) {                                                    \
        perror(msg);                                                         \
        exit(EXIT_FAILURE);                                                  \
    }

#define CHECK_NULL(status, msg)                                              \
    if (NULL == (status)) {                                                  \
        perror(msg);                                                         \
        exit(EXIT_FAILURE);                                                  \
    }

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))


#ifdef DEBUG
    #define DEBUG_PRINT(msg, ...) do {                                      \
        FILE *log_file;                                                     \
        CHECK_NULL(log_file = fopen(PATH_LOG, "a"), "fopen(debug.log)"); \
        fprintf(log_file, msg, ##__VA_ARGS__);                              \
        fflush(log_file);                                                   \
        fclose(log_file);                                                   \
    } while (0)
#else
    #define DEBUG_PRINT(msg, ...) // Ne fait rien si DEBUG n'est pas défini
#endif

    
#endif
