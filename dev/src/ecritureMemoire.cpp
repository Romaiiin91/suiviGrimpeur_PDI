#include <opencv2/opencv.hpp>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include <utils.h>

static void signalHandler(int numSig);

bool ecritureEnCours = true;

int main(int argc, char * argv[]) {
    DEBUG_PRINT("[%d] Démarrage de l'écriture mémoire\n", getpid());

    if (argc < 2) {
        DEBUG_PRINT("Pas url video");
        exit(EXIT_FAILURE);
    }

    int shm_fd;
    void* virtAddr;
    sem_t *semReaders, *semWriter, *semNewFrame, *semActiveReaders;
    int readers;
    
    
    // Installation du gestionnaire de signaux pour géré l'arrêt du programme
    struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask ), "ecritureMemoire: sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGINT, &newAction, NULL), "ecritureMemoire: sigaction (SIGINT)");
    CHECK(sigaction(SIGTERM, &newAction, NULL), "ecritureMemoire: sigaction (SIGTERM)");
    
   

    // Overture des semaphores
    CHECK_NULL(semReaders = sem_open(SEM_READERS, 0), "ecritureMemoire: sem_open(semReaders)");
    CHECK_NULL(semWriter = sem_open(SEM_WRITER, 0), "ecritureMemoire: sem_open(semWriter)");
    CHECK_NULL(semNewFrame = sem_open(SEM_NEW_FRAME, 0), "ecritureMemoire: sem_open(semNewFrame)");
    CHECK_NULL(semActiveReaders = sem_open(SEM_ACTIVE_READERS, 0), "ecritureMemoire: sem_open(semActiveReaders)");

    // Ouvrir la mémoire partagée
    CHECK(shm_fd = shm_open(SHM_IMAGE, O_CREAT | O_RDWR, 0666), "ecritureMemoire: shm_open(SHM_IMAGE)");
    CHECK(ftruncate(shm_fd, SHM_FRAME_SIZE), "ecritureMemoire: ftruncate(shm_fd)");   
    CHECK_NULL(virtAddr = mmap(0, SHM_FRAME_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0), "ecritureMemoire: mmap(virtAddr)");
    
    

  
    // Capture vidéo
    // Ouvrir la capture vidéo avec l'URL
    cv::VideoCapture cap(argv[1]);
    DEBUG_PRINT("Debut écriture mémoire sur le lien : %s\n", argv[1]);

    // Vérifier si la capture est ouverte
    if (!cap.isOpened()) {
        std::cerr << "Erreur: Impossible d'ouvrir le flux vidéo." << std::endl;
        exit(EXIT_FAILURE);
    }

    cap.set(cv::CAP_PROP_FPS, 25);           // Définir les FPS à 25
    cap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);  // Largeur à 1280
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);  // Hauteur à 720

    cv::Mat frame;
    
    while (ecritureEnCours) {
        
        if (!cap.read(frame)) {
            std::cerr << "Erreur: Impossible de capturer une image." << std::endl;
            break;
        }

        cv::rotate(frame, frame, cv::ROTATE_90_COUNTERCLOCKWISE);

        sem_wait(semWriter);  // Bloquer l'accès en écriture

        // Copier les données dans la mémoire partagée
        std::memcpy(virtAddr, frame.data, SHM_FRAME_SIZE);

        sem_post(semWriter);  // Débloquer l'accès en écriture

        
        sem_getvalue(semActiveReaders, &readers);

        for (int i = 0; i < readers; i++) {
            sem_post(semNewFrame);
        }
    }

    cap.release();
    munmap(virtAddr, SHM_FRAME_SIZE);
    close(shm_fd);
    shm_unlink(SHM_IMAGE);

    // Fermer les sémaphores
    sem_close(semReaders);
    sem_close(semWriter);
    sem_close(semNewFrame);
    sem_close(semActiveReaders);

    exit(EXIT_SUCCESS);

    return 0;
}


static void signalHandler(int numSig)
{ 
    switch(numSig) {
        case SIGINT : // traitement de SIGINT
            DEBUG_PRINT("\t[%d] --> Interruption du programme d'écriture en cours...\n", getpid());
            //ecritureEnCours = false;
            exit(EXIT_FAILURE);
            break;
        case SIGTERM : // traitement de SIGTERM
            DEBUG_PRINT("\t[%d] --> Arrêt du programme d'écriture en cours...\n", getpid());
            ecritureEnCours = false;
            break;
        default :
            printf (" Signal %d non traité \n", numSig );
            break ;
    }
}
