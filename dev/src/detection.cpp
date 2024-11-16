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
    std::vector<cv::Rect> detections;

    cv::Mat frame, small_frame;
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

        
        

        // Appliquer la détection seulement toutes les 5 images
        if (frame_count % 1 == 0) {
    
            hog.detectMultiScale(small_frame, detections);

            for (auto& rect : detections) {
                rect.x *= 2;
                rect.y *= 2;
                rect.width *= 2;
                rect.height *= 2;
            }

            // // Dessiner des rectangles autour des détections
            // for (const auto& rect : detections) {
            //     cv::rectangle(frame, rect, cv::Scalar(0, 255, 0), 2);
            // }
        }

        // Dessiner des rectangles autour des détections
        for (const auto& rect : detections) {
            cv::rectangle(frame, rect, cv::Scalar(0, 255, 0), 2);
        }

        // Afficher le résultat
        cv::imshow("Détection de personnes", frame);

        frame_count++;

        // Attendre 40 ms entre chaque image (1000 ms / 25 FPS) et quitter avec la touche 'q'
        if (cv::waitKey(40) == 'q') {
            break;
        }
    }

    // Libérer la capture
    cap.release();
    cv::destroyAllWindows();
    
    return 0;
}
