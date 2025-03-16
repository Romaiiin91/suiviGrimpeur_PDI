/******************************************************************************
 * Name        : detection.cpp
 * Description : Programme to perform video detection and control PTZ cameras.
 * Author      : Romain
 * Date        : 16/03/2025
 * Version     : 1.0
 *
 * This programme processes video frames to detect movements and controls
 * PTZ (Pan-Tilt-Zoom) cameras accordingly. It uses shared memory and semaphores
 * for inter-process communication and synchronisation.
 *
 * Compilation : make bin/detection
 *
 * Usage       : This programme is executed to monitor video streams and adjust
 *               camera positions based on detected movements.
 *
 * Notes       : 
 * - The programme uses OpenCV for image processing.
 * - Shared memory and semaphores are used for synchronisation.
 * - Signal handling is implemented for graceful termination.
 * 
 * - Need to be refactored and separated into functions.
 *
 ******************************************************************************/

/* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */

#include <detection.hpp>
#include <csignal>
#include <thread>
#include <chrono>

/* ------------------------------------------------------------------------ */
/*                 V A R I A B L E S    G L O B A L E S                     */
/* ------------------------------------------------------------------------ */


bool detectionEnCours = true;
int exitStatus = -1;

/* ------------------------------------------------------------------------ */
/*            D E F I N I T I O N   D E    F O N C T I O N S                */
/* ------------------------------------------------------------------------ */


size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output){
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

float getActualZoom(const camera_t* cameraActive){
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    float zoom = 1.0;

    // Url
    std::string url = ENTETE_HTTP + std::string("@") + cameraActive->ip + "/" + SCRIPT_PTZ + "?query=position";
    

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); // Timeout de 5 secondes

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Erreur curl: " << curl_easy_strerror(res) << std::endl;
            readBuffer.clear(); // Retourne une chaîne vide en cas d'erreur
        }
        else{
            std::string zoomStr;
            zoomStr = readBuffer.substr(readBuffer.find("zoom=") + 5, 4);
            zoom = std::stof(zoomStr)/588 + 1; // min de zoom Camera = 1 et max = 9999 et zoomHumain = 0.1 à 18
        }

        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Erreur: Impossible d'initialiser CURL" << std::endl;
    }

    return MAX(zoom,1.0);
}

static void signalHandler(int numSig)
{
    switch (numSig)
    {
    case SIGTERM: // traitement de SIGINT
        DEBUG_PRINT("\t[%d] --> Arrêt du programme de detection en cours...\n", getpid());
        detectionEnCours = false;
        exitStatus = 0;
        break;
    case SIGINT:
        DEBUG_PRINT("\t[%d] --> Interruption du programme de detection en cours...\n", getpid());
        exit(EXIT_FAILURE);
        break;
    default:
        printf(" Signal %d non traité \n", numSig);
        break;
    }
}


