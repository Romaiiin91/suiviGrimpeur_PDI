/******************************************************************************
 * Name        : video.cpp
 * Description : CGI programme to stream video frames as MJPEG over HTTP.
 * Author      : Romain
 * Date        : 16/03/2025
 * Version     : 1.0
 *
 * This programme reads video frames from shared memory, encodes them as JPEG,
 * and streams them as MJPEG over HTTP. It uses semaphores for synchronisation
 * and signal handling for graceful termination.
 *
 * Compilation : make bin/video
 *
 * Usage       : ./video
 *
 * Notes       : 
 * - The programme uses OpenCV for image processing.
 * - Shared memory and semaphores are used for synchronisation.
 * - Signal handling is implemented for graceful termination.
 *
 ******************************************************************************/

 /* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <jpeglib.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <semaphore.h>
#include <utilsCgi.h>

/* ------------------------------------------------------------------------ */
/*            P R O T O T Y P E S    D E    F O N C T I O N S               */
/* ------------------------------------------------------------------------ */

/**
 * @brief Encode an image in JPEG format
 * 
 * @param image_data Raw image data
 * @param width Image width
 * @param height Image height
 * @param channels Number of image channels (e.g., 3 for RGB)
 * @param jpeg_data Pointer to the encoded JPEG data
 * @param jpeg_size Size of the encoded JPEG data
 */
void encode_jpeg(unsigned char* image_data, int width, int height, int channels, unsigned char** jpeg_data, unsigned long* jpeg_size);

/**
 * @brief Signal handler for graceful termination
 * 
 * @param numSig Signal number received
 */
static void signalHandler(int numSig);

/**
 * @brief Main program to stream video frames in MJPEG
 * 
 * @return int 0 on success, -1 on error
 */
int main();

/* ------------------------------------------------------------------------ */
/*                 V A R I A B L E S    G L O B A L E S                     */
/* ------------------------------------------------------------------------ */

bool affichageEnCours = true;

/* ------------------------------------------------------------------------ */
/*            D E F I N I T I O N   D E    F O N C T I O N S                */
/* ------------------------------------------------------------------------ */

void encode_jpeg(unsigned char* image_data, int width, int height, int channels, unsigned char** jpeg_data, unsigned long* jpeg_size) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    // Initialisation de la structure JPEG
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    // Spécifier la sortie (mémoire)
    jpeg_mem_dest(&cinfo, jpeg_data, jpeg_size);

    // Paramètres de l'image
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = channels;
    cinfo.in_color_space = JCS_RGB;

    // Définir les paramètres par défaut
    jpeg_set_defaults(&cinfo);

    // Définir la qualité (0-100)
    jpeg_set_quality(&cinfo, 75, TRUE);

    // Démarrer la compression
    jpeg_start_compress(&cinfo, TRUE);

    // Écrire les lignes de l'image
    JSAMPROW row_pointer[1];
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &image_data[cinfo.next_scanline * width * channels];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // Finaliser la compression
    jpeg_finish_compress(&cinfo);

    // Nettoyer
    jpeg_destroy_compress(&cinfo);
}


static void signalHandler(int numSig)
{
    switch (numSig)
    {
    case SIGTERM: 
        DEBUG_CGI_PRINT("\t[%d] --> Arrêt du cgi video en cours...\n", getpid());
        affichageEnCours = false;
        break;
    case SIGINT:
        DEBUG_CGI_PRINT("\t[%d] --> Interruption du cgi video en cours...\n", getpid());
        break;
    
    default:
        DEBUG_CGI_PRINT(" Signal %d non traité \n", numSig);
        break;
    }
}


