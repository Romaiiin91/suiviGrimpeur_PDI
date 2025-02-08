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
    // sprintf(cmd, "%s %s %s %s %s %s %s %s %s %s %s %s %s", "ffmpeg", "-y", "-loglevel 32", "-f rawvideo", "-pix_fmt bgr24", "-s 1280x720", "-r 25", "-i pipe:0", "-c:v libx264", "-preset medium", "-an", argc > 1 ? argv[1]:"serveur/videos/output.mp4");
    sprintf(cmd, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s", "ffmpeg", "-y", "-loglevel 32", "-f rawvideo", "-fflags +discardcorrupt", "-pixel_format bgr24", "-s 1280x720", "-r 25", "-i pipe:0",  "-c:v libx264", "-preset medium", "-pix_fmt yuv420p", "-crf 23", "-an", argc > 1 ? argv[1] : "serveur/videos/output.mp4");
    
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
        -c:v libx264                                            // Codec vidéo H.264 pour la sortie
        -preset medium                                          // Compromis entre vitesse et qualité d'encodage
        -pix_fmt yuv420p                                        // Format de pixel YUV 4:2:0 pour compatibilité
        -crf 23                                                 // Qualité de compression (23 = bonne qualité par défaut)
        -an                                                     // Désactive l'audio
        ./serveur/videos/20250208_2319_nom_prenom_voie.mp4      // Fichier de sortie (nom de fichier personnalisé)
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
       usleep(39000); // 25 FPS
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
            DEBUG_PRINT("\t[%d] --> Arrêt du programme d'enregistrement video en cours...\n", getpid());
            enregistrementEnCours = 0;
            break;
        default :
            printf (" Signal %d non traité \n", numSig );
            break ;
    }
}
