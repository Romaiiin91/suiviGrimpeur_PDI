/******************************************************************************
 * Name        : enregistrementVideo.c
 * Description : Programme to record video frames from shared memory.
 * Author      : Romain
 * Date        : 16/03/2025
 * Version     : 1.0
 *
 * This programme reads video frames from shared memory and writes them
 * into a video file using FFmpeg. It uses semaphores for synchronisation
 * and signal handling for graceful termination.
 *
 * Compilation : make bin/enregistrementVideo
 *
 * Usage       : ./enregistrementVideo <output_file> <orientation>
 *
 * Notes       : 
 * - The programme uses shared memory and semaphores for inter-process communication.
 * - FFmpeg is used for video encoding and processing.
 * - Signal handling is implemented for graceful termination.
 *
 ******************************************************************************/

#include <stdio.h>

#include <utils.h>
#include <jansson.h>

#define VIDEO_TEMP "./data/videos/temp.mp4"

#ifdef DEBUG
    #define LOGLEVEL "-loglevel 32"
#else
    #define LOGLEVEL "-loglevel 8"
#endif
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void signalHandler(int numSig);
void decouperVideo(const char * nomFichier, int orientation);

int enregistrementEnCours = 1;


int main(int argc, char * argv[]) {
    DEBUG_PRINT("[%d] Démarrage de l'enregistrement video\n", getpid());

    if (argc < 3) {
        DEBUG_PRINT("Pas assez d'arguments\n");
        exit(EXIT_FAILURE);
    }

    int shm_fd;
    void* virtAddr;
    sem_t *semReaders, *semWriter, *semMutex, *semNewFrame, *semActiveReaders;
    int reader_count;

    int orientation;
    orientation = atoi(argv[2]);
    
    
    // Installation du gestionnaire de signaux pour géré l'arrêt du programme
    struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask ), "enregistrementVideo: sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGINT, &newAction, NULL), "enregistrementVideo: sigaction (SIGINT)");
    CHECK(sigaction(SIGTERM, &newAction, NULL), "enregistrementVideo: sigaction (SIGTERM)");
    
   

    // Overture des semaphores
    CHECK_NULL(semReaders = sem_open(SEM_READERS, 0), "enregistrementVideo: sem_open(semReaders)");
    CHECK_NULL(semWriter = sem_open(SEM_WRITER, 0), "enregistrementVideo: sem_open(semWriter)");
    CHECK_NULL(semMutex = sem_open(SEM_MUTEX, 0), "enregistrementVideo: sem_open(semMutex)");
    CHECK_NULL(semNewFrame = sem_open(SEM_NEW_FRAME, 0), "enregistrementVideo: sem_open(semNewFrame)");
    CHECK_NULL(semActiveReaders = sem_open(SEM_ACTIVE_READERS, 0), "enregistrementVideo: sem_open(semActiveReaders)");

    // Ouvrir la mémoire partagée
    CHECK(shm_fd = shm_open(SHM_IMAGE, O_CREAT | O_RDWR, 0666), "enregistrementVideo: shm_open(SHM_IMAGE)");
    CHECK(ftruncate(shm_fd, SHM_FRAME_SIZE), "enregistrementVideo: ftruncate(shm_fd)");   
    CHECK_NULL(virtAddr = mmap(0, SHM_FRAME_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0), "enregistrementVideo: mmap(virtAddr)");

    // commande ffmpeg
    char cmd[1000]; // a mieux definir

    #ifdef X264
    sprintf(cmd, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s", "ffmpeg", "-y", LOGLEVEL, "-f rawvideo", "-fflags +discardcorrupt+genpts", "-pixel_format bgr24", "-s 1280x720", "-r 25",  "-framerate 25", "-i pipe:0", "-c:v libx264 "  " -preset medium", "-pix_fmt yuv420p", "-an", VIDEO_TEMP);
    
    #else

    sprintf(cmd, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s", "ffmpeg", "-y", LOGLEVEL, "-f rawvideo", "-fflags +discardcorrupt+genpts", "-pixel_format bgr24", "-s 1280x720", "-r 25", "-framerate 25", "-i pipe:0", "-c:v h264_v4l2m2m"  " -b:v 5M ", "-pix_fmt yuv420p", "-an", VIDEO_TEMP);

    #endif
    DEBUG_PRINT("Commande FFMPEG : \"%s\"\n", cmd);




    // Ouvrir un pipe vers FFmpeg
    FILE* ffmpeg;
    CHECK_NULL(ffmpeg = popen(cmd, "w"), "enregistrementVideo: popen(ffmpeg)");

        
    
    /*
        Commande ffmpeg pour l'enregistrement vidéo :

        -y                                                      // Écrase le fichier de sortie sans confirmation
        -loglevel 32                                            // Définit le niveau de journalisation à "warning"
        -f rawvideo                                             // Spécifie que l'entrée est une vidéo brute sans en-tête
        -fflags +discardcorrupt                                 // Ignore les images corrompues
        -pixel_format bgr24                                     // Format de l'entrée en BGR 24 bits
        -s 1280x720                                             // Définit la résolution d'entrée à 1280x720
        -r 25                                                   // Débit d'images (FPS) : 25
        -i pipe:0                                               // L'entrée provient du pipe standard (stdin)
        -c:v h264_v4l2m2m                                       // Codec vidéo H.264pour gpu pour la sortie
        -preset medium                                          // Compromis entre vitesse et qualité d'encodage
        -pix_fmt yuv420p                                        // Format de pixel YUV 4:2:0 pour compatibilité
        -crf 23                                                 // Qualité de compression (23 = bonne qualité par défaut)
        -an                                                     // Désactive l'audio
        ./data/videos/20250208_2319_nom_prenom_voie.mp4      // Fichier de sortie (nom de fichier personnalisé)
    */

    // Incrémenter le compteur de lecteurs actifs
    sem_post(semActiveReaders);




    while (enregistrementEnCours) {

        sem_wait(semNewFrame);  // Attendre une nouvelle image

        sem_wait(semMutex);  // Acquérir le sémaphore de synchronisation

        // Incrémenter le nombre de lecteurs
        
        sem_getvalue(semReaders, &reader_count);
        if (reader_count == 0) {
            sem_wait(semWriter);  // Bloquer le rédacteur si c'est le premier lecteur
        }
        sem_post(semReaders);  // Incrémenter le compteur de lecteurs

        sem_post(semMutex);  // Relâcher le sémaphore de synchronisation

        
        
        
        // Lire l’image de la mémoire partagée and write it to the pipe
        fwrite(virtAddr, 1, SHM_FRAME_SIZE, ffmpeg);




        sem_wait(semMutex);  // Acquérir le sémaphore de synchronisation

        // Décrémenter le nombre de lecteurs
        sem_wait(semReaders);  // Décrémenter le compteur de lecteurs
        sem_getvalue(semReaders, &reader_count);
        if (reader_count == 0) {
            sem_post(semWriter);  // Débloquer le rédacteur si c'est le dernier lecteur
        }

        sem_post(semMutex);  // Relâcher le sémaphore de synchronisation
       
    }

    // Décrémenter le compteur de lecteurs actifs à la fin de l'exécution du lecteur
    sem_wait(semActiveReaders);

    // Fermer les ressources
    pclose(ffmpeg);
    munmap(virtAddr, SHM_FRAME_SIZE);
    close(shm_fd);

    #ifdef PC
    printf("\n\n----------------- Fin de l'enregistrement vidéo. -----------------\n\n");
    #endif

    // Fermer les sémaphores
    sem_close(semReaders);
    sem_close(semWriter);
    sem_close(semMutex);
    sem_close(semNewFrame);
    sem_close(semActiveReaders);


    // Découper la vidéo pour garder seulement la partie interessante
    decouperVideo( argc > 1 ? argv[1] : "./data/videos/output.mp4", orientation);

    
    exit(EXIT_SUCCESS);
    return 0;
}




static void signalHandler(int numSig)
{ 
    switch(numSig) {
        case SIGTERM : // traitement de SIGTERM
            DEBUG_PRINT("\t[%d] --> Arrêt du programme d'enregistrement video en cours...\n", getpid());
            enregistrementEnCours = 0;
            break;
        case SIGINT : // traitement de SIGINT
            DEBUG_PRINT("\t[%d] --> Interruption du programme d'enregistrement video en cours...\n", getpid());
            exit(EXIT_FAILURE);
            break;
        default :
            printf (" Signal %d non traité \n", numSig );
            break ;
    }
}


void decouperVideo(const char * nomFichier, int orientation) {
    DEBUG_PRINT("Découpage de la vidéo \"%s\"\n", nomFichier);
    FILE* fvideo;
    float frameDebut = 0, frameFin = 0;


    fvideo = fopen(PATH_FRAMES, "r");
    if (fvideo == NULL) {
        DEBUG_PRINT("Erreur lors de l'ouverture du fichier \"%s\"\n", PATH_FRAMES);
        frameFin = 9999;
    } else {
        fscanf(fvideo, "%f\n%f", &frameDebut, &frameFin);
        fclose(fvideo);
    }
    

    DEBUG_PRINT("Frame de début : %f\nFrame de fin : %f\n", frameDebut, frameFin);

    // Why ?
    // if (frameDebut == 0){
    //     return;
    // }

    // Charger les paramètres depuis le fichier JSON Peut etre supprimer car on ne découpe pu la vidéo
    json_error_t error;
    json_t *root = json_load_file(PATH_PARAM_DETECTION, 0, &error);

    CHECK_NULL(root, "detection: json_load_file(PATH_PARAM_DETECTION)");

    int numberFrameBeforeFirstMove = json_integer_value(json_object_get(root, "numberFrameBeforeFirstMove"));
    int numberFrameWithoutMove = json_integer_value(json_object_get(root, "numberFrameWithoutMove"));

    json_decref(root);

    frameDebut = MAX(0, frameDebut - numberFrameBeforeFirstMove);
    frameFin = MAX(frameFin - 0.3*numberFrameWithoutMove, frameDebut + 1);

    char cmd[256];

    #ifdef X264
    sprintf(cmd, "%s %s %s %s %s %s%d%s %s", "ffmpeg", "-y", LOGLEVEL,  "-i", VIDEO_TEMP, "-vf \"transpose=", orientation/90, "\"", nomFichier); // A tester 

    // sprintf(cmd, "%s %s %s %s %s  %s %.2f %s %.2f %s %s", "ffmpeg", "-y", LOGLEVEL,  "-i", VIDEO_TEMP,  "-ss", frameDebut/25.0, "-to", frameFin/25.0, "-vf \"transpose=1\"", nomFichier); // Ancien avec découpage

    #else
    sprintf(cmd, "%s %s %s %s %d %s %s  %s %s", "ffmpeg", "-y", LOGLEVEL, "-display_rotation", 360 - orientation, "-i", VIDEO_TEMP, "-c copy", nomFichier);

    // sprintf(cmd, "%s %s %s %s %d %s %s  %s %.2f %s %.2f %s %s", "ffmpeg", "-y", LOGLEVEL, "-display_rotation", 360 - orientation, "-i", VIDEO_TEMP,  "-ss", frameDebut/25.0, "-to", frameFin/25.0, "-c copy", nomFichier);

    #endif
    
    DEBUG_PRINT("Commande FFMPEG : \"%s\"\n", cmd);
    system(cmd);

    #ifdef PC
    printf("\n\n----------------- Fin découpage vidéo. -----------------\n\n");
    #endif
}