int main(int argc, char const *argv[])
{
    DEBUG_PRINT("[%d] Démarrage de detection video\n", getpid());

    // Installation du gestionnaire de signaux pour géré l'arrêt du programme
    struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask), "detectionVideo: sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGINT, &newAction, NULL), "detectionVideo: sigaction (SIGINT)");
    CHECK(sigaction(SIGTERM, &newAction, NULL), "detectionVideo: sigaction (SIGTERM)");


    // Creation de la structure cameraActive
    camera_t cameraActive;

    json_error_t error;
    json_t *root = NULL;

    CHECK_NULL(root = json_load_file(PATH_CAMERA_ACTIVE, 0, &error), "detectionVideo: json_load_file(PATH_CAMERA_ACTIVE)");
    cameraActive.id = json_integer_value(json_object_get(root, "id"));
    strcpy(cameraActive.ip, json_string_value(json_object_get(root, "IP")));
    cameraActive.width = json_integer_value(json_object_get(root, "width"));
    cameraActive.height = json_integer_value(json_object_get(root, "height"));
    cameraActive.orientation = json_integer_value(json_object_get(root, "orientation"));
    cameraActive.up = json_integer_value(json_object_get(root, "up"));
    cameraActive.down = json_integer_value(json_object_get(root, "down"));
    cameraActive.left = json_integer_value(json_object_get(root, "left"));
    cameraActive.right = json_integer_value(json_object_get(root, "right"));
    strcpy(cameraActive.cmdVertical, json_string_value(json_object_get(root, "cmdVertical")));
    strcpy(cameraActive.cmdHorizontal, json_string_value(json_object_get(root, "cmdHorizontal")));
    
    json_decref(root); 
    
    DEBUG_PRINT("deteciton.cpp : cameraActive : \n\t id : %d\n\t ip : %s\n\t width : %d\n\t height : %d\n\t orientation : %d\n\t up : %d\n\t down : %d\n\t left : %d\n\t right : %d\n\t cmdVertical : %s\n\t cmdHorizontal : %s\n", cameraActive.id, cameraActive.ip, cameraActive.width, cameraActive.height, cameraActive.orientation, cameraActive.up, cameraActive.down, cameraActive.left, cameraActive.right, cameraActive.cmdVertical, cameraActive.cmdHorizontal); 



    // Charger les paramètres depuis le fichier JSON
    CHECK_NULL(root = json_load_file(PATH_PARAM_DETECTION, 0, &error), "detection.cpp: json_load_file(PATH_PARAM_DETECTION)");

    int framesBetweenReference = json_integer_value(json_object_get(root, "framesBetweenReferences"));
    float verticalThreshold = json_real_value(json_object_get(root, "verticalThreshold"));
    float horizontalThreshold = json_real_value(json_object_get(root, "horizontalThreshold"));

    float coefAverageMovingFilter = json_real_value(json_object_get(root, "coefAverageMovingFilter"));
    int coefGaussianBlur = json_integer_value(json_object_get(root, "coefGaussianBlur"));
    int heightResizeRatio = json_integer_value(json_object_get(root, "heightResizeRatio"));
    int widthResizeRatio = json_integer_value(json_object_get(root, "widthResizeRatio"));
    int nbMoveBeforeChangeDetectionArea = json_integer_value(json_object_get(root, "nbMoveBeforeChangeDetectionArea"));

    
    int cropRatioDetectionArea;
    if (cameraActive.orientation == 0 || cameraActive.orientation == 180) cropRatioDetectionArea = json_integer_value(json_object_get(root, "cropRatioDetectionAreaLandscape"));
    else cropRatioDetectionArea = json_integer_value(json_object_get(root, "cropRatioDetectionAreaPortrait"));
    

    int numberFrameBetweenMove = json_integer_value(json_object_get(root, "numberFrameBetweenMove"));
    int numberFrameWithoutMove = json_integer_value(json_object_get(root, "numberFrameWithoutMove"));

    float increasePTZ = json_real_value(json_object_get(root, "increasePTZ"));

    json_decref(root);
    DEBUG_PRINT("detection.cpp: ParamDetection chargés\n");
    

    

    


    // Calcul des paramètres de detection
    int widthResize = cameraActive.width / widthResizeRatio;
    int heightResize = cameraActive.height / heightResizeRatio;

    int widthDetectionArea = (cropRatioDetectionArea - 2) * widthResize / cropRatioDetectionArea;
    int heightDetectionArea = (cropRatioDetectionArea - 2) * heightResize / cropRatioDetectionArea;

    float floatHeightDetectionArea = heightDetectionArea * 1.0;
    float floatWidthDetectionArea = widthDetectionArea * 1.0;



    // Reset file from key frames detection
    FILE *fvideo;
    CHECK_NULL(fvideo = fopen(PATH_FRAMES, "w"), "fopen(PATH_FRAMES)");
    fprintf(fvideo, "0\n"); // Reset du fichier
    fflush(fvideo);
    fclose(fvideo);



    int shm_fd;
    void *virtAddr;
    sem_t *semReaders, *semWriter, *semMutex, *semNewFrame, *semActiveReaders;
    int reader_count;

    // Overture des semaphores
    CHECK_NULL(semReaders = sem_open(SEM_READERS, 0), "video.cgi: sem_open(semReaders)");
    CHECK_NULL(semWriter = sem_open(SEM_WRITER, 0), "video.cgi: sem_open(semWriter)");
    CHECK_NULL(semMutex = sem_open(SEM_MUTEX, 0), "video.cgi: sem_open(semMutex)");
    CHECK_NULL(semNewFrame = sem_open(SEM_NEW_FRAME, 0), "video.cgi: sem_open(semNewFrame)");
    CHECK_NULL(semActiveReaders = sem_open(SEM_ACTIVE_READERS, 0), "video.cgi: sem_open(semActiveReaders)");

    // Incrémenter le compteur de lecteurs actifs
    sem_post(semActiveReaders);

    // Ouvrir la mémoire partagée
    CHECK(shm_fd = shm_open(SHM_IMAGE, O_CREAT | O_RDWR, 0666), "video.cgi: shm_open(SHM_IMAGE)");
    CHECK(ftruncate(shm_fd, SHM_FRAME_SIZE), "video.cgi: ftruncate(shm_fd)");
    CHECK_NULL(virtAddr = mmap(0, SHM_FRAME_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0), "video.cgi: mmap(virtAddr)");

    // Get actual zoom value
    float actualZoom = getActualZoom(&cameraActive);
    DEBUG_PRINT("Actual zoom : %f\n", actualZoom);

    increasePTZ = increasePTZ / actualZoom;

    // Variables for detection

    int frame_count = 0, resetFrameReference = 0;
    cv::Moments m, mDetectionArea;
    double cX_detectionArea = widthResize / 2, cY_detectionArea = heightResize / 2;

    std::string mvmtVert = "", mvmtHoriz = "";
    cv::Mat frame, frameDelta, thresh, frameReference, gray, detectionArea;

    int lastFrameMoveCam = 0;
    int lastFrameWithMovement = 0;

    int nbMoveUp = 0;
    int frameFirstMove = 0;

    int abscisseDetectionAreaTop = 0, abscisseDetectionAreaBottom = 0;

    cv::RotateFlags rotateCode;
    switch (cameraActive.orientation)
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

    while (detectionEnCours && frame_count < INT_MAX)
    {

        sem_wait(semNewFrame); // Attendre une nouvelle image

        sem_wait(semMutex); // Acquérir le sémaphore de synchronisation

        // Incrémenter le nombre de lecteurs

        sem_getvalue(semReaders, &reader_count);
        if (reader_count == 0)
        {
            sem_wait(semWriter); // Bloquer le rédacteur si c'est le premier lecteur
        }
        sem_post(semReaders); // Incrémenter le compteur de lecteurs

        sem_post(semMutex); // Relâcher le sémaphore de synchronisation

        // Lire les données de la mémoire partagée
        frame = cv::Mat(HEIGHT, WIDTH, CV_8UC3, virtAddr);

        sem_wait(semMutex); // Acquérir le sémaphore de synchronisation

        // Décrémenter le nombre de lecteurs
        sem_wait(semReaders); // Décrémenter le compteur de lecteurs
        sem_getvalue(semReaders, &reader_count);
        if (reader_count == 0)
        {
            sem_post(semWriter); // Débloquer le rédacteur si c'est le dernier lecteur
        }

        sem_post(semMutex); // Relâcher le sémaphore de synchronisation

        if (cameraActive.orientation != 0) cv::rotate(frame, frame, rotateCode);


        // First frame processing to encode frame in a true / false image
        cv::resize(frame, frame, cv::Size(widthResize, heightResize));
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);
        cv::GaussianBlur(gray, gray, cv::Size(coefGaussianBlur, coefGaussianBlur), 0);

        // Remettre à zéro l'image de référence toutes les x images ou lors d'un mouvement de camera
        if (frame_count % framesBetweenReference == 0 || resetFrameReference == 1)
        {
            frameReference = gray.clone();
            resetFrameReference = 0;
        }

        // cv::imshow("Frame Reference", frameReference);

        cv::absdiff(frameReference, gray, thresh);
        cv::threshold(thresh, thresh, 25, 255, cv::THRESH_BINARY);
        cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 2);

        // Forcer le reset de l'image de reference si trop de pixel blanc
        if (cv::countNonZero(thresh) > 30000) resetFrameReference = 1;
        else
        {
            // Definition of detection area

            if (nbMoveUp < nbMoveBeforeChangeDetectionArea)
            {
                abscisseDetectionAreaTop = 0;
                abscisseDetectionAreaBottom = heightResize / 2;
            }
            else
            {
                abscisseDetectionAreaTop = heightResize / cropRatioDetectionArea;
                abscisseDetectionAreaBottom = heightResize - heightResize / cropRatioDetectionArea;
            }

            detectionArea = thresh.rowRange(abscisseDetectionAreaTop, abscisseDetectionAreaBottom);
            detectionArea = detectionArea.colRange(widthResize / cropRatioDetectionArea, widthResize - widthResize / cropRatioDetectionArea);

            // Ensure that the detection area is not empty
            if (cv::countNonZero(detectionArea) > 1000)
            {
                lastFrameWithMovement = frame_count;

                mDetectionArea = cv::moments(detectionArea, true);

                // Get the baricenter of the detection area with a moving average filter
                cX_detectionArea = coefAverageMovingFilter * ((mDetectionArea.m00 > 1) ? mDetectionArea.m10 / mDetectionArea.m00 : cX_detectionArea) + (1 - coefAverageMovingFilter) * cX_detectionArea;
                cY_detectionArea = coefAverageMovingFilter * ((mDetectionArea.m00 > 1) ? mDetectionArea.m01 / mDetectionArea.m00 : cY_detectionArea) + (1 - coefAverageMovingFilter) * cY_detectionArea;

                /*------------------------------------------------------------------------ */
                /*                      D E T E C T I O N   D U   M O U V E M E N T        */
                /*------------------------------------------------------------------------ */

                // premier if equivalent attente entre chaque mouvement
                if (frame_count - lastFrameMoveCam > numberFrameBetweenMove)
                {

                    // std::cout << "cy detection area : " << cY_detectionArea << " heightResize / verticalThreshold : " << heightResize / verticalThreshold << std::endl;

                    // Detection up movement
                    if (cY_detectionArea < floatHeightDetectionArea / verticalThreshold)
                    {
                        // std::cout << "cy detection area : " << cY_detectionArea << " heightResize / verticalThreshold : " << heightResize / verticalThreshold << std::endl;
                        mvmtVert = "Monte";

                        requetePTZ("up", increasePTZ, &cameraActive);
                        lastFrameMoveCam = frame_count;
                        resetFrameReference = 1;

                        if (nbMoveUp == 0)
                        {
                            frameFirstMove = frame_count;
                            // std::cout << "frame first move : " << frameFirstMove << std::endl;
                        }

                        ++nbMoveUp;
                    } 
                    else mvmtVert = "";

                    // Detection horizontal movement
                    if (cX_detectionArea < floatWidthDetectionArea / horizontalThreshold)
                    {
                        mvmtHoriz = "Gauche";
                        requetePTZ("left", increasePTZ, &cameraActive);
                        lastFrameMoveCam = frame_count;
                        resetFrameReference = 1;
                       
                    }

                    else if (cX_detectionArea > floatWidthDetectionArea - floatWidthDetectionArea / horizontalThreshold)
                    {
                        mvmtHoriz = "Droite";
                        requetePTZ("right", increasePTZ, &cameraActive);
                        lastFrameMoveCam = frame_count;
                        resetFrameReference = 1;
                    }
                    else mvmtHoriz = "";
                    
                }
            }
        }

        if ((frame_count - lastFrameWithMovement) > numberFrameWithoutMove && nbMoveUp > 0)
        {
            detectionEnCours = false;
            exitStatus = 1;
        }

        // If the program is build on a pc with a screen, display the detection area and frames