int main() {
    DEBUG_CGI_PRINT("[%d] Démarrage du cgi video\n", getpid());

    int shm_fd;
    void* virtAddr;
    sem_t *semReaders, *semWriter, *semMutex, *semNewFrame, *semActiveReaders;
    int reader_count;

    // Recuperer les dimensions de l'image depuis le fichier cameraActive.json
    json_error_t error;
    json_t *root = json_load_file(PATH_CAMERA_ACTIVE, 0, &error);

    CHECK_NULL(root, "video.cgi: json_load_file(PATH_CAMERA_ACTIVE)");
    int width = json_integer_value(json_object_get(root, "width"));
    int height = json_integer_value(json_object_get(root, "height"));
    int orientation = json_integer_value(json_object_get(root, "orientation"));
    json_decref(root);


    cv::Mat frame;
    cv::Mat frameRGB;   

    unsigned long jpeg_size = 0;
    unsigned char* jpeg_buffer; // Taille maximale possible
    CHECK_NULL(jpeg_buffer = (unsigned char*) malloc(FRAME_SIZE), "video.cgi: malloc(jpeg_buffer)");

    // Installation du gestionnaire de signal
    struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask ), " sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGINT, &newAction, NULL), "sigaction (SIGINT)");
    CHECK(sigaction(SIGTERM, &newAction, NULL), "sigaction (SIGTERM)");
    
    

    // Overture des semaphores
    CHECK_NULL(semReaders = sem_open(SEM_READERS, 0), "video.cgi: sem_open(semReaders)");
    CHECK_NULL(semWriter = sem_open(SEM_WRITER, 0), "video.cgi: sem_open(semWriter)");
    CHECK_NULL(semMutex = sem_open(SEM_MUTEX, 0), "video.cgi: sem_open(semMutex)");
    CHECK_NULL(semNewFrame = sem_open(SEM_NEW_FRAME, 0), "video.cgi: sem_open(semNewFrame)");
    CHECK_NULL(semActiveReaders = sem_open(SEM_ACTIVE_READERS, 0), "video.cgi: sem_open(semActiveReaders)");

    // Ouvrir la mémoire partagée
    CHECK(shm_fd = shm_open(SHM_IMAGE, O_CREAT | O_RDWR, 0666), "video.cgi: shm_open(SHM_IMAGE)");
    CHECK(ftruncate(shm_fd, SHM_FRAME_SIZE), "video.cgi: ftruncate(shm_fd)");   
    CHECK_NULL(virtAddr = mmap(0, SHM_FRAME_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0), "video.cgi: mmap(virtAddr)");

    // En-tête HTTP pour le flux MJPEG
    printf("Content-Type: multipart/x-mixed-replace; boundary=--myboundary\r\n\r\n");

    cv::RotateFlags rotateCode;
    switch (orientation)
    {
    case 90:
        rotateCode = cv::ROTATE_90_CLOCKWISE;
        break;
    case 180:
        rotateCode = cv::ROTATE_180;
        break;
    case 270:
        rotateCode = cv::ROTATE_90_COUNTERCLOCKWISE;
        break;
    
    default:
        break;
    }
 
    // Incrémenter le compteur de lecteurs actifs
    sem_post(semActiveReaders);

    // Boucle pour envoyer les images en continu
    while (affichageEnCours) {
        sem_wait(semNewFrame);  // Attendre une nouvelle image

        sem_wait(semMutex);  // Acquérir le sémaphore de synchronisation
 
        // Incrémenter le nombre de lecteurs
        
        sem_getvalue(semReaders, &reader_count);
        if (reader_count == 0) {
            sem_wait(semWriter);  // Bloquer le rédacteur si c'est le premier lecteur
        }
        sem_post(semReaders);  // Incrémenter le compteur de lecteurs

        sem_post(semMutex);  // Relâcher le sémaphore de synchronisation



        // Mettre à jour les données de la matrice frame
        frame = cv::Mat(720, 1280, CV_8UC3, virtAddr);
       

        sem_wait(semMutex);  // Acquérir le sémaphore de synchronisation

        // Décrémenter le nombre de lecteurs
        sem_wait(semReaders);  // Décrémenter le compteur de lecteurs
        sem_getvalue(semReaders, &reader_count);
        if (reader_count == 0) {
            sem_post(semWriter);  // Débloquer le rédacteur si c'est le dernier lecteur
        }

        sem_post(semMutex);  // Relâcher le sémaphore de synchronisation
        
        // Rotation de l'image si nécessaire
        if (orientation != 0) cv::rotate(frame, frame, rotateCode);



        // Convertir de BGR à RGB
        cv::cvtColor(frame, frameRGB, cv::COLOR_BGR2RGB);

        // Encoder l'image en JPEG
        encode_jpeg(frameRGB.data, width, height, 3, &jpeg_buffer, &jpeg_size);

        // Envoyer l'en-tête de la partie
        printf("--myboundary\r\n");
        printf("Content-Type: image/jpeg\r\n");
        printf("Content-Length: %lu\r\n\r\n", jpeg_size);
        
        fwrite(jpeg_buffer, 1, jpeg_size, stdout);
        fflush(stdout); // Forcer l'envoi des données

        // usleep(39000); // 25 FPS
    }

    // Décrémenter le compteur de lecteurs actifs à la fin de l'exécution du lecteur
    sem_wait(semActiveReaders);

    free(jpeg_buffer);
 

    // Fermer les ressources
    munmap(virtAddr, SHM_FRAME_SIZE);
    close(shm_fd);

    // Fermer les sémaphores
    sem_close(semReaders);
    sem_close(semWriter);
    sem_close(semMutex);
    sem_close(semNewFrame);
    sem_close(semActiveReaders);
    
    DEBUG_CGI_PRINT("[%d] Fin du cgi video\n", getpid());

    exit(EXIT_SUCCESS);
    return 0;
}