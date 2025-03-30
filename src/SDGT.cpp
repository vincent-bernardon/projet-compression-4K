#include <opencv2/opencv.hpp>
#include "SLIC.hpp"
#include "SDGT.hpp"
#include "Superpixel.hpp"


cv::Mat SDGT(char* imagePath , int K = 100, int m = 10, int mp = 10){
    if(K < mp){
        std::cout << "nombre de superpixels inférieur au nouveau nombre" << std::endl;
        exit(1);
    }


    cv::Mat I1 = cv::imread(imagePath);

    int nH = I1.rows;
    int nW = I1.cols;

    std::cout << "taille de l'image : " << nH * nW << std::endl;

    //vérifier si l'image est chargée correctement
    if (I1.empty()) {
        std::cerr << "Could not open or find the image : %d"<< imagePath << std::endl;
        exit(1);
    }
    // std::cout << "Image chargée avec succès" << std::endl;

    int S = round(sqrt(nW * nH / K));
    std::vector<Superpixel> superpixels; 

    int nS = S/2;

    int nbSuperpixelPerLines = ceil((float)(nW - nS) / S);
    int nbSuperpixelPerCols = ceil((float)(nH - nS) / S);

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

    // vecteur pour stocker la distance entre chaque pixel et le superpixel associé
    std::vector<std::pair<float, int>> pixels(nH * nW, std::make_pair(std::numeric_limits<float>::infinity(), -1));

    SLIC_RECURSIVE(I1, superpixels, pixels, S, m, nH, nW);

    cv::Mat slicResult = I1.clone();


    //vecteur pour stocker a quel superpixel le pixel est associé (sans la distance)
    std::vector<int> pixelToSuperpixel(nH * nW);
    for (int i = 0; i < nH * nW; i++) {
        pixelToSuperpixel[i] = pixels[i].second;
    }
    
    pixels.clear();

    // Color each pixel with its superpixel's color
    for (int x = 0; x < nH; x++) {
        for (int y = 0; y < nW; y++) {
            int pixelIndex = x * nW + y;
            int spID = pixelToSuperpixel[pixelIndex];

            if (spID >= 0 && spID < superpixels.size()) {
                slicResult.at<cv::Vec3b>(x, y) = superpixels[spID].rgb;
            }
        }
    }

    std::cout << "PSNR entre l'image de base et slic : " << PSNR(I1, slicResult) <<std::endl;
    
    std::string slicOutputPath = std::string(imagePath);
    size_t lastDot = slicOutputPath.find_last_of('.');
    if (lastDot != std::string::npos) {
        slicOutputPath = slicOutputPath.substr(0, lastDot) + "_slic" + slicOutputPath.substr(lastDot);
    } else {
        slicOutputPath += "_slic.png";
    }
    cv::imwrite(slicOutputPath, slicResult);
    std::cout << "SLIC visualization saved to: " << slicOutputPath << std::endl;


    //calculer la couleur lab (passe de rgb à xyz à lab)
    computeLAB(superpixels);

    // créer graph
    Graph graph(superpixels, pixelToSuperpixel, nbSuperpixelPerLines, nbSuperpixelPerCols);
      
    std::cout << "\n=== Graph crée ===\n";

    int nbOfSuperPixelInGraph = superpixels.size() ;
    while( nbOfSuperPixelInGraph > mp){
        graph.mergeClosestSuperpixel(pixelToSuperpixel);
        nbOfSuperPixelInGraph--;
    }
    std::cout << "\n=== Graph réduit ===\n";

    int maxPixelCount = 0;
    int maxPixelSuperpixelID = -1;

    for (size_t i = 0; i < graph.nodes.size(); i++) {
        const GraphNode& node = graph.nodes[i];
        if (node.ignore) continue;
        
        if (node.superpixel->nbPixels > maxPixelCount) {
            maxPixelCount = node.superpixel->nbPixels;
            maxPixelSuperpixelID = i;
            node.superpixel->setRGBfromLAB();
        }
    }

    std::cout << "\n=== Graph Reconverti en RGB ===\n";
    std::cout << "Le nombre le plus élevé d'élément dans un superpixel : " << maxPixelCount << " , pour le pixel id : " << maxPixelSuperpixelID << std::endl;

    cv::Mat result = I1.clone(); // Déclarer avant la boucle des superpixels

    unsigned int nbSP = 0;
    //unsigned int nbpixels= 0;
    // pour chaque superpixels 
    for (size_t i = 0; i < graph.nodes.size(); i++) {
        const GraphNode& node = graph.nodes[i];
        if (node.ignore) continue;
        
        nbSP ++;
        
        // on récupère les pixels du superpixel
        std::vector<std::pair<int, int>> pixelCoords;  // (x, y)
        for (int x = 0; x < nH; x++) {
            for (int y = 0; y < nW; y++) {
                int pixelIndex = x * nW + y;
                if (pixelToSuperpixel[pixelIndex] == i) {
                    pixelCoords.push_back(std::make_pair(x, y));
                }
            }
        }
        //
        //nbpixels += pixelCoords.size();
        //std::cout << "Nb de pixels récupéré :" << pixelCoords.size() << std::endl;

  
        int numPixels = pixelCoords.size();
        if (numPixels <= 1) {
            // si le superpixel n'a qu'un seul pixel, on le garde tel quel
            if (numPixels == 1) {
                int y = pixelCoords[0].first;
                int x = pixelCoords[0].second;
                result.at<cv::Vec3b>(y, x) = I1.at<cv::Vec3b>(y, x);
                std::cout << "Superpixel " << i << " a un seul pixel - conservé tel quel" << std::endl;
            }
            continue;
        }
        
        // ----------------- ok, testé --------------------------

/*         std::cout << "Création du graphe interne pour le superpixel " << i 
                << " avec " << numPixels << " pixels" << std::endl; */
        
        //  map pixel coord vers index dans la matrice
        std::map<std::pair<int, int>, int> coordToIndex;
        for (int p = 0; p < numPixels; p++) {
            coordToIndex[pixelCoords[p]] = p;
            //std::cout << "pixelCoords[p], : " << pixelCoords[p].first << ", "  <<pixelCoords[p].second << " pointe vers : " << p << std::endl;
        }
        
        // ----------------- ok, testé --------------------------

        // matrice d'adjacence ou [pixel i][ pixel j] = 1 s'ils sont voisins
        cv::Mat adjacencyMatrix = cv::Mat::zeros(numPixels, numPixels, CV_32F);
        
        // on remplis la matrice d'adjacence
        for (int p = 0; p < numPixels; p++) {
            //recup les coordonnées du pixel
            int x = pixelCoords[p].first;
            int y = pixelCoords[p].second;
            
            // voisins possibles
            std::vector<std::pair<int, int>> neighbors = {
                {x-1, y}, {x+1, y}, {x, y-1}, {x, y+1}
            };
            
            // pour chaque voisins possibles
            for (const auto& neighbor : neighbors) {
                // on cherche le voisin dans la map
                auto it = coordToIndex.find(neighbor);
                // si on l'a trouvé, 
                if (it != coordToIndex.end()) {
                    int neighborIdx = it->second;
                    //std::cout << p << " est voisin avec , " << neighborIdx << std::endl;
                    //std::cout << "coordToIndex[neighbor] : " << neighbor.first << ", "  << neighbor.second << " pointe vers : " << neighborIdx << std::endl;
                    //std::cout << "coordToIndex[p] : " << pixelCoords[p].first << ", "  << pixelCoords[p].second << " pointe vers : " << p << std::endl;
                    adjacencyMatrix.at<float>(p, neighborIdx) = 1.0f;
                }
            } 
        }

        // ----------------- ok, testé --------------------------
        
        // creation de la matrice de degré D
        cv::Mat degreeMatrix = cv::Mat::zeros(numPixels, numPixels, CV_32F);
        for (int i = 0; i < numPixels; i++) {
            //  pour le pixel i, on calcule le degré
            // on compte combien de voisins a le pixel i
            float degree = 0;
            for (int j = 0; j < numPixels; j++) {
                degree += adjacencyMatrix.at<float>(i, j);
            }
            degreeMatrix.at<float>(i, i) = degree;
        }

        // ----------------- ok, testé --------------------------
        
        // laplacien: L = D - A
        cv::Mat laplacian = degreeMatrix - adjacencyMatrix;

        // ----------------- ok, testé --------------------------
        

        // on recupère les valeurs propres et vecteurs propres
        cv::Mat eigenvalues, eigenvectors;
        cv::eigen(laplacian, eigenvalues, eigenvectors);
        
        // ----------------- pas testé mais devrait marché parce que truc opencv --------------------------

        // récupére les valeurs rgb des pixels 
        std::vector<cv::Vec3f> pixelValues(numPixels);
        for (int p = 0; p < numPixels; p++) {
            int x = pixelCoords[p].first;
            int y = pixelCoords[p].second;
            cv::Vec3b rgb = I1.at<cv::Vec3b>(x, y);
            pixelValues[p] = cv::Vec3f(rgb[0], rgb[1], rgb[2]);
            //std::cout << "pixelCoords[p] : " << pixelCoords[p].first << ", "  << pixelCoords[p].second << std::endl;
            //std::cout << "pixelValues[p] : " << pixelValues[p][0] << ", "  << pixelValues[p][1] << ", "  << pixelValues[p][2] << std::endl;
        }

        // ----------------- ok, testé --------------------------

        // 7. Projeter les données sur les vecteurs propres (transformation)
        cv::Mat signalR = cv::Mat::zeros(numPixels, 1, CV_32F);
        cv::Mat signalG = cv::Mat::zeros(numPixels, 1, CV_32F);
        cv::Mat signalB = cv::Mat::zeros(numPixels, 1, CV_32F);

        for (int p = 0; p < numPixels; p++) {
/*             std::cout << "pixelCoords[p] : " << pixelCoords[p].first << ", "  << pixelCoords[p].second << std::endl;
            std::cout << "pixelValues[p] : " << pixelValues[p][0] << ", "  << pixelValues[p][1] << ", "  << pixelValues[p][2] << std::endl; */
            signalR.at<float>(p, 0) = pixelValues[p][0];
            signalG.at<float>(p, 0) = pixelValues[p][1];
            signalB.at<float>(p, 0) = pixelValues[p][2];
/*             std::cout << "signalR[p] : " << signalR.at<float>(p, 0) << std::endl;
            std::cout << "signalG[p] : " << signalG.at<float>(p, 0) << std::endl;
            std::cout << "signalB[p] : " << signalB.at<float>(p, 0) << std::endl; */
        }

        // Transformation : ^f = U * f
        cv::Mat spectrumR = eigenvectors * signalR;
        cv::Mat spectrumG = eigenvectors * signalG;
        cv::Mat spectrumB = eigenvectors * signalB;

        // Conversion des spectres en int (CV_32S) pour simuler l'envoi de données
        cv::Mat spectrumR_int, spectrumG_int, spectrumB_int;
        spectrumR.convertTo(spectrumR_int, CV_32S);
        spectrumG.convertTo(spectrumG_int, CV_32S);
        spectrumB.convertTo(spectrumB_int, CV_32S);

        // Simuler la réception et reconvertir les spectres en CV_32F
        cv::Mat spectrumR_float, spectrumG_float, spectrumB_float;
        spectrumR_int.convertTo(spectrumR_float, CV_32F);
        spectrumG_int.convertTo(spectrumG_float, CV_32F);
        spectrumB_int.convertTo(spectrumB_float, CV_32F);

        // Calcul du signal reconstruit (transformation inverse)
        cv::Mat reconstructedSignalR = eigenvectors.t() * spectrumR_float;
        cv::Mat reconstructedSignalG = eigenvectors.t() * spectrumG_float;
        cv::Mat reconstructedSignalB = eigenvectors.t() * spectrumB_float;

        for (int p = 0; p < numPixels; p++) {
            int y = pixelCoords[p].first;
            int x = pixelCoords[p].second;
            int r = std::min(255, std::max(0, static_cast<int>(reconstructedSignalR.at<float>(p,0) + 0.5f)));
            int g = std::min(255, std::max(0, static_cast<int>(reconstructedSignalG.at<float>(p,0) + 0.5f)));
            int b = std::min(255, std::max(0, static_cast<int>(reconstructedSignalB.at<float>(p,0) + 0.5f)));
            
            result.at<cv::Vec3b>(y,x) = cv::Vec3b(r, g, b);
        }


        std::cout << "Graphe du superpixel " << i << " traité avec succès" << std::endl;
    }

    //std::cout << "Nb de pixels récupéré : "<< nbpixels << std::endl;

    // Ajouter après la boucle des superpixels:


    // Vérifier s'il y a des pixels non traités
    int nonProcessedPixels = 0;
    for (int y = 0; y < nH; y++) {
        for (int x = 0; x < nW; x++) {
            int pixelIndex = y * nW + x;
            int spID = pixelToSuperpixel[pixelIndex];
            
            if (spID < 0 || graph.nodes[spID].ignore) {
                // Ce pixel n'a pas été traité, utiliser la couleur originale
                result.at<cv::Vec3b>(y, x) = I1.at<cv::Vec3b>(y, x);
                nonProcessedPixels++;
            }
        }
    }
    std::cout << "Nombre de pixels non traités: " << nonProcessedPixels << std::endl;

    std::cout << "nb total de superpixels :" << nbSP << std::endl;

    
    std::cout << "PSNR entre l'image de base et SDGT : " << PSNR(I1, result) <<std::endl;

    std::string outputPath = std::string(imagePath);
    lastDot = outputPath.find_last_of('.');
    if (lastDot != std::string::npos) {
        outputPath = outputPath.substr(0, lastDot) + "_superpixels" + outputPath.substr(lastDot);
    } else {
        outputPath += "_SDGT.png";
    }
    cv::imwrite(outputPath, result);
    std::cout << "Superpixel visualization saved to: " << outputPath << std::endl;
    

    // faire 2.2 du pa

    return I1;
}