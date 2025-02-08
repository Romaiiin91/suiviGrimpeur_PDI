#include <stdio.h>

#include <utils.h>


static void signalHandler(int numSig);

int enregistrementEnCours = 1;


int main(int argc, char * argv[]) {
    DEBUG_PRINT("[%d] Démarrage de l'enregistrement video\n", getpid());

    int shm_fd;
    void* virtAddr;
    sem_t *semReaders, *semWriter, *semMutex;
    int reader_count;
    
    
    // Installation du gestionnaire de signaux pour géré l'arrêt du programme
    struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask ), " sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGINT, &newAction, NULL), "sigaction (SIGINT)");
    
   

    // Overture des semaphores
    CHECK_NULL(semReaders = sem_open(SEM_READERS, 0), "enregistrementVideo: sem_open(semReaders)");
    CHECK_NULL(semWriter = sem_open(SEM_WRITER, 0), "enregistrementVideo: sem_open(semWriter)");
    CHECK_NULL(semMutex = sem_open(SEM_MUTEX, 0), "enregistrementVideo: sem_open(semMutex)");

    // Ouvrir la mémoire partagée
    CHECK(shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666), "enregistrementVideo: shm_open(SHM_NAME)");
    CHECK(ftruncate(shm_fd, SHM_FRAME_SIZE), "enregistrementVideo: ftruncate(shm_fd)");   
    CHECK_NULL(virtAddr = mmap(0, SHM_FRAME_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0), "enregistrementVideo: mmap(virtAddr)");

    // commande ffmpeg
    char cmd[256];
    sprintf(cmd, "%s %s %s %s %s %s %s %s %s %s %s %s %s", "ffmpeg", "-y", "-loglevel 32", "-f rawvideo", "-pix_fmt bgr24", "-s 1280x720", "-r 25", "-i pipe:0", "-c:v libx264", "-crf 23", "-preset medium", "-an", argc > 1 ? argv[1]:"serveur/videos/output.mp4");
    DEBUG_PRINT("Commande FFMPEG : \"%s\"\n", cmd);



    // Ouvrir un pipe vers FFmpeg
    FILE* ffmpeg;
    CHECK_NULL(ffmpeg = popen(cmd, "w"), "enregistrementVideo: popen(ffmpeg)");

        
    
        /*
        "ffmpeg",                       // args[0] : Nom de l'exécutable
        "-y",                           // args[1] : Overwrite output files
        "-loglevel 32",                    // args[2] : Option pour le niveau de log
        -f rawvideo",                   // args[4] : Option pour l'entrée
        "-i ",                           // args[4] : Option pour l'entrée
        "http://serveur:serveur@192.168.1.13/axis-cgi/mjpg/video.cgi?resolution=1280x720&fps=25&compression=25",                // args[5] : URL d'entrée
        "-c:v",                         // args[6] : Option pour le codec vidéo
        "libx264",                      // args[7] : Codec vidéo (H.264)
        "-crf",                         // args[8] : Option pour la qualité (Constant Rate Factor)
        "23",                           // args[9] : Valeur CRF (23 est une bonne qualité)
        "-preset",                      // args[10] : Option pour la vitesse d'encodage
        "medium",                       // args[11] : Vitesse d'encodage (medium est un bon compromis)
        "-an",                          // args[12] : Désactive l'audio
        "-r",                           // args[13] : Option pour les FPS
        "25",                           // args[14] : FPS de sortie
        -vsync", "cfr",                // args[15] : Synchronisation des frames
        "./serveur/videos/20250201_2228_x_x_voie1.mp4", // args[15] : Fichier de sortie
    
    */



    while (enregistrementEnCours) {

        sem_wait(semMutex);  // Acquérir le sémaphore de synchronisation

        // Incrémenter le nombre de lecteurs
        
        sem_getvalue(semReaders, &reader_count);
        if (reader_count == 0) {
            sem_wait(semWriter);  // Bloquer le rédacteur si c'est le premier lecteur
        }
        sem_post(semReaders);  // Incrémenter le compteur de lecteurs

        sem_post(semMutex);  // Relâcher le sémaphore de synchronisation

        
        
        
        // Lire l’image de la mémoire partagée
        fwrite(virtAddr, 1, SHM_FRAME_SIZE, ffmpeg);




        sem_wait(semMutex);  // Acquérir le sémaphore de synchronisation

        // Décrémenter le nombre de lecteurs
        sem_wait(semReaders);  // Décrémenter le compteur de lecteurs
        sem_getvalue(semReaders, &reader_count);
        if (reader_count == 0) {
            sem_post(semWriter);  // Débloquer le rédacteur si c'est le dernier lecteur
        }

        sem_post(semMutex);  // Relâcher le sémaphore de synchronisation


        /*
        Le sémaphore mutex_sem est utilisé pour protéger les variables partagées, comme le compteur de lecteurs (reader_count), afin d'éviter que plusieurs processus ne modifient cette variable en même temps. Sans mutex_sem, il y a un risque que deux lecteurs incrémentent ou décrémentent reader_count simultanément, ce qui pourrait entraîner des incohérences.

        */
    }

    // Fermer les ressources
    pclose(ffmpeg);
    munmap(virtAddr, SHM_FRAME_SIZE);
    close(shm_fd);

    // Fermer les sémaphores
    sem_close(semReaders);
    sem_close(semWriter);
    sem_close(semMutex);
    
    exit(EXIT_SUCCESS);
    return 0;
}

static void signalHandler(int numSig)
{ 
    switch(numSig) {
        case SIGINT : // traitement de SIGINT
            DEBUG_PRINT("\t[%d] --> Arrêt du programme d'écriture en cours...\n", getpid());
            enregistrementEnCours = 0;
            break;
        default :
            printf (" Signal %d non traité \n", numSig );
            break ;
    }
}
