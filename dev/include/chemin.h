

#ifndef __CHEMIN_H

#define __CHEMIN_H

#ifdef PC
#define PATH_FILE_ORDRE         "/home/romain/Documents/suiviGrimpeur_PDI/dev/bin/fichierOrdre"
#define PATH_FPID               "/home/romain/Documents/suiviGrimpeur_PDI/dev/bin/suiviGrimpeur.pid"
#define PATH_VIDEOS             "/home/romain/Documents/suiviGrimpeur_PDI/dev/data/videos"
#define PATH_LOG                "/home/romain/Documents/suiviGrimpeur_PDI/dev/log/debug.log"
#define PATH_LOG_CGI            "/home/romain/Documents/suiviGrimpeur_PDI/dev/log/debugCgi.log"
#define PATH_PARAM_DETECTION    "/home/romain/Documents/suiviGrimpeur_PDI/dev/data/paramDetection.json"  

#define PATH_POSITIONS          "/home/romain/Documents/suiviGrimpeur_PDI/dev/data/positionsEnregistrees.json"
#define PATH_FRAMES             "/home/romain/Documents/suiviGrimpeur_PDI/dev/bin/frames" 



#else

#define PATH_FILE_ORDRE         "/home/pi/suiviGrimpeur/bin/fichierOrdre"
#define PATH_FPID               "/home/pi/suiviGrimpeur/bin/suiviGrimpeur.pid"
#define PATH_VIDEOS             "/home/pi/suiviGrimpeur/data/videos"
#define PATH_LOG                "/home/pi/suiviGrimpeur/log/debug.log"
#define PATH_LOG_CGI            "/home/pi/suiviGrimpeur/log/debugCgi.log"
#define PATH_PARAM_DETECTION    "/home/pi/suiviGrimpeur/data/paramDetection.json"  
#define PATH_POSITIONS          "/home/pi/suiviGrimpeur/data/positionsEnregistrees.json"
#define PATH_FRAMES             "/home/pi/suiviGrimpeur/bin/frames"

#endif

#define SHM_NAME "/shm_image"
#define WIDTH 1280
#define HEIGHT 720
#define CHANNELS 3
#define FRAME_SIZE (WIDTH * HEIGHT * CHANNELS)
#define SHM_FRAME_SIZE ((FRAME_SIZE + sysconf(_SC_PAGE_SIZE) - 1) / sysconf(_SC_PAGE_SIZE)) * sysconf(_SC_PAGE_SIZE) 

#define SEM_READERS "/semReaders"
#define SEM_WRITER  "/semWriter"
#define SEM_MUTEX   "/semMutex"
#define SEM_NEW_FRAME "/semNewFrame"
#define SEM_ACTIVE_READERS "/semActiveReaders"

#endif