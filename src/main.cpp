#include <opencv2/opencv.hpp>
#include <iostream>
#include "timer.hpp"

double PSNR(const cv::Mat& I1, const cv::Mat& I2) {
    double EQM = 0;
    int nH = I1.rows;
    int nW = I1.cols;
    int nTaille = nH * nW;

    for(int i = 0; i < nW; i++) {
        for(int j = 0; j < nH-1; j++) {
            for(int k = 0; k < 3; k++) {
                double diff = I1.at<cv::Vec3b>(j, i)[k] - I2.at<cv::Vec3b>(j, i)[k];
                EQM += diff * diff;
            }
        }
    }

    EQM /= nTaille*3;
    double psnr = 10.0 * std::log10(255 * 255 / EQM);

    return psnr;
}

int main() {
    //lire une image
    cv::Mat image = cv::imread("../src/image/test/OIP.png");
    cv::Mat image2 = cv::imread("../src/image/test/RGB_Y_OIP.png");

    //vérifier si l'image est chargée correctement
    if (image.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return -1;
    }
    if(image2.empty()) {
        std::cerr << "Could not open or find the image2" << std::endl;
        return -1;
    }

    //redimensionner la fenêtre d'affichage
    // int window_width = 480;  //largeur de la fenêtre (par exemple 1920px)
    // int window_height = 480; //hauteur de la fenêtre (par exemple 1080px)
    // cv::namedWindow("fenetre1", cv::WINDOW_NORMAL);  // Crée une fenêtre redimensionnable
    // cv::resizeWindow("fenetre1", window_width, window_height);  // Redimensionne la fenêtre

    //afficher l'image
    //c'est avec "fenetre1" que l'on relie l'affichage de l'image a la fenetre aprametrée audessus
    // cv::imshow("fenetre1", image);

    //attendre une touche pour fermer la fenêtre
    // cv::waitKey(0);

    // //ecrire l'image dans un fichier
    // cv::imwrite("../src/image/R_copy.png", image);

    // test psnr
    //démaré un timer : 
    Timer timer;
    std::cout << "PSNR: " << PSNR(image, image2) <<" dB"<< std::endl;
    //stoper le timer 
    timer.stop();
    std::cout << "Time: " << timer.elapsed() << "s" << std::endl;

    return 0;
}