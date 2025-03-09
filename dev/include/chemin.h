

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
#define PATH_CAMERAS            "/home/romain/Documents/suiviGrimpeur_PDI/dev/data/cameras.json"
#define PATH_CAMERA_ACTIVE      "/home/romain/Documents/suiviGrimpeur_PDI/dev/bin/cameraActive.json"
#define PATH_FRAMES             "/home/romain/Documents/suiviGrimpeur_PDI/dev/bin/frames" 


#ifdef DEBUG
#define PATH_EXE_ENREGISTREMENT_VIDEO            "/home/romain/Documents/suiviGrimpeur_PDI/dev/bin/enregistrementVideoDEBUG"
#define PATH_EXE_DETECTION               "/home/romain/Documents/suiviGrimpeur_PDI/dev/bin/detectionDEBUG"
#define PATH_EXE_ECRITURE_MEMOIRE         "/home/romain/Documents/suiviGrimpeur_PDI/dev/bin/ecritureMemoireDEBUG"
#else
#define PATH_EXE_ECRITURE_MEMOIRE               "/home/romain/Documents/suiviGrimpeur_PDI/dev/bin/ecritureMemoire"
#define PATH_EXE_DETECTION                      "/home/romain/Documents/suiviGrimpeur_PDI/dev/bin/detection"
#define PATH_EXE_ENREGISTREMENT_VIDEO           "/home/romain/Documents/suiviGrimpeur_PDI/dev/bin/enregistrementVideo"
#endif


#else

#define PATH_FILE_ORDRE         "/home/pi/suiviGrimpeur/bin/fichierOrdre"
#define PATH_FPID               "/home/pi/suiviGrimpeur/bin/suiviGrimpeur.pid"
#define PATH_VIDEOS             "/home/pi/suiviGrimpeur/data/videos"
#define PATH_LOG                "/home/pi/suiviGrimpeur/log/debug.log"
#define PATH_LOG_CGI            "/home/pi/suiviGrimpeur/log/debugCgi.log"
#define PATH_PARAM_DETECTION    "/home/pi/suiviGrimpeur/data/paramDetection.json"  
#define PATH_POSITIONS          "/home/pi/suiviGrimpeur/data/positionsEnregistrees.json"
#define PATH_CAMERAS            "/home/pi/suiviGrimpeur/data/cameras.json"
#define PATH_CAMERA_ACTIVE      "/home/pi/suiviGrimpeur/bin/cameraActive.json"
#define PATH_FRAMES             "/home/pi/suiviGrimpeur/bin/frames"




#ifdef DEBUG
#define PATH_EXE_ENREGISTREMENT_VIDEO    "/home/pi/suiviGrimpeur/bin/enregistrementVideoDEBUG"
#define PATH_EXE_DETECTION               "/home/pi/suiviGrimpeur/bin/detectionDEBUG"
#define PATH_EXE_ECRITURE_MEMOIRE         "/home/pi/suiviGrimpeur/bin/ecritureMemoireDEBUG"
#else
#define PATH_EXE_ECRITURE_MEMOIRE               "/home/pi/suiviGrimpeur/bin/ecritureMemoire"
#define PATH_EXE_DETECTION                      "/home/pi/suiviGrimpeur/bin/detection"
#define PATH_EXE_ENREGISTREMENT_VIDEO           "/home/pi/suiviGrimpeur/bin/enregistrementVideo"
#endif

#endif

#define SHM_IMAGE "/shmImage"
#define HEIGHT 720
#define WIDTH 1280
#define FRAME_SIZE (WIDTH * HEIGHT * 3)// (WIDTH * HEIGHT * CHANNELS)
#define SHM_FRAME_SIZE ((FRAME_SIZE + sysconf(_SC_PAGE_SIZE) - 1) / sysconf(_SC_PAGE_SIZE)) * sysconf(_SC_PAGE_SIZE) 

#define SHM_ORDRE "/shmOrdre"
#define SHM_ORDRE_SIZE sysconf(_SC_PAGE_SIZE) 

#define SEM_READERS "/semReaders"
#define SEM_WRITER  "/semWriter"
#define SEM_MUTEX   "/semMutex"
#define SEM_NEW_FRAME "/semNewFrame"
#define SEM_ACTIVE_READERS "/semActiveReaders"

#endif