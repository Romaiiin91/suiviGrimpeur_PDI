#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <jpeglib.h>
#include <string.h>
#include <opencv2/opencv.hpp>

#include <utils.h>

#ifdef DEBUG
    #define DEBUG_CGI_PRINT(msg, ...) do {                                      \
        FILE *log_file;                                                     \
        CHECK_NULL(log_file = fopen("../debugCgi.log", "a"), "fopen(debug.log)"); \
        fprintf(log_file, msg, ##__VA_ARGS__);                              \
        fflush(log_file);                                                   \
        fclose(log_file);                                                   \
    } while (0)
#else
    #define DEBUG_CGI_PRINT(msg, ...) // Ne fait rien si DEBUG n'est pas défini
#endif

// Fonction pour encoder une image en JPEG
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

int main() {
    DEBUG_CGI_PRINT("[%d] Démarrage du cgi video\n", getpid());

    int shm_fd;
    void* virtAddr;
    sem_t *semReaders, *semWriter, *semMutex;
    int reader_count;

    cv::Mat frame(HEIGHT, WIDTH, CV_8UC3);
    cv::Mat frameRGB;   
    unsigned char* jpeg_data = NULL;
    unsigned long jpeg_size = 0;

    // Overture des semaphores
    CHECK_NULL(semReaders = sem_open(SEM_READERS, 0), "video.cgi: sem_open(semReaders)");
    CHECK_NULL(semWriter = sem_open(SEM_WRITER, 0), "video.cgi: sem_open(semWriter)");
    CHECK_NULL(semMutex = sem_open(SEM_MUTEX, 0), "video.cgi: sem_open(semMutex)");

    // Ouvrir la mémoire partagée
    CHECK(shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666), "video.cgi: shm_open(SHM_NAME)");
    CHECK(ftruncate(shm_fd, SHM_FRAME_SIZE), "video.cgi: ftruncate(shm_fd)");   
    CHECK_NULL(virtAddr = mmap(0, SHM_FRAME_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0), "video.cgi: mmap(virtAddr)");

    // En-tête HTTP pour le flux MJPEG
    printf("Content-Type: multipart/x-mixed-replace; boundary=--myboundary\r\n\r\n");

 

    // Boucle pour envoyer les images en continu
    while (true) {

        sem_wait(semMutex);  // Acquérir le sémaphore de synchronisation
 
        // Incrémenter le nombre de lecteurs
        
        sem_getvalue(semReaders, &reader_count);
        if (reader_count == 0) {
            sem_wait(semWriter);  // Bloquer le rédacteur si c'est le premier lecteur
        }
        sem_post(semReaders);  // Incrémenter le compteur de lecteurs

        sem_post(semMutex);  // Relâcher le sémaphore de synchronisation



        // Lire les données de la mémoire partagée
        frame = cv::Mat(HEIGHT, WIDTH, CV_8UC3, virtAddr);
       

        sem_wait(semMutex);  // Acquérir le sémaphore de synchronisation

        // Décrémenter le nombre de lecteurs
        sem_wait(semReaders);  // Décrémenter le compteur de lecteurs
        sem_getvalue(semReaders, &reader_count);
        if (reader_count == 0) {
            sem_post(semWriter);  // Débloquer le rédacteur si c'est le dernier lecteur
        }

        sem_post(semMutex);  // Relâcher le sémaphore de synchronisation
        
        //cv::imshow("Frame", frame);

        
        


        // Convertir de BGR à RGB
        cv::cvtColor(frame, frameRGB, cv::COLOR_BGR2RGB);

        // Encoder l'image en JPEG
        encode_jpeg(frameRGB.data, WIDTH, HEIGHT, CHANNELS, &jpeg_data, &jpeg_size);

        // Envoyer l'en-tête de la partie
        printf("--myboundary\r\n");
        printf("Content-Type: image/jpeg\r\n");
        printf("Content-Length: %lu\r\n\r\n", jpeg_size);

        // Envoyer les données JPEG
        fwrite(jpeg_data, 1, jpeg_size, stdout);
        fflush(stdout); // Forcer l'envoi des données
        // Libérer la mémoire allouée pour les données JPEG
        free(jpeg_data);
        jpeg_data = NULL; // Réinitialiser le pointeur pour éviter les accès invalides

        usleep(39000); // 25 FPS
    }

 

    // Fermer les ressources
    munmap(virtAddr, SHM_FRAME_SIZE);
    close(shm_fd);

    // Fermer les sémaphores
    sem_close(semReaders);
    sem_close(semWriter);
    sem_close(semMutex);

    
    exit(EXIT_SUCCESS);
    return 0;
}