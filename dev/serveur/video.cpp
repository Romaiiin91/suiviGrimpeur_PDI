#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <jpeglib.h>
#include <string.h>
#include <opencv2/opencv.hpp>

#define SHM_NAME "/shm_image"
#define WIDTH 1280
#define HEIGHT 720
#define CHANNELS 3
#define FRAME_SIZE (WIDTH * HEIGHT * CHANNELS)

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
    // Ouvrir la mémoire partagée
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Mapper la mémoire partagée
    void* ptr = mmap(0, FRAME_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return 1;
    }

    // En-tête HTTP pour le flux MJPEG
    printf("Content-Type: multipart/x-mixed-replace; boundary=--myboundary\r\n\r\n");

    // Boucle pour envoyer les images en continu
    while (1) {
        // Lire les données de la mémoire partagée
        cv::Mat frame(HEIGHT, WIDTH, CV_8UC3, ptr);

        // Convertir de BGR à RGB
        cv::Mat frameRGB;
        cv::cvtColor(frame, frameRGB, cv::COLOR_BGR2RGB);

        // Encoder l'image en JPEG
        unsigned char* jpeg_data = NULL;
        unsigned long jpeg_size = 0;
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

        // Attendre un court instant avant la prochaine image
        usleep(100000); // 100 ms
    }

    // Nettoyer (jamais atteint dans cette boucle infinie)
    munmap(ptr, FRAME_SIZE);
    close(shm_fd);

    return 0;
}