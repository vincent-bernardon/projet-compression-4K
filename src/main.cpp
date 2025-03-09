#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    //lire une image
    cv::Mat image = cv::imread("../src/image/R.png");

    //vérifier si l'image est chargée correctement
    if (image.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return -1;
    }

    //redimensionner la fenêtre d'affichage
    int window_width = 480;  //largeur de la fenêtre (par exemple 1920px)
    int window_height = 480; //hauteur de la fenêtre (par exemple 1080px)
    cv::namedWindow("fenetre1", cv::WINDOW_NORMAL);  // Crée une fenêtre redimensionnable
    cv::resizeWindow("fenetre1", window_width, window_height);  // Redimensionne la fenêtre

    //afficher l'image
    //c'est avec "fenetre1" que l'on relie l'affichage de l'image a la fenetre aprametrée audessus
    cv::imshow("fenetre1", image);

    //attendre une touche pour fermer la fenêtre
    cv::waitKey(0);

    // //ecrire l'image dans un fichier
    // cv::imwrite("../src/image/R_copy.png", image);

    return 0;
}