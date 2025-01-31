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

    
    int frame_count = 0, resetFrameReference = 0;
    cv::Moments m, m2;
    double cX = 0, cX2 = 0, cY = 0, cY2 = 0, old_cY = 0, old_cY2 = 0;
    double d_cY = 0, d_cY2 = 0;

    double a = 0.3; // Correcteur filtre exponential moving average

    int tps = 0;

    std::string mvmt = "";

    cv::Mat frame, small_frame, frameDelta, thresh, frameReference, gray, moitieHaute;
    
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

        
        // Convertir l'image en niveaux de gris pour améliorer la détection
        cv::cvtColor(small_frame, gray, cv::COLOR_BGR2GRAY);
        
        // Appliquer une égalisation d'histogramme pour améliorer le contraste
        cv::equalizeHist(gray, gray); 

        // Appliquer un flou gaussien pour réduire le bruit
        cv::GaussianBlur(gray, gray, cv::Size(21, 21), 0);
        

        
        // Remettre à zéro l'image de référence toutes les 25 images ou lors d'un mouvement de camera
        if (frame_count  % 3 == 0 ||  resetFrameReference == 1) {
            frameReference = gray.clone();
            resetFrameReference = 0;
        }

        //cv::imshow("Frame Reference", frameReference);

        // Image des differences en binaires
        cv::absdiff(frameReference, gray, frameDelta);
        cv::threshold(frameDelta, thresh, 25, 255, cv::THRESH_BINARY);
        cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 2);
         

        // Forcer le reset de l'image de reference en cas de mvt de camera
        //std::cout <<  cv::countNonZero(thresh) << std::endl;
        if (cv::countNonZero(thresh) > 30000 ) {
            resetFrameReference = 1;
            // cX = 0;
            // cY = 0;
            // cX2 = 0;
            // cY2 = 0;
        }
        else {
            // calcule du barycentre du blanc sur l'image
            m = cv::moments(thresh, true);
            

            /*
            ---------- Definition cv::Moments ------------
            
La classe cv::Moments est utilisée pour calculer les moments spatiaux d'une image. Les moments spatiaux sont des caractéristiques d'une image qui sont utilisées pour décrire la forme et l'orientation de l'objet. Ils sont utilisés pour calculer le centre de gravité de l'objet, la surface de l'objet, etc. Les moments spatiaux sont calculés à partir de l'image binaire. Les moments spatiaux sont utilisés pour calculer le centre de gravité de l'objet, la surface de l'objet, etc.


Les moments spatiaux sont définis comme suit :

m.m00 : Moment d'ordre zéro, qui représente l'aire (ou la somme des intensités des pixels pour une image binaire).

m.m10 et m.m01 : Moments d'ordre un, qui sont utilisés pour calculer le centre de gravité (barycentre) de l'image.

m.m20, m.m11, m.m02 : Moments d'ordre deux, qui sont utilisés pour calculer des informations sur la forme et l'orientation de l'objet.


            */

            // Calculer les coordonnées du barycentre, filtre exponential moving average
            
            cX = a * ((m.m00 > 1) ? m.m10 / m.m00 : cX ) + (1-a) * cX;
            cY = a * ((m.m00 > 1) ? m.m01 / m.m00 : cY ) + (1-a) * cY;
            
            
            // std::cout << "cX : " << cX << " cY : " << cY << std::endl;
            


            // Moitie haute de l'image prise en compte seulement s'il y a assez de pixel en blanc

            moitieHaute = thresh.rowRange(0, 180);
            if (cv::countNonZero(moitieHaute) > 1000) {
                // Moitie haute de l'image
                

                m2 = cv::moments(moitieHaute, true);
            

                cX2 = a * ((m2.m00 > 1) ? m2.m10 / m2.m00 : cX2 ) + (1-a) * cX2;
                cY2 = a * ((m2.m00 > 1) ? m2.m01 / m2.m00 : cY2 ) + (1-a) * cY2;}
        }
        
        /*
        Comment decider du mouvement de la camera en fonction des deux barycentres (tout l'image et moitie haute) ?

        1. Comparer les deux barycentres
        
        2. Si confondus ou proche a epsilon pres, bouger la cam si c'est dans les 60 pixels du bord de l'écran

        3. Si ce n'est pas confondu, Comparaison des variations des barycentres des deux images : 
            - Si la variation de barycentre de l'image entière est petite a esp pres grand que la variation de barycentre de la moitié haute, le grimpeur va vers le haut (bien regler le esp)
        
        Autre idee : se focaliser que sur la moitie haute de l'image si l'image du bas a trop de blanc (plus que l'equivalent d'un homme)

        Pour descendre voir apres, peut etre meme methode avec la demi image basse

        */
       
       int toleranceNbPx = 50;
       if (abs(cY - cY2) < toleranceNbPx ) {
            // Les deux barycentres sont confondus en ordonnee
            // std::cout << "Les deux barycentres sont confondus en ordonnee " << std::endl;

            if (cY < 60) {
                // Bouger la camera vers le haut
                //std::cout << "Bouger la camera vers le haut" << std::endl;
                // requetePTZ("move", "up"); 
                mvmt = "haut";

            }
            
            mvmt = "";

        } else {
            //  Les deux barycentres ne sont pas confondus en ordonnee
            // std::cout << "Les deux barycentres ne sont pas confondus" << std::endl;
           
           
            // d_cY = abs(cY - old_cY);
            // d_cY2 = abs(cY2 - old_cY2);
            
            // std::cout << "d_cY : " << abs(d_cY - d_cY2) << std::endl;    

            double esp = 10;

            if (d_cY2 > (1 + esp) * d_cY){
                // std::cout << "Le grimpeur va vers le haut" << std::endl;
                mvmt = "haut";
            }
            else mvmt = "";
       }

        d_cY = abs(cY - old_cY);
        d_cY2 = abs(cY2 - old_cY2);
        tps += 40;
        std::cout << tps << "," << cY << ","<< cY2 << "," << cY2- cY << "," << cv::countNonZero(thresh) << std::endl;

        

        // Afficher le barycentre sur l'image
        cv::circle(small_frame, cv::Point(cX, cY), 5, cv::Scalar(0, 0, 255), -1);
        cv::circle(small_frame, cv::Point(cX2, cY2), 5, cv::Scalar(0, 255, 0), -1);


        // Afficher du nb de point blanc sur la moitie haute de l'image sur l'image
         // cv::putText(moitieHaute, (std::string) "nb px blancs haut : " + std::to_string(cv::countNonZero(thresh)), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);

        cv::putText(small_frame, mvmt, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255), 2);


        // Afficher l'image
        cv::imshow("Thresh", thresh);
        //cv::imshow("Moitie haute", moitieHaute);

        cv::imshow("Barycentre", small_frame); 
        

        frame_count++;
        
        // Sauvegarder l'ancien barycentre
        old_cY = cY;
        old_cY2 = cY2;

        // Attendre 40 ms entre chaque image (1000 ms / 25 FPS) et quitter avec la touche 'q'
        if (cv::waitKey(40) == 'q') {
            break;
        }
    }

    // Libérer la capture
    cap.release();
    cv::destroyAllWindows();

    DEBUG_PRINT("Fin de la détection.\n");
    
    return 0;
}