#ifdef PC

        // Afficher le barycentre sur l'image
        cv::circle(frame, cv::Point(cX_detectionArea + widthResize / cropRatioDetectionArea, cY_detectionArea + abscisseDetectionAreaTop), 5, cv::Scalar(0, 255, 0), -1);

        // Afficher du nb de point blanc sur la moitie haute de l'image sur l'image
        cv::putText(thresh, (std::string) "nb px blancs Area : " + std::to_string(cv::countNonZero(detectionArea)), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);

        cv::putText(frame, mvmtVert, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);
        cv::putText(frame, mvmtHoriz, cv::Point(500, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);
        cv::putText(frame, std::to_string(frame_count - lastFrameWithMovement), cv::Point(10, 350), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);

        // Afficher l'image
        // cv::imshow("Thresh", thresh);

        cv::rectangle(frame, cv::Point(widthResize / cropRatioDetectionArea, abscisseDetectionAreaTop), cv::Point(widthResize - widthResize / cropRatioDetectionArea, abscisseDetectionAreaBottom), cv::Scalar(0, 255, 0), 2);

        cv::line(frame, cv::Point(0, heightDetectionArea / verticalThreshold + abscisseDetectionAreaTop), cv::Point(widthResize, heightDetectionArea / verticalThreshold + abscisseDetectionAreaTop), cv::Scalar(0, 0, 255), 2);

        cv::line(frame, cv::Point(0, heightDetectionArea - heightDetectionArea / verticalThreshold + abscisseDetectionAreaTop), cv::Point(widthResize, heightDetectionArea - heightDetectionArea / verticalThreshold + abscisseDetectionAreaTop), cv::Scalar(0, 0, 255), 2);

        cv::line(frame, cv::Point(widthResize / cropRatioDetectionArea + widthDetectionArea / horizontalThreshold, 0), cv::Point(widthResize / cropRatioDetectionArea + widthDetectionArea / horizontalThreshold, heightResize), cv::Scalar(0, 0, 255), 2);

        cv::line(frame, cv::Point(widthResize / cropRatioDetectionArea + widthDetectionArea - widthDetectionArea / horizontalThreshold, 0), cv::Point(widthResize / cropRatioDetectionArea + widthDetectionArea - widthDetectionArea / horizontalThreshold, heightResize), cv::Scalar(0, 0, 255), 2);

        // cv::imshow("Detection Area ", detectionArea);

        cv::imshow("Barycentre", frame);

#endif

        frame_count++;


        if (cv::waitKey(5) == 'q')
        { // en attente du semaphore
            detectionEnCours = false;
            exitStatus = 0;
        }

    }


    // Décrémenter le compteur de lecteurs actifs à la fin de l'exécution du lecteur
    sem_wait(semActiveReaders);

    // Fermer les ressources
    munmap(virtAddr, SHM_FRAME_SIZE);
    close(shm_fd);

    // Fermer les sémaphores
    sem_close(semReaders);
    sem_close(semWriter);
    sem_close(semMutex);
    sem_close(semNewFrame);
    sem_close(semActiveReaders);

    cv::destroyAllWindows();

    DEBUG_PRINT("Fin de la détection.\n");
    DEBUG_PRINT("FrameFirstMove : %d\n", frameFirstMove);
    DEBUG_PRINT("FrameCount : %d\n", frame_count);

    CHECK_NULL(fvideo = fopen(PATH_FRAMES, "w"), "detection: fopen(PATH_FRAMES)");
    fprintf(fvideo, "%d\n%d\n", frameFirstMove, frame_count);
    fflush(fvideo);
    fclose(fvideo);

    exit(exitStatus);
    return 0;
}

