#include "ImageUtils.hpp"
#include "global.hpp"
#include <dirent.h>


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

void traceCourbesPSNRSuperpixelsAVG(std::vector<std::string> imagePaths){
    std::vector<float> psnrValues;
    std::vector<int> KValues;

    std::cout << "Calcul des PSNR pour différentes valeurs de K..." << std::endl;
    int totalSteps = ((400000 - KMIN) / 10000 + 1) * imagePaths.size();
    int currentStep = 0;

    for(int K = KMIN; K <= 400000; K += 10000){
        float psnrSum = 0;
        for (std::string imagePath : imagePaths) {
            cv::Mat image = cv::imread(imagePath);
            if(image.empty()) {
                std::cerr << "Could not open or find the image" << std::endl;
                return;
            }
            cv::Mat modifiedImage = SLICWithoutSaving(stringduplicate(imagePath.c_str()), K, 10);
            psnrSum += PSNR(image, modifiedImage);

            // Update progress
            currentStep++;
            int progress = (currentStep * 100) / totalSteps;
            std::cout << "\rProgress: " << progress << "%";
            std::cout.flush();
        }
        psnrValues.push_back(psnrSum / imagePaths.size());
        KValues.push_back(K);
    }

    std::cout << std::endl << "Affichage des PSNR en fonction de K..." << std::endl;

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

void getAllImagesInFolder(std::string folderPath, std::vector<std::string>& imagePaths){ // on ne dois prendre que les image donc des fichier qui finissent par .png ou .jpg sauf les image qui commence par "SLIC_"
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(folderPath.c_str())) != NULL) { // ouvrir le dossier
        while ((ent = readdir(dir)) != NULL) { // lire les fichiers du dossier
            std::string fileName = ent->d_name;
            if (fileName.length() > 4 && (fileName.substr(fileName.length() - 4) == ".png" || fileName.substr(fileName.length() - 4) == ".jpg") && fileName.substr(0, 5) != "SLIC_") { // si le fichier est une image et ne commence pas par "SLIC_"
                imagePaths.push_back(folderPath + "/" + fileName);
            }
        }
        closedir(dir);
    } else {
        std::cerr << "Could not open directory" << std::endl;
    }
}