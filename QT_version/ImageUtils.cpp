#include "ImageUtils.h"
#include "global.h"
#include "SLIC.h"
#include <dirent.h>
#include <filesystem>
#include <thread>
#include <future>
#include <mutex>
#include <algorithm>
#include <iostream>
#include "SDGT.h"
#include <QWidget>  // Add this at the top of the file

std::uintmax_t getFileSize(const std::string& filename) {
    return std::filesystem::file_size(filename);
}

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


double PSNR(char* imagePath1, char* imagePath2) {
    cv::Mat I1 = cv::imread(imagePath1);
    cv::Mat I2 = cv::imread(imagePath2);

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

void traceCourbesPSNRSuperpixelsAVG(std::vector<std::string> &imagePaths){
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
            //il faut charger l'image deja modifié par SLIC qui se trouve dans le dossier image/courbe
            //on setrouve dans le dossie "../src/image" et on dois prendre l'image dans "../src/image/courbe" de plus le nom de l'iamge est SLIC_Knombre_nom.png
            //donc on doit ajouter le prefixe "SLIC_Knombre_" et changer de dossier pour le trouver biensur nom c'est le nom actuelle du fichier de imagePaths

            // juste le nom de l'image
            std::string fileName = imagePath.substr(imagePath.find_last_of("/\\") + 1);

            std::string cheminImageSLIC = "../src/image/courbe/SLIC_K" + std::to_string(K) + "_" + fileName;

            cheminImageSLIC.find(".jpg");

            cv::Mat modifiedImage = cv::imread(cheminImageSLIC);

            //afficher le chemin de l'image
            // std::cout << cheminImageSLIC<<" : "<< imagePath << std::endl;
            if(modifiedImage.empty()) {
                std::cerr << "ERROR: Could not open or find the processed image: " << cheminImageSLIC << std::endl;
                std::exit(EXIT_FAILURE);
            }
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
    FILE *pipe = popen("gnuplot -persist", "w");
    if (pipe) {
        // Save to both screen and file
        fprintf(pipe, "set terminal pngcairo size 1280,720 enhanced font 'Arial,12'\n");
        fprintf(pipe, "set output 'psnr_moyen_K.png'\n");

        // Improved title and labels
        fprintf(pipe, "set title 'PSNR moyen en fonction de K' font 'Arial,14'\n");
        fprintf(pipe, "set xlabel 'Nombre de superpixels (K)' font 'Arial,12'\n");
        fprintf(pipe, "set ylabel 'PSNR moyen (dB)' font 'Arial,12'\n");

        // Add grid and improve styling
        fprintf(pipe, "set grid\n");
        fprintf(pipe, "set key top right\n");
        fprintf(pipe, "set style line 1 lc rgb '#008000' lt 1 lw 2 pt 7 ps 1.5\n");

        fprintf(pipe, "set xrange [100000:420000]\n"); // on dépase du x max car sinon le dernier point et sur l'axe et donc peux visible

        // Plot with improved styling
        fprintf(pipe, "plot '-' with linespoints ls 1 title 'Valeurs du PSNR'\n");
        for (int i = 0; i < psnrValues.size(); i++) {
            fprintf(pipe, "%d %f\n", KValues[i], psnrValues[i]);
        }
        fprintf(pipe, "e\n");

        // Create a second plot for interactive viewing
        fprintf(pipe, "set terminal wxt\n");
        fprintf(pipe, "set output\n");
        fprintf(pipe, "replot\n");

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

//le calcule du taux se fait comme cela : taux = (taille du fichier initial)/(taille du fichier compressé)
void traceCourbesTauxSuperpixelsAVG(std::vector<std::string> &imagePaths){
    std::vector<float> tauxValues;
    std::vector<int> KValues;

    std::cout << "Calcul des taux de compression pour différentes valeurs de K..." << std::endl;
    int totalSteps = ((400000 - KMIN) / 10000 + 1) * imagePaths.size();
    int currentStep = 0;




    for(int K = KMIN; K <= 400000; K += 10000){
        float tauxSum = 0;
        for (std::string imagePath : imagePaths) {

            // juste le nom de l'image
            std::string fileName = imagePath.substr(imagePath.find_last_of("/\\") + 1);

            std::string cheminImageSLIC = "../src/image/courbe/SLIC_K" + std::to_string(K) + "_" + fileName;

            cheminImageSLIC.find(".jpg");

            std::cout << cheminImageSLIC << " : " << imagePath << std::endl;

            uintmax_t tailleFichierCompres = getFileSize(cheminImageSLIC);
            uintmax_t tailleFichierInitials = getFileSize(imagePath);

            std::cout << "Taille fichier initial: " << tailleFichierInitials << " Taille fichier compressé: " << tailleFichierCompres << std::endl;

            tauxSum += (tailleFichierInitials) / tailleFichierCompres;


            // Update progress
            currentStep++;
            int progress = (currentStep * 100) / totalSteps;
            std::cout << "\rProgress: " << progress << "%";
            std::cout.flush();
        }
        tauxValues.push_back(tauxSum / imagePaths.size());
        KValues.push_back(K);

    }

    std::cout << std::endl << "Affichage des taux de compression en fonction de K..." << std::endl;

    // Affichage le graphe des taux de compression en fonction de K en utilisant gnuplot
    FILE *pipe = popen("gnuplot -persist", "w");
    if (pipe) {
        // Save to both screen and file
        fprintf(pipe, "set terminal pngcairo size 1280,720 enhanced font 'Arial,12'\n");
        fprintf(pipe, "set output 'taux_compression_K.png'\n");

        // Improved title and labels
        fprintf(pipe, "set title 'Taux de compression en fonction de K' font 'Arial,14'\n");
        fprintf(pipe, "set xlabel 'Nombre de superpixels (K)' font 'Arial,12'\n");
        fprintf(pipe, "set ylabel 'Taux moyen de compression' font 'Arial,12'\n");

        // Add grid and improve styling
        fprintf(pipe, "set grid\n");
        fprintf(pipe, "set key top right\n");
        fprintf(pipe, "set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 1.5\n");

        // Set x-axis range to extend beyond the maximum K value
        fprintf(pipe, "set xrange [100000:420000]\n");
        fprintf(pipe, "set yrange [0.8:1.25]\n");


        // Plot with improved styling
        fprintf(pipe, "plot '-' with linespoints ls 1 title 'Valeurs du taux de compression'\n");
        for (size_t i = 0; i < tauxValues.size(); i++) {
            fprintf(pipe, "%d %f\n", KValues[i], tauxValues[i]);
        }
        fprintf(pipe, "e\n");

        // Create a second plot for interactive viewing
        fprintf(pipe, "set terminal wxt\n");
        fprintf(pipe, "set output\n");
        fprintf(pipe, "replot\n");

        fflush(pipe);
        pclose(pipe);
    } else {
        std::cerr << "Could not open gnuplot" << std::endl;
    }

    std::cout << "Fin de l'affichage des taux de compression en fonction de K" << std::endl;

}


//pour chaque image dans le dossier image on va générer une image SLIC pour chaque valeur de K de 120000 à 400000 par pas de 10000
//il faut enregistrer les images générées dans le dossier image/courbe
//le nom de l'image générée doit être de la forme SLIC_Knombre_nom.
//cela génère les images a utiliser pour les courbes de PSNR et de taux de compression
void genererImageSLIC(std::vector<std::string> &imagePaths){
    std::cout << "Génération des images SLIC pour différentes valeurs de K..." << std::endl;

    // Calculate number of hardware threads, leave one for system
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4; // Fallback if detection fails
    num_threads = std::max(1u, num_threads - 1);

    std::cout << "Using " << num_threads << " threads for processing" << std::endl;

    // Calculate total operations for progress reporting
    int totalSteps = ((400000 - KMIN) / 10000 + 1) * imagePaths.size();
    std::atomic<int> currentStep(0);
    std::mutex progress_mutex;

    // This will hold our futures
    std::vector<std::future<void>> futures;

    // Process by image first, then K value (better memory locality)
    for (const std::string& imagePath : imagePaths) {
        // Create a task for each image
        auto future = std::async(std::launch::async, [&imagePath, &currentStep, totalSteps, &progress_mutex]() {
            // Read image once per thread to avoid reopening
            cv::Mat originalImage = cv::imread(imagePath);
            if (originalImage.empty()) {
                std::cerr << "ERROR: Could not open or find the image: " << imagePath << std::endl;
                std::exit(EXIT_FAILURE);
            }

            std::string fileName = imagePath.substr(imagePath.find_last_of("/\\") + 1);

            for(int K = KMIN; K <= 400000; K += 10000) {
                std::string cheminImageSLIC = "../src/image/courbe/SLIC_K" + std::to_string(K) + "_" + fileName;
                char *cheminImage = stringduplicate(cheminImageSLIC.c_str());

                // Process image with SLIC
                SLIC(stringduplicate(imagePath.c_str()), cheminImage, K, 10);

                // Update progress
                int step = ++currentStep;
                int progress = (step * 100) / totalSteps;

                // Print progress (thread-safe)
                {
                    std::lock_guard<std::mutex> lock(progress_mutex);
                    std::cout << "\rProgress: " << progress << "% - Processing image: " << fileName << " with K=" << K << "      ";
                    std::cout.flush();
                }
            }
        });

        futures.push_back(std::move(future));

        // If we've reached our thread limit, wait for one to complete
        if (futures.size() >= num_threads) {
            // Wait for the first future to complete
            futures.front().wait();
            futures.erase(futures.begin());
        }
    }

    // Wait for remaining futures
    for (auto& future : futures) {
        future.wait();
    }

    std::cout << std::endl << "Fin de la génération des images SLIC" << std::endl;
}

void genererImageSDGT(std::vector<std::string> &imagePaths, int K, QWidget *parentWidget) {
    std::cout << "Génération des images SDGT pour K=" << K << "..." << std::endl;

    // Calculate total operations for progress reporting
    int totalSteps = imagePaths.size(); // Just one K value per image
    int currentStep = 0;

    // Process each image sequentially
    for (const std::string& imagePath : imagePaths) {
        std::cout<<"\n\n"<<std::endl;
        // Read image
        cv::Mat originalImage = cv::imread(imagePath);
        if (originalImage.empty()) {
            std::cerr << "ERROR: Could not open or find the image: " << imagePath << std::endl;
            std::exit(EXIT_FAILURE);
        }

        std::string fileName = imagePath.substr(imagePath.find_last_of("/\\") + 1);
        
        std::string cheminImageSDGT = "../../src/image/courbe/SDGT_K" + std::to_string(K) + "_" + fileName;
        char *cheminImageOut = stringduplicate(cheminImageSDGT.c_str());

        std::cout << "Processing image: " << fileName << " with K=" << K << std::endl;
        
        // Process image with SDGT with the specific K value
        bool success = SDGT(parentWidget, stringduplicate(imagePath.c_str()), cheminImageOut, K, 10, 0, 20);

        if (success) {
            std::cout << "SDGT appliqué avec succès!" << std::endl;
            std::cout << "Image sauvegardée à: " << cheminImageOut << std::endl;
        } else {
            std::cerr << "Erreur lors de l'application de SDGT" << std::endl;
            exit(1);
        }

        // Update and print progress
        currentStep++;
        int progress = (currentStep * 100) / totalSteps;
        std::cout << "\rProgress: " << progress << "% - Completed processing: " << fileName;
        std::cout.flush();
    }

    std::cout << std::endl << "Fin de la génération des images SDGT avec K=" << K << std::endl;
}