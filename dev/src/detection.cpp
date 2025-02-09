/* ------------------------------------------------------------------------ */
/*                       Suivi grimpeur - Detection                         */
/*                        Auteur : CHEVALIER Romain                         */
/*                            Date : 26-10-2024                             */
/* ------------------------------------------------------------------------ */


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

#define WIDTH 1280
#define HEIGHT 720

bool detectionEnCours = true;
int exitStatus = 0;

/* ------------------------------------------------------------------------ */
/*            D E F I N I T I O N   D E    F O N C T I O N S                */
/* ------------------------------------------------------------------------ */




int main(int argc, char const *argv[])
{
    DEBUG_CGI_PRINT("[%d] Démarrage de detection video\n", getpid());

    

    // Installation du gestionnaire de signaux pour géré l'arrêt du programme
    struct sigaction newAction;
    newAction.sa_handler = signalHandler;
    CHECK(sigemptyset(&newAction.sa_mask ), " sigemptyset ()");
    newAction.sa_flags = 0;
    CHECK(sigaction(SIGINT, &newAction, NULL), "sigaction (SIGINT)");

    // Charger les paramètres depuis le fichier JSON
    json_error_t error;
    json_t *root = json_load_file("src/paramDetection.json", 0, &error);

    if (!root) {
        std::cerr << "Erreur de lecture du fichier JSON: " << error.text << std::endl;
        exit(EXIT_FAILURE);
    }

    int framesBetweenReference = json_integer_value(json_object_get(root, "framesBetweenReferences"));
    float verticalThreshold = json_real_value(json_object_get(root, "verticalThreshold"));
    float horizontalThreshold = json_real_value(json_object_get(root, "horizontalThreshold"));

    float coefAverageMovingFilter = json_real_value(json_object_get(root, "coefAverageMovingFilter"));
    int coefGaussianBlur = json_integer_value(json_object_get(root, "coefGaussianBlur"));
    int heightResize = json_integer_value(json_object_get(root, "heightResize"));
    int widthResize = json_integer_value(json_object_get(root, "widthResize"));
    int nbMoveBeforeChangeDetectionArea = json_integer_value(json_object_get(root, "nbMoveBeforeChangeDetectionArea"));
    
    
    int cropRatioDetectionArea = json_integer_value(json_object_get(root, "cropRatioDetectionArea")); // 3 cetait bien
    
    int numberFrameBetweenMove = json_integer_value(json_object_get(root, "numberFrameBetweenMove"));
    int numberFrameWithoutMove = json_integer_value(json_object_get(root, "numberFrameWithoutMove"));

    float increasePTZ = json_real_value(json_object_get(root, "increasePTZ"));
    
    json_decref(root);

    int widthDetectionArea = (cropRatioDetectionArea - 2) * widthResize/cropRatioDetectionArea;
    int heightDetectionArea = (cropRatioDetectionArea - 2) * heightResize/cropRatioDetectionArea;

    float floatHeightDetectionArea = heightDetectionArea*1.0;
    float floatWidthDetectionArea = widthDetectionArea*1.0;

    FILE *fvideo;
    CHECK_NULL(fvideo = fopen("./bin/frameFirstMove", "w"), "fopen(frameFirstMove)");
    fprintf(fvideo, "0\n"); // Reset du fichier
    fflush(fvideo);
    fclose(fvideo);

    #ifdef VIDEO

    if (argc < 2) {
        DEBUG_PRINT("Pas url video");
        exit(EXIT_FAILURE);
    }

    // Ouvrir la capture vidéo avec l'URL
    cv::VideoCapture cap(argv[1]);
    DEBUG_PRINT("Debut detection sur le lien : %s\n", argv[1]);

    // Vérifier si la capture est ouverte
    if (!cap.isOpened()) {
        std::cerr << "Erreur: Impossible d'ouvrir le flux vidéo." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Forcer les FPS et la résolution
    cap.set(cv::CAP_PROP_FPS, 25);           // Définir les FPS à 25
    cap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);  // Largeur à 1280
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);  // Hauteur à 720

    #else

    int shm_fd;
    void* virtAddr;
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
    CHECK(shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666), "video.cgi: shm_open(SHM_NAME)");
    CHECK(ftruncate(shm_fd, SHM_FRAME_SIZE), "video.cgi: ftruncate(shm_fd)");   
    CHECK_NULL(virtAddr = mmap(0, SHM_FRAME_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0), "video.cgi: mmap(virtAddr)");
    
    #endif
    
    int frame_count = 0, resetFrameReference = 0;
    cv::Moments m, mDetectionArea;
    double cX_detectionArea = widthResize/2, cY_detectionArea = heightResize/2;
   

    std::string mvmtVert = "", mvmtHoriz = "";
    cv::Mat frame, frameDelta, thresh, frameReference, gray, detectionArea;

    int lastFrameMoveCam = 0;
    int lastFrameWithMovement = 0;

    int nbMoveUp = 0;
    int frameFirstMove = 0;
    


    int abscisseDetectionAreaTop = 0, abscisseDetectionAreaBottom = 0;
    
    while (detectionEnCours && frame_count < INT_MAX) {

        #ifdef VIDEO
        // Lire une image
        cap >> frame;

        // Vérifier si l'image est vide (fin du flux)
        if (frame.empty()) {
            std::cerr << "Erreur: Image vide capturée." << std::endl;
            break;
        }

        #else

        sem_wait(semNewFrame);  // Attendre une nouvelle image

        sem_wait(semMutex);  // Acquérir le sémaphore de synchronisation
 
        // Incrémenter le nombre de lecteurs
        
        sem_getvalue(semReaders, &reader_count);
        if (reader_count == 0) {
            sem_wait(semWriter);  // Bloquer le rédacteur si c'est le premier lecteur
        }
        sem_post(semReaders);  // Incrémenter le compteur de lecteurs

        sem_post(semMutex);  // Relâcher le sémaphore de synchronisation



        // Lire les données de la mémoire partagée
        frame  = cv::Mat(HEIGHT, WIDTH, CV_8UC3, virtAddr);
       

        sem_wait(semMutex);  // Acquérir le sémaphore de synchronisation

        // Décrémenter le nombre de lecteurs
        sem_wait(semReaders);  // Décrémenter le compteur de lecteurs
        sem_getvalue(semReaders, &reader_count);
        if (reader_count == 0) {
            sem_post(semWriter);  // Débloquer le rédacteur si c'est le dernier lecteur
        }

        sem_post(semMutex);  // Relâcher le sémaphore de synchronisation
        
        #endif



        cv::resize(frame, frame, cv::Size(widthResize, heightResize));
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray); 
        cv::GaussianBlur(gray, gray, cv::Size(coefGaussianBlur, coefGaussianBlur), 0);
        

        
        // Remettre à zéro l'image de référence toutes les x images ou lors d'un mouvement de camera
        if (frame_count % framesBetweenReference == 0 || resetFrameReference == 1) {
            frameReference = gray.clone();
            resetFrameReference = 0;
        }

        //cv::imshow("Frame Reference", frameReference);


        cv::absdiff(frameReference, gray, thresh);
        cv::threshold(thresh, thresh, 25, 255, cv::THRESH_BINARY);
        cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 2);
         

        // Forcer le reset de l'image de reference si trop de pixel blanc
        if (cv::countNonZero(thresh) > 30000 ) {
            resetFrameReference = 1;
            #ifdef VIDEO
            ++nbMoveUp;
            #endif
        }
        else {
            // Zone de detection
            

            if (nbMoveUp < nbMoveBeforeChangeDetectionArea){
                abscisseDetectionAreaTop = 0;
                abscisseDetectionAreaBottom = heightResize/2;
            }
            else {
                abscisseDetectionAreaTop = heightResize/cropRatioDetectionArea;
                abscisseDetectionAreaBottom = heightResize - heightResize/cropRatioDetectionArea;
            }

            detectionArea = thresh.rowRange(abscisseDetectionAreaTop, abscisseDetectionAreaBottom);
            detectionArea = detectionArea.colRange(widthResize/cropRatioDetectionArea, widthResize -  widthResize/cropRatioDetectionArea);
            
            if (cv::countNonZero(detectionArea) >1000) {
                lastFrameWithMovement = frame_count;

                mDetectionArea = cv::moments(detectionArea, true);
            

                cX_detectionArea = coefAverageMovingFilter * ((mDetectionArea.m00 > 1) ? mDetectionArea.m10 / mDetectionArea.m00 : cX_detectionArea ) + (1-coefAverageMovingFilter) * cX_detectionArea;
                cY_detectionArea = coefAverageMovingFilter * ((mDetectionArea.m00 > 1) ? mDetectionArea.m01 / mDetectionArea.m00 : cY_detectionArea ) + (1-coefAverageMovingFilter) * cY_detectionArea;

                /*------------------------------------------------------------------------ */
                /*                      D E T E C T I O N   D U   M O U V E M E N T        */
                /*------------------------------------------------------------------------ */

                // premier if equivalent attente entre chaque mouvement
                if (frame_count - lastFrameMoveCam > numberFrameBetweenMove){

                    // std::cout << "cy detection area : " << cY_detectionArea << " heightResize / verticalThreshold : " << heightResize / verticalThreshold << std::endl;
                    
                    if (cY_detectionArea < floatHeightDetectionArea / verticalThreshold){
                        // std::cout << "cy detection area : " << cY_detectionArea << " heightResize / verticalThreshold : " << heightResize / verticalThreshold << std::endl;
                        mvmtVert = "Monte";
                        
                        #ifndef VIDEO 
                        requetePTZ("rtilt", std::to_string(increasePTZ).c_str());
                        lastFrameMoveCam = frame_count;
                        resetFrameReference = 1;
                        #endif
    
                        if (nbMoveUp == 0){ 
                            frameFirstMove = frame_count;
                            //std::cout << "frame first move : " << frameFirstMove << std::endl;
                        }

                        ++nbMoveUp;
                    
                    }
                    else {
                        mvmtVert = "";
                    }

                    if (cX_detectionArea < floatWidthDetectionArea / horizontalThreshold){
                        mvmtHoriz= "Gauche";

                        #ifndef VIDEO
                        requetePTZ("rpan", std::to_string(-1*increasePTZ).c_str());
                        lastFrameMoveCam = frame_count;
                        resetFrameReference = 1;
                        #endif
                        
                    }

                    else if (cX_detectionArea > floatWidthDetectionArea - floatWidthDetectionArea / horizontalThreshold){
                        mvmtHoriz = "Droite";

                        #ifndef VIDEO
                        requetePTZ("rpan", std::to_string(increasePTZ).c_str());
                        lastFrameMoveCam = frame_count;
                        resetFrameReference = 1;
                        #endif
                    }
                    else {
                        mvmtHoriz = "";
                    }

                    // Baisser la zone de detc apres le premier ou 2e mvt vers le haut 
                    
                    
                }
            }
        }

        if ((frame_count - lastFrameWithMovement) > numberFrameWithoutMove && nbMoveUp > 0){
            detectionEnCours = false;
            exitStatus = 1;
        }
        
       
        #ifdef DEBUG
        

        // Afficher le barycentre sur l'image
        cv::circle(frame, cv::Point(cX_detectionArea + widthResize/cropRatioDetectionArea, cY_detectionArea + abscisseDetectionAreaTop), 5, cv::Scalar(0, 255, 0), -1);


        // Afficher du nb de point blanc sur la moitie haute de l'image sur l'image
        cv::putText(thresh, (std::string) "nb px blancs Area : " + std::to_string(cv::countNonZero(detectionArea)), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);


        cv::putText(frame, mvmtVert, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);
        cv::putText(frame, mvmtHoriz, cv::Point(500, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);
        cv::putText(frame, std::to_string(frame_count - lastFrameWithMovement) , cv::Point(10, 350), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);


        // Afficher l'image
        //cv::imshow("Thresh", thresh);
        

        cv::rectangle(frame, cv::Point(widthResize/cropRatioDetectionArea, abscisseDetectionAreaTop), cv::Point(widthResize -  widthResize/cropRatioDetectionArea, abscisseDetectionAreaBottom), cv::Scalar(0, 255, 0), 2);
        
        cv::line(frame, cv::Point(0, heightDetectionArea / verticalThreshold + abscisseDetectionAreaTop ), cv::Point(widthResize, heightDetectionArea / verticalThreshold + abscisseDetectionAreaTop), cv::Scalar(0, 0, 255), 2);
        
        cv::line(frame, cv::Point(widthResize/cropRatioDetectionArea + widthDetectionArea / horizontalThreshold, 0), cv::Point(widthResize/cropRatioDetectionArea + widthDetectionArea / horizontalThreshold, heightResize), cv::Scalar(0, 0, 255), 2);

        cv::line(frame, cv::Point(widthResize/cropRatioDetectionArea + widthDetectionArea - widthDetectionArea / horizontalThreshold, 0), cv::Point(widthResize/cropRatioDetectionArea + widthDetectionArea - widthDetectionArea / horizontalThreshold, heightResize), cv::Scalar(0, 0, 255), 2);

        // cv::imshow("Detection Area ", detectionArea);

        cv::imshow("Barycentre", frame); 
        
        #endif

        frame_count++;

        #ifdef VIDEO
        if (cv::waitKey(40) == 'q') { // 25 fps
            detectionEnCours = false;
            exitStatus = 2;
        }
        #else
        if (cv::waitKey(5) == 'q') { // en attente du semaphore
            detectionEnCours = false;
            exitStatus = 2;
        }
        #endif
        
    }

    // Libérer la capture
    #ifdef VIDEO
    cap.release();
    #else
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

    #endif

    cv::destroyAllWindows();
    

    DEBUG_PRINT("Fin de la détection.\n");
    std::cout << "frame first move : " << frameFirstMove << std::endl;

    
    CHECK_NULL(fvideo = fopen("./bin/frameFirstMove", "w"), "fopen(frameFirstMove)");
    fprintf(fvideo, "%d\n", frameFirstMove);
    fflush(fvideo);
    fclose(fvideo);


    exit(exitStatus);
    return 0;
}

static void signalHandler(int numSig)
{ 
    switch(numSig) {
        case SIGINT : // traitement de SIGINT
            DEBUG_PRINT("\t[%d] --> Arrêt du programme de detection en cours...\n", getpid());
            detectionEnCours = false;
            exitStatus = 2;
            break;
        default :
            printf (" Signal %d non traité \n", numSig );
            break ;
    }
}
