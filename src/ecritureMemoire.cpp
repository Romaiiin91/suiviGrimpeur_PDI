/******************************************************************************
 * Name        : ecritureMemoire.cpp
 * Description : Program to write video frames into shared memory.
 * Author      : Romain
 * Date        : 16/03/2025
 * Version     : 1.0
 *
 * This program captures video frames from a given source and writes them
 * into shared memory for inter-process communication. It uses semaphores
 * for synchronization and signal handling for graceful termination.
 *
 * Compilation : make bin/ecritureMemoire
 *
 * Usage       : ./ecritureMemoire <video_source>
 *
 * Notes       : 
 * - The program uses OpenCV for video capture and processing.
 * - Shared memory and semaphores are used for synchronization.
 * - Signal handling is implemented for graceful termination.
 *
 ******************************************************************************/

#include <opencv2/opencv.hpp>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include <utils.h>

static void signalHandler(int numSig);

bool ecritureEnCours = true;

int main(int argc, char * argv[]) {
    DEBUG_PRINT("[%d] Starting memory writing\n", getpid());

    if (argc < 2) {
        DEBUG_PRINT("Not enough arguments\n");
        exit(EXIT_FAILURE);
    }

    int shm_fd;
    void* virtAddr;
    sem_t *semReaders, *semWriter, *semNewFrame, *semActiveReaders;
    int readers;

    // Install the signal handler to manage program termination
    struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask ), "ecritureMemoire: sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGINT, &newAction, NULL), "ecritureMemoire: sigaction (SIGINT)");
    CHECK(sigaction(SIGTERM, &newAction, NULL), "ecritureMemoire: sigaction (SIGTERM)");

    // Open semaphores
    CHECK_NULL(semReaders = sem_open(SEM_READERS, 0), "ecritureMemoire: sem_open(semReaders)");
    CHECK_NULL(semWriter = sem_open(SEM_WRITER, 0), "ecritureMemoire: sem_open(semWriter)");
    CHECK_NULL(semNewFrame = sem_open(SEM_NEW_FRAME, 0), "ecritureMemoire: sem_open(semNewFrame)");
    CHECK_NULL(semActiveReaders = sem_open(SEM_ACTIVE_READERS, 0), "ecritureMemoire: sem_open(semActiveReaders)");

    // Open shared memory
    CHECK(shm_fd = shm_open(SHM_IMAGE, O_RDWR, 0666), "ecritureMemoire: shm_open(SHM_IMAGE)");
    CHECK(ftruncate(shm_fd, SHM_FRAME_SIZE), "ecritureMemoire: ftruncate(shm_fd)");   
    CHECK_NULL(virtAddr = mmap(0, SHM_FRAME_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0), "ecritureMemoire: mmap(virtAddr)");

    // Video capture
    // Open video capture with the URL
    cv::VideoCapture cap(argv[1]);
    DEBUG_PRINT("Starting memory writing on the link: %s\n", argv[1]);

    // Check if the capture is open
    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to open the video stream." << std::endl;
        exit(EXIT_FAILURE);
    }

    cap.set(cv::CAP_PROP_FPS, 25);           // Set FPS to 25
    cap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);  // Set width to 1280
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);  // Set height to 720

    cv::Mat frame;

    while (ecritureEnCours) {
        if (!cap.read(frame)) {
            std::cerr << "Error: Unable to capture an image." << std::endl;
            break;
        }

        sem_wait(semWriter);  // Block write access

        // Copy data into shared memory
        std::memcpy(virtAddr, frame.data, SHM_FRAME_SIZE);

        sem_post(semWriter);  // Unblock write access

        sem_getvalue(semActiveReaders, &readers);

        for (int i = 0; i < readers; i++) {
            sem_post(semNewFrame);
        }
    }

    cap.release();
    if (virtAddr != MAP_FAILED) {
        munmap(virtAddr, SHM_ORDRE_SIZE);
    }
    if (shm_fd != -1) {
        close(shm_fd);
    }

    // Close semaphores
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
        case SIGINT: // Handle SIGINT
            DEBUG_PRINT("\t[%d] --> Interrupting the memory writing program...\n", getpid());
            //ecritureEnCours = false;
            exit(EXIT_FAILURE);
            break;
        case SIGTERM: // Handle SIGTERM
            DEBUG_PRINT("\t[%d] --> Stopping the memory writing program...\n", getpid());
            ecritureEnCours = false;
            break;
        default:
            printf(" Signal %d not handled \n", numSig);
            break;
    }
}
