#include <opencv2/opencv.hpp>
#include <iostream>
#include "timer.hpp"

#define SLIC_THRESHOLD 2.0

class Superpixel{
    public:
        cv::Vec3b rgb;
        int x;
        int y;
        int nbPixels;

        float getDistance(int _S, int m, const cv::Vec3b* _color, int _x, int _y) {
            float d_rgb = cv::norm(cv::Vec3f(rgb) - cv::Vec3f(*_color));
            float d_xy = sqrt(pow(x - _x, 2) + pow(y - _y, 2));
            float ratio = (float)m/_S;
            return d_rgb + ratio * d_xy;
        }

};

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

void SLIC_RECURSIVE(const cv::Mat & I1, std::vector<Superpixel> & superpixels, std::vector<std::pair<float, int>> & pixels, int S, int m, int nH, int nW) {

    // pour chaque superpixel, on calcule la distance entre lui et les pixels de son voisinage
    for (size_t sp_idx = 0; sp_idx < superpixels.size(); sp_idx++) {
        Superpixel& sp = superpixels[sp_idx];

        int minI = std::max(0, sp.x-S);
        int maxI = std::min(nH, sp.x+S);
        int minJ = std::max(0, sp.y-S);
        int maxJ = std::min(nW, sp.y+S);

        for (int i = minI; i < maxI; i++) {
            for (int j = minJ; j < maxJ; j++) {

                cv::Vec3b pixelColor = I1.at<cv::Vec3b>(i, j);
                float distance = sp.getDistance(S, m, &pixelColor, i, j);

                // si la distance est plus petite que celle déjà enregistrée, on la met à jour
                if (distance < pixels[i * nW + j].first) {
                    pixels[i * nW + j].first = distance;
                    pixels[i * nW + j].second = sp_idx;
                }

            }
        }
    }

    // vecteur pour calculer et stocker la nouvelle couleur des superpixels
    std::vector<std::pair<cv::Vec3f, int>> newSuperpixelValues(superpixels.size(), std::make_pair(cv::Vec3f(0.0f, 0.0f,0.0f), 0));

    // pour chaque pixel, on récupère le superpixel associé et on ajoute sa couleur à la somme
    for (int i = 0; i < nH; i++) {
        for (int j = 0; j < nW; j++) {

            int sp_idx = pixels[i * nW + j].second;

            if (sp_idx >= 0 && sp_idx < superpixels.size()) {
                cv::Vec3f color = (cv::Vec3f)I1.at<cv::Vec3b>(i, j);
                newSuperpixelValues[sp_idx].first += color;
                newSuperpixelValues[sp_idx].second++;
            }
        }
    }

    // on divise la somme par le nombre de pixels pour obtenir la nouvelle couleur des superpixels
    for(auto & newSp : newSuperpixelValues) {
        newSp.first /= newSp.second;
    }
    
    float deltaMax = 0.0f;

    // on met à jour la couleur des superpixels avec celle calculée
    for (size_t sp_idx = 0; sp_idx < superpixels.size(); sp_idx++) {
        float delta = cv::norm(cv::Vec3f(superpixels[sp_idx].rgb) - newSuperpixelValues[sp_idx].first);
        deltaMax = std::max(deltaMax, delta);
        superpixels[sp_idx].rgb = newSuperpixelValues[sp_idx].first;
    } 

    // si la différence entre les anciennes et nouvelles couleurs des superpixels est inférieure à un seuil, on arrête 
    if (deltaMax < SLIC_THRESHOLD){
        return;
    }
    
    // on relance l'algorithme avec les nouvelles couleurs des superpixels
    SLIC_RECURSIVE(I1, superpixels, pixels, S, m, nH, nW);
}
 

void SLIC(const cv::Mat& I1, char imgOutName[250], int K = 100, int m = 10) {
    int nH = I1.rows;
    int nW = I1.cols;

    int S = round(sqrt(nW * nH / K));
    std::vector<Superpixel> superpixels; 

    for(int i = S/2; i < nH; i+=S) {
        for(int j = S/2; j < nW; j+=S) {
            Superpixel sp;
            sp.x = round(i);
            sp.y = round(j);
            sp.nbPixels = 0;
            sp.rgb = I1.at<cv::Vec3b>(sp.x, sp.y);
            superpixels.push_back(sp);
        }
    }
    std::cout << "Nb de superpixels créés : " <<superpixels.size() << std::endl;

    // vecteur pour stocker la distance entre chaque pixel et le superpixel associé
    std::vector<std::pair<float, int>> pixels(nH * nW, std::make_pair(std::numeric_limits<float>::infinity(), -1));

    SLIC_RECURSIVE(I1, superpixels, pixels, S, m, nH, nW);

    // creation de l'image de sortie 
    cv::Mat I2 = I1.clone();
    for(int i = 0; i < nH; i++) {
        for(int j = 0; j < nW; j++) {
            int sp_idx = pixels[i * nW + j].second;
            I2.at<cv::Vec3b>(i, j) = superpixels[sp_idx].rgb;
        }
    }

    cv::imwrite(imgOutName, I2);

}



int main() {
    //lire une image
    cv::Mat image = cv::imread("../src/image/test4k.png");
    char modifiedImagePath[250] = "../src/image/test4k_SLIC.png";
    //cv::Mat image2 = cv::imread("../src/image/test/RGB_Y_OIP.png");

    //vérifier si l'image est chargée correctement
    if (image.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return -1;
    }
/*     if(image2.empty()) {
        std::cerr << "Could not open or find the image2" << std::endl;
        return -1;
    } */

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
/*     Timer timer;
    std::cout << "PSNR: " << PSNR(image, image2) <<" dB"<< std::endl;
    //stoper le timer 
    timer.stop();
    std::cout << "Time: " << timer.elapsed() << "s" << std::endl; */

    //test SLIC
    Timer timer;
    SLIC(image, modifiedImagePath, 48000, 10); // 29.7 de PSNR avec ces paramètres sur l'image test4k.png
    timer.stop();
    std::cout << "Time: " << timer.elapsed() << "s" << std::endl; 
    cv::Mat modifiedImage = cv::imread(modifiedImagePath);
    std::cout << "PSNR: " << PSNR(image, modifiedImage) <<" dB"<< std::endl;

    return 0;
}