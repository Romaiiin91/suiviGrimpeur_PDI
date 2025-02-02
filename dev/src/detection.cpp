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


/* ------------------------------------------------------------------------ */
/*                 V A R I A B L E S    G L O B A L E S                     */
/* ------------------------------------------------------------------------ */

#define WIDTH 1280
#define HEIGHT 720

bool detectionEnCours = true;

/* ------------------------------------------------------------------------ */
/*            D E F I N I T I O N   D E    F O N C T I O N S                */
/* ------------------------------------------------------------------------ */




int main(int argc, char const *argv[])
{
    if (argc < 2) {
        DEBUG_PRINT("Pas url video");
        exit(EXIT_FAILURE);
    }


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
        return 1;
    }

    int framesBetweenReference = json_integer_value(json_object_get(root, "framesBetweenReferences"));
    int verticalThreshold = json_integer_value(json_object_get(root, "verticalThreshold"));
    int horizontalThreshold = json_integer_value(json_object_get(root, "horizontalThreshold"));

    float coefAverageMovingFilter = json_real_value(json_object_get(root, "coefAverageMovingFilter"));
    int coefGaussianBlur = json_integer_value(json_object_get(root, "coefGaussianBlur"));
    int heightResize = json_integer_value(json_object_get(root, "heightResize"));
    int widthResize = json_integer_value(json_object_get(root, "widthResize"));
    int cropRatioDetectionAreaWidth = json_integer_value(json_object_get(root, "cropRatioDetectionAreaWidth"));
    int waitTimeMs = json_integer_value(json_object_get(root, "waitTimeMs"));

    float increasePTZ = json_real_value(json_object_get(root, "increasePTZ"));

    json_decref(root);

    int widthDetectionArea = (cropRatioDetectionAreaWidth - 2) * widthResize/cropRatioDetectionAreaWidth;

    FILE *fvideo;
    CHECK_NULL(fvideo = fopen("./bin/frameFirstMove", "w"), "fopen(frameFirstMove)");
    fprintf(fvideo, "0\n"); // Reset du fichier
    fflush(fvideo);
    fclose(fvideo);



    // Ouvrir la capture vidéo avec l'URL
    cv::VideoCapture cap(argv[1]);
    DEBUG_PRINT("Debut detection sur le lien : %s\n", argv[1]);

    // Vérifier si la capture est ouverte
    if (!cap.isOpened()) {
        std::cerr << "Erreur: Impossible d'ouvrir le flux vidéo." << std::endl;
        return -1;
    }

    // Forcer les FPS et la résolution
    cap.set(cv::CAP_PROP_FPS, 25);           // Définir les FPS à 25
    cap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);  // Largeur à 1280
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);  // Hauteur à 720

    
    int frame_count = 0, resetFrameReference = 0;
    cv::Moments m, mDetectionArea;
    double cX_detectionArea = widthDetectionArea/2, cY_detectionArea = heightResize/2;
   

    std::string mvmtVert = "", mvmtHoriz = "";
    cv::Mat frame, frameDelta, thresh, frameReference, gray, detectionArea;

    bool firstMove = false;
    int frameFirstMove = 0;
    
    while (detectionEnCours) {
        // Lire une image
        cap >> frame;

        // Vérifier si l'image est vide (fin du flux)
        if (frame.empty()) {
            std::cerr << "Erreur: Image vide capturée." << std::endl;
            break;
        }

        cv::resize(frame, frame, cv::Size(widthResize, heightResize));
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray); 
        cv::GaussianBlur(gray, gray, cv::Size(coefGaussianBlur, coefGaussianBlur), 0);
        

        
        // Remettre à zéro l'image de référence toutes les x images ou lors d'un mouvement de camera
        if (frame_count  % framesBetweenReference == 0 ||  resetFrameReference == 1) {
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
        }
        else {
            // Zone de detection

            detectionArea = thresh.rowRange(0, heightResize/2);
            detectionArea = detectionArea.colRange(widthResize/cropRatioDetectionAreaWidth, widthResize -  widthResize/cropRatioDetectionAreaWidth);
            
            if (cv::countNonZero(detectionArea) > 1000) {
                // Moitie haute de l'image
                

                mDetectionArea = cv::moments(detectionArea, true);
            

                cX_detectionArea = coefAverageMovingFilter * ((mDetectionArea.m00 > 1) ? mDetectionArea.m10 / mDetectionArea.m00 : cX_detectionArea ) + (1-coefAverageMovingFilter) * cX_detectionArea;

                cY_detectionArea = coefAverageMovingFilter * ((mDetectionArea.m00 > 1) ? mDetectionArea.m01 / mDetectionArea.m00 : cY_detectionArea ) + (1-coefAverageMovingFilter) * cY_detectionArea;

                if (cY_detectionArea < heightResize / verticalThreshold){
                    mvmtVert = "Monte";
                     
                    #ifndef VIDEO 
                    requetePTZ("rtilt", std::to_string(increasePTZ).c_str());   
                    resetFrameReference = 1;
                    #endif
 
                    if (!firstMove) {
                        frameFirstMove = frame_count;
                        firstMove = true;

                        //std::cout << "frame first move : " << frameFirstMove << std::endl;
                    }
                   
                }
                else {
                    mvmtVert = "";
                }
                
                

                if (cX_detectionArea < widthDetectionArea / horizontalThreshold){
                    mvmtHoriz= "Gauche";

                    #ifndef VIDEO
                    requetePTZ("rpan", std::to_string(-1*increasePTZ).c_str());
                    #endif
                    resetFrameReference = 1;
                }
                else if (cX_detectionArea > widthDetectionArea - widthDetectionArea / horizontalThreshold){
                    mvmtHoriz = "Droite";

                    #ifndef VIDEO
                    requetePTZ("rpan", std::to_string(increasePTZ).c_str());
                    resetFrameReference = 1;
                    #endif
                }
                else {
                    mvmtHoriz = "";
                }
                
            }
        }
        
       
        

        // Afficher le barycentre sur l'image
        cv::circle(frame, cv::Point(cX_detectionArea + widthResize/cropRatioDetectionAreaWidth, cY_detectionArea), 5, cv::Scalar(0, 255, 0), -1);


        // Afficher du nb de point blanc sur la moitie haute de l'image sur l'image
        cv::putText(thresh, (std::string) "nb px blancs Area : " + std::to_string(cv::countNonZero(detectionArea)), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);

        cv::putText(frame, mvmtVert, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);
        cv::putText(frame, mvmtHoriz, cv::Point(500, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);


        // Afficher l'image
        cv::imshow("Thresh", thresh);
        // cv::imshow("Moitie haute", detectionArea);

        cv::imshow("Barycentre", frame); 
        

        frame_count++;
        
        

        // Attendre 40 ms entre chaque image (1000 ms / 25 FPS) et quitter avec la touche 'q'
        if (cv::waitKey(waitTimeMs) == 'q') {
            break;
        }
    }

    // Libérer la capture
    cap.release();
    cv::destroyAllWindows();

    DEBUG_PRINT("Fin de la détection.\n");
    std::cout << "frame first move : " << frameFirstMove << std::endl;

    
    CHECK_NULL(fvideo = fopen("./bin/frameFirstMove", "w"), "fopen(frameFirstMove)");
    fprintf(fvideo, "%d\n", frameFirstMove);
    fflush(fvideo);
    fclose(fvideo);


    exit(EXIT_SUCCESS);
    return 0;
}

static void signalHandler(int numSig)
{ 
    switch(numSig) {
        case SIGINT : // traitement de SIGINT
            DEBUG_PRINT("\t[%d] --> Arrêt du programme de detection en cours...\n", getpid());
            detectionEnCours = false;
            break;
        default :
            printf (" Signal %d non traité \n", numSig );
            break ;
    }
}
