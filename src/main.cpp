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

void transformeNomImage(char *cheminImage, const char *nouvelleExtension, const char *prefixe, const char *suffixe, char *resultat)
{
    // Extraire uniquement le nom du fichier de cheminImage
    char *base = basename(cheminImage);

    // Copier le chemin de cheminImage dans le résultat
    strcpy(resultat, cheminImage);
    char *lastSlash = strrchr(resultat, '/');
    if (lastSlash != NULL)
    {
        *(lastSlash + 1) = '\0'; // Terminer la chaîne après le dernier '/'
    }
    else
    {
        resultat[0] = '\0'; // Si aucun '/' n'est trouvé, vider le résultat
    }

    // Ajouter le préfixe au résultat
    strcat(resultat, prefixe);

    // Ajouter le nom du fichier (sans extension) au résultat
    char *point = strrchr(base, '.');
    if (point != NULL)
    {
        *point = '\0'; // Terminer la chaîne à l'emplacement du point
    }
    strcat(resultat, base);

    // Ajouter le suffixe au résultat
    strcat(resultat, suffixe);

    // Ajouter la nouvelle extension au résultat
    strcat(resultat, ".");
    if (nouvelleExtension == nullptr || strlen(nouvelleExtension) == 0) {
        strcat(resultat, point + 1); // Utiliser l'extension de cheminImage
    } else {
        strcat(resultat, nouvelleExtension);
    }
}

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
 

void SLIC(char* imagePath , char* imgOutName, int K = 100, int m = 10) {
    cv::Mat I1 = cv::imread(imagePath);
    int nH = I1.rows;
    int nW = I1.cols;

    //vérifier si l'image est chargée correctement

    if (I1.empty()) {
        std::cerr << "Could not open or find the image : %d"<< imagePath << std::endl;
        exit(1);
    }
    std::cout << "Image chargée avec succès" << std::endl;

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

    std::cout << "SLIC terminé" << std::endl;

    // creation de l'image de sortie 
    cv::Mat I2 = I1.clone();
    for(int i = 0; i < nH; i++) {
        for(int j = 0; j < nW; j++) {
            int sp_idx = pixels[i * nW + j].second;
            I2.at<cv::Vec3b>(i, j) = superpixels[sp_idx].rgb;
        }
    }
    std::cout << "Image de sortie créée" << std::endl;
    if(strlen(imgOutName) == 0){
        char suffix[50];
        sprintf(suffix, "_K%d_m%d", K, m);
        transformeNomImage(imagePath, nullptr, "SLIC_", suffix, imgOutName);
    }
    std::cout << "Image de sortie enregistrée" << std::endl;

    cv::imwrite(imgOutName, I2);

}



int main() {
    //lire une image
    char imagePath[250] = "../src/image/test4k.png";
    char imageOut[250] = {0};



    //test SLIC
    Timer timer;
    SLIC(imagePath, imageOut ,48000, 10); // 29.7 de PSNR avec ces paramètres sur l'image test4k.png
    timer.stop();
    std::cout << "imageOut: " << imageOut << std::endl;
    // std::cout << "Time: " << timer.elapsed() << "s" << std::endl; 
    // cv::Mat modifiedImage = cv::imread(imageOut);
    // std::cout << "PSNR: " << PSNR(image, modifiedImage) <<" dB"<< std::endl;

    return 0;
}