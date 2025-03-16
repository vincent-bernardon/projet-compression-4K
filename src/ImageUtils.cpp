#include "ImageUtils.hpp"

char* stringduplicate(const char* source) {
    if (!source) return nullptr;
    size_t len = strlen(source) + 1;  // +1 for null terminator
    char* copy = new char[len];
    strcpy(copy, source);
    return copy;
}

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

void traceCourbesPSNRSuperpixels(char *imagePath){
    cv::Mat image = cv::imread(imagePath);
    if(image.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return;
    }
    
    std::vector<float> psnrValues;
    std::vector<int> KValues;

    std::cout << "Calcul des PSNR pour différentes valeurs de K..." << std::endl;
    for(int K = 63000; K <= 200000; K += 10000){
        cv::Mat modifiedImage = SLICWithoutSaving(stringduplicate(imagePath), K, 10);
        
        psnrValues.push_back(PSNR(image, modifiedImage));
        KValues.push_back(K);
    }

    std::cout << "Affichage des PSNR en fonction de K..." << std::endl;

    // Affichage le graphe des PSNR en fonction de K en utilisant gnuplot
    FILE *pipe = popen("gnuplot -persist", "w"); //permet d'ouvrir un pipe pour écrire dans gnuplot et donc pas généré un fichier de sortie
    if (pipe) {
        fprintf(pipe, "set title 'PSNR en fonction de K'\n");
        fprintf(pipe, "set xlabel 'K'\n");
        fprintf(pipe, "set ylabel 'PSNR (dB)'\n");
        fprintf(pipe, "plot '-' with linespoints title 'Valeurs du PSNR'\n");
        for (int i = 0; i < psnrValues.size(); i++) {
            fprintf(pipe, "%d %f\n", KValues[i], psnrValues[i]);
        }
        fprintf(pipe, "e\n");
        fflush(pipe);
        pclose(pipe);
    } else {
        std::cerr << "Could not open gnuplot" << std::endl;
    }

    std::cout << "Fin de l'affichage des PSNR en fonction de K" << std::endl;

}

void traceCourbesPSNRCompacite(char *imagePath){
    cv::Mat image = cv::imread(imagePath);
    if(image.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return;
    }
    
    std::vector<float> psnrValues;
    std::vector<int> MValues;

    std::cout << "Calcul des PSNR pour différentes valeurs de M..." << std::endl;
    for(int M = 1; M <= 101; M += 10){
        std::cout << "M = " << M << std::endl;
        cv::Mat modifiedImage = SLICWithoutSaving(stringduplicate(imagePath), 63000, M);
        
        psnrValues.push_back(PSNR(image, modifiedImage));
        MValues.push_back(M);
    }

    std::cout << "Affichage des PSNR en fonction de M..." << std::endl;

    // Affichage le graphe des PSNR en fonction de M en utilisant gnuplot
    FILE *pipe = popen("gnuplot -persist", "w"); //permet d'ouvrir un pipe pour écrire dans gnuplot et donc pas généré un fichier de sortie
    if (pipe) {
        fprintf(pipe, "set title 'PSNR en fonction de M'\n");
        fprintf(pipe, "set xlabel 'M'\n");
        fprintf(pipe, "set ylabel 'PSNR (dB)'\n");
        fprintf(pipe, "plot '-' with linespoints title 'Valeurs du PSNR'\n");
        for (int i = 0; i < psnrValues.size(); i++) {
            fprintf(pipe, "%d %f\n", MValues[i], psnrValues[i]);
        }
        fprintf(pipe, "e\n");
        fflush(pipe);
        pclose(pipe);
    } else {
        std::cerr << "Could not open gnuplot" << std::endl;
    }

    std::cout << "Fin de l'affichage des PSNR en fonction de M" << std::endl;

}