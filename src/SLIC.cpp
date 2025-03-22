#include "SLIC.hpp"
#define SLIC_THRESHOLD 2.0

void SLIC_RECURSIVE(const cv::Mat & I1, std::vector<Superpixel> & superpixels, std::vector<std::pair<float, int>> & pixels, int S, int m, int nH, int nW) {

    // pour chaque superpixel, on calcule la distance entre lui et les pixels de son voisinage
    int superpixelsSize = superpixels.size();
    for (size_t sp_idx = 0; sp_idx < superpixelsSize; sp_idx++) {
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
                int idx = i * nW + j;
                if (distance < pixels[idx].first) {
                    pixels[idx].first = distance;
                    pixels[idx].second = sp_idx;
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

            if (sp_idx >= 0 && sp_idx < superpixelsSize) {
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
    for (size_t sp_idx = 0; sp_idx < superpixelsSize; sp_idx++) {
        float delta = cv::norm(cv::Vec3f(superpixels[sp_idx].rgb) - newSuperpixelValues[sp_idx].first);
        deltaMax = std::max(deltaMax, delta);
        superpixels[sp_idx].rgb = newSuperpixelValues[sp_idx].first;
        superpixels[sp_idx].nbPixels = newSuperpixelValues[sp_idx].second;
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
    // std::cout << "Image chargée avec succès" << std::endl;

    int S = round(sqrt(nW * nH / K));
    std::vector<Superpixel> superpixels; 

    int nS = S/2;

    for(int i = nS; i < nH; i+=S) {
        for(int j = nS; j < nW; j+=S) {
            Superpixel sp;
            sp.x = round(i);
            sp.y = round(j);
            sp.nbPixels = 0;
            sp.rgb = I1.at<cv::Vec3b>(sp.x, sp.y);
            superpixels.push_back(sp);
        }
    }
    // std::cout << "Nb de superpixels créés : " <<superpixels.size() << std::endl;

    // vecteur pour stocker la distance entre chaque pixel et le superpixel associé
    std::vector<std::pair<float, int>> pixels(nH * nW, std::make_pair(std::numeric_limits<float>::infinity(), -1));

    SLIC_RECURSIVE(I1, superpixels, pixels, S, m, nH, nW);

    // std::cout << "SLIC terminé" << std::endl;

    // creation de l'image de sortie 
    cv::Mat I2 = I1.clone();
    for(int i = 0; i < nH; i++) {
        for(int j = 0; j < nW; j++) {
            int sp_idx = pixels[i * nW + j].second;
            I2.at<cv::Vec3b>(i, j) = superpixels[sp_idx].rgb;
        }
    }
    // std::cout << "Image de sortie créée" << std::endl;
    if(strlen(imgOutName) == 0){
        char suffix[50];
        sprintf(suffix, "_K%d_m%d", K, m);
        transformeNomImage(imagePath, nullptr, "SLIC_", suffix, imgOutName);
    }
    // std::cout << "Image de sortie enregistrée" << std::endl;

    cv::imwrite(imgOutName, I2);

}

cv::Mat SLICWithoutSaving(char* imagePath , int K = 100, int m = 10){
    cv::Mat I1 = cv::imread(imagePath);
    int nH = I1.rows;
    int nW = I1.cols;

    //vérifier si l'image est chargée correctement

    if (I1.empty()) {
        std::cerr << "Could not open or find the image : %d"<< imagePath << std::endl;
        exit(1);
    }
    // std::cout << "Image chargée avec succès" << std::endl;

    int S = round(sqrt(nW * nH / K));
    std::vector<Superpixel> superpixels; 

    int nS = S/2;

    for(int i = nS; i < nH; i+=S) {
        for(int j = nS; j < nW; j+=S) {
            Superpixel sp;
            sp.x = round(i);
            sp.y = round(j);
            sp.nbPixels = 0;
            sp.rgb = I1.at<cv::Vec3b>(sp.x, sp.y);
            superpixels.push_back(sp);
        }
    }
    // std::cout << "Nb de superpixels créés : " <<superpixels.size() << std::endl;

    // vecteur pour stocker la distance entre chaque pixel et le superpixel associé
    std::vector<std::pair<float, int>> pixels(nH * nW, std::make_pair(std::numeric_limits<float>::infinity(), -1));

    SLIC_RECURSIVE(I1, superpixels, pixels, S, m, nH, nW);

    // std::cout << "SLIC terminé" << std::endl;

    // creation de l'image de sortie 
    cv::Mat I2 = I1.clone();
    for(int i = 0; i < nH; i++) {
        for(int j = 0; j < nW; j++) {
            int sp_idx = pixels[i * nW + j].second;
            I2.at<cv::Vec3b>(i, j) = superpixels[sp_idx].rgb;
        }
    }

    return I2;
    
}