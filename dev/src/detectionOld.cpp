/* ------------------------------------------------------------------------ */
/*                       Suivi grimpeur - Detection                         */
/*                        Auteur : CHEVALIER Romain                         */
/*                            Date : 26-10-2024                             */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/*                   E N T Ê T E S    S T A N D A R D S                     */
/* ------------------------------------------------------------------------ */

#include <detection.hpp>

/* ------------------------------------------------------------------------ */
/*                 V A R I A B L E S    G L O B A L E S                     */
/* ------------------------------------------------------------------------ */

#define AREA_MAX 921600
#define WIDTH 1280
#define HEIGHT 720
#define TOLERANCE_MIN 0.1
#define TOLERANCE_MAX 0.9

/* ------------------------------------------------------------------------ */
/*            D E F I N I T I O N   D E    F O N C T I O N S                */
/* ------------------------------------------------------------------------ */



int main(int argc, char const *argv[])
{
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
        return -1;
    }

    // Forcer les FPS et la résolution
    cap.set(cv::CAP_PROP_FPS, 25);           // Définir les FPS à 25
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);  // Largeur à 1280
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);  // Hauteur à 720

    // Initialiser le détecteur HOG pour la détection de personnes
    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

    int frame_count = 0;
    std::vector<cv::Rect> detectionsNormal, detectionsGray, detectionsThresh;

    cv::Mat frame, small_frame, frameDelta, thresh, frameReference, gray;
    while (true) {
        // Lire une image
        cap >> frame;

        // Vérifier si l'image est vide (fin du flux)
        if (frame.empty()) {
            std::cerr << "Erreur: Image vide capturée." << std::endl;
            break;
        }

        // Redimensionner l'image à une taille plus petite (ex : 640x360)
        cv::resize(frame, small_frame, cv::Size(640, 360));

        
        // // Convertir l'image en niveaux de gris pour améliorer la détection
        cv::cvtColor(small_frame, gray, cv::COLOR_BGR2GRAY);
        // Appliquer une égalisation d'histogramme pour améliorer le contraste
        

        // Appliquer un flou gaussien pour réduire le bruit
        cv::Size ksize(21, 21);
        cv::GaussianBlur(gray, gray, ksize, 0);
        

        
        // Si la camera bouge je change l'amge de reference sinon ca reste la meme
        if (frame_count  % 25 == 0) {
            frameReference = gray.clone();
        }
        //cv::imshow("Frame Reference", frameReference);


        cv::absdiff(frameReference, gray, frameDelta);
        //cv::imshow("Frame Delta", frameDelta);

        cv::threshold(frameDelta, thresh, 25, 255, cv::THRESH_BINARY);
        cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 2);
         

        cv::equalizeHist(gray, gray); 
        

        // Appliquer la détection seulement toutes les 5 images
        if (frame_count % 5 == 0) {
    
            hog.detectMultiScale(small_frame, detectionsNormal);
            hog.detectMultiScale(gray, detectionsGray);
            hog.detectMultiScale(thresh, detectionsThresh);


            if (!detectionsThresh.empty()) {
                auto rect = detectionsThresh[0];
                rect.x *= 2;
                rect.y *= 2;
                rect.width *= 2;
                rect.height *= 2;
                // std::cout << "Detection normal : x=" << rect.x << ", y=" << rect.y << ", w=" << rect.width << ", h=" << rect.height << ", area=" << rect.area() << std::endl;

                // if (rect.area() < AREA_MAX* TOLERANCE_MIN){
                //     // Voir si on peut pas calculer le zoom en fonction du rapport des aires
                //     requetePTZ("rzoom", "500");
                // }
                // if (rect.area() > AREA_MAX* TOLERANCE_MAX){
                //     requetePTZ("rzoom", "-500");
                // }
                // if (rect.x < WIDTH * (1-TOLERANCE_MAX)){
                //     requetePTZ("move", "right");
                // }
                // if (rect.x > WIDTH*TOLERANCE_MAX){
                //     requetePTZ("move", "left");
                // }
                // if (rect.y < HEIGHT*(1-TOLERANCE_MAX)){
                //     requetePTZ("move", "down");
                // }
                // if (rect.y > HEIGHT*TOLERANCE_MAX){
                //     requetePTZ("move", "up");
                // }


                cv::rectangle(frame, rect, cv::Scalar(0, 255, 0), 2);
                cv::imshow("Detection normal", frame);
            }

        }

        // Dessiner des rectangles autour des détections
        for (const auto& rect : detectionsNormal) { // Rouge
            cv::rectangle(small_frame, rect, cv::Scalar(0, 0, 255), 2);
        }

        for (const auto& rect : detectionsGray) { // bleu
            cv::rectangle(small_frame, rect, cv::Scalar(255, 0, 0), 2);
            cv::rectangle(gray, rect, cv::Scalar(255, 0, 0), 2);
        }
        cv::imshow("Gray", gray);

        for (const auto& rect : detectionsThresh) { //vert
            cv::rectangle(small_frame, rect, cv::Scalar(0, 255, 0), 2);
            cv::rectangle(thresh, rect, cv::Scalar(0, 255, 0), 2);
        }
        cv::imshow("Thresh", thresh); 

        // Afficher le résultat
        cv::imshow("Détection de personnes", small_frame);

        
        
        

        frame_count++;

        // Attendre 40 ms entre chaque image (1000 ms / 25 FPS) et quitter avec la touche 'q'
        if (cv::waitKey(10) == 'q') {
            break;
        }
    }

    // Libérer la capture
    cap.release();
    cv::destroyAllWindows();

    DEBUG_PRINT("Fin de la détection.\n");
    
    return 0;
}
