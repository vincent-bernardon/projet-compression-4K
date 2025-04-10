#include "SDGTWorker.h"
#include "SLIC.h"
#include <omp.h>
#include <atomic>
#include <opencv2/opencv.hpp>

SDGTWorker::SDGTWorker(QObject *parent)
    : QThread(parent), m_cancelled(false)
{
    // Optimisation : Définir le nombre de threads OpenMP
    int numThreads = omp_get_max_threads();
    omp_set_num_threads(numThreads);
}

SDGTWorker::~SDGTWorker()
{
    m_cancelled = true;
    wait();
}

void SDGTWorker::setParameters(const char *inputPath, const char *outputPath,
                               int initialSuperpixels, int compactness,
                               int finalSuperpixels, float coefficient)
{
    m_inputPath = inputPath;
    m_outputPath = outputPath;
    m_initialSuperpixels = initialSuperpixels;
    m_compactness = compactness;
    m_finalSuperpixels = finalSuperpixels;
    m_coefficient = coefficient;
}

void SDGTWorker::run()
{
    bool success = processImage();
    emit finished(success);
}

bool SDGTWorker::processImage()
{
    emit statusUpdated("Chargement de l'image...");
    emit progressUpdated(1);

    cv::Mat I1 = cv::imread(m_inputPath);
    int nH = I1.rows;
    int nW = I1.cols;

    std::cout << "taille de l'image : " << nH * nW << std::endl;

    if (I1.empty()) {
        std::cerr << "Could not open or find the image : " << m_inputPath << std::endl;
        return false;
    }

    emit statusUpdated("Segmentation de l'image avec SLIC en cours...");
    emit progressUpdated(5);

    int S = round(sqrt(nW * nH / m_initialSuperpixels));
    std::vector<Superpixel> superpixels;

    int nS = S/2;

    int nbSuperpixelPerLines = ceil((float)(nW - nS) / S);
    int nbSuperpixelPerCols = ceil((float)(nH - nS) / S);

    // Pré-allocation de la capacité du vecteur
    superpixels.reserve((nH/S) * (nW/S) + 100);

    for(int i = nS; i < nH; i+=S) {
        for(int j = nS; j < nW; j+=S) {
            if (m_cancelled) return false;

            Superpixel sp;
            sp.x = round(i);
            sp.y = round(j);
            sp.nbPixels = 0;
            sp.rgb = I1.at<cv::Vec3b>(sp.x, sp.y);
            superpixels.push_back(sp);
        }
    }

    emit statusUpdated("Calcul des superpixels SLIC...");
    emit progressUpdated(10);

    // Optimisation : utiliser un vecteur pré-alloué
    std::vector<std::pair<float, int>> pixels(nH * nW, std::make_pair(std::numeric_limits<float>::infinity(), -1));

    // Nous utilisons SLIC_RECURSIVE directement car il n'y a pas d'UI ici
    bool slicSuccess = SLIC_RECURSIVE(I1, superpixels, pixels, S, m_compactness, nH, nW, nullptr, 0);
    if (m_cancelled || !slicSuccess) return false;

    emit statusUpdated("SLIC terminé, préparation du graphe...");
    emit progressUpdated(30);

    // Optimisation : utiliser un vecteur pré-alloué pour pixelToSuperpixel
    std::vector<int> pixelToSuperpixel(nH * nW);
    
    // Optimisation : utiliser des blocs parallèles pour les conversions
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < nH * nW; i++) {
        pixelToSuperpixel[i] = pixels[i].second;
    }

    // Libérer la mémoire dès que possible
    pixels.clear();
    pixels.shrink_to_fit();

    // Calculer la couleur lab
    computeLAB(superpixels);

    // Créer graph
    Graph graph(superpixels, pixelToSuperpixel, nbSuperpixelPerLines, nbSuperpixelPerCols);

    int nbOfSuperPixelInGraph = superpixels.size();
    int newNbSp = (1-(m_finalSuperpixels / 100.0)) * nbOfSuperPixelInGraph;
    int totalReduction = nbOfSuperPixelInGraph - newNbSp;

    std::cout << "Nb super pixel in graph : " << nbOfSuperPixelInGraph << std::endl;
    std::cout << "Nb de superpixels final : " << newNbSp << std::endl;

    emit statusUpdated("Réduction du graphe en cours...");
    emit progressUpdated(35);

    int lastProgress = 35;
    // Optimisation : opération de fusion du graphe
    while(nbOfSuperPixelInGraph > newNbSp) {
        if (m_cancelled) return false;

        graph.mergeClosestSuperpixel(pixelToSuperpixel);
        nbOfSuperPixelInGraph--;

        int reducedSoFar = superpixels.size() - nbOfSuperPixelInGraph;
        int progressPercentage = 35 + static_cast<int>((reducedSoFar / static_cast<float>(totalReduction)) * 25.0f);

        if (progressPercentage > lastProgress) {
            emit progressUpdated(progressPercentage);
            lastProgress = progressPercentage;
        }
    }

    emit statusUpdated("Graph réduit, traitement des superpixels...");
    emit progressUpdated(60);

    // OPTIMISATION: Prétraitement pour identifier les pixels dans chaque superpixel
    std::vector<std::vector<std::pair<int, int>>> superpixelToPixels(graph.nodes.size());

    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < nH; y++) {
        for (int x = 0; x < nW; x++) {
            int pixelIndex = y * nW + x;
            int spID = pixelToSuperpixel[pixelIndex];
            
            if (spID >= 0 && spID < static_cast<int>(graph.nodes.size()) && !graph.nodes[spID].ignore) {
                #pragma omp critical
                {
                    superpixelToPixels[spID].push_back(std::make_pair(y, x));
                }
            }
        }
    }

    // Trouver le superpixel avec le plus de pixels
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

    if(maxPixelCount > 20000) {
        std::cout << "Trop de pixels dans un graphe, augmenter le nombre de superpixels" << std::endl;
        return false;
    }

    cv::Mat result = I1.clone();
    float coeffInv = 1/m_coefficient;

    // Variables atomiques pour progression thread-safe
    std::atomic<unsigned int> nbSP(0);
    std::atomic<bool> cancelRequested(false);

    // Traitement des superpixels en parallèle avec vérification d'annulation régulière
    #pragma omp parallel for schedule(dynamic, 1) shared(cancelRequested, nbSP, superpixelToPixels)
    for (size_t i = 0; i < graph.nodes.size(); i++) {
        // Vérification plus fréquente de l'annulation
        if (m_cancelled || cancelRequested) {
            // Signal pour interrompre d'autres threads
            cancelRequested = true;
            continue;
        }

        const GraphNode& node = graph.nodes[i];
        if (node.ignore) continue;

        nbSP++;
        
        // OPTIMISÉ : Utiliser les pixels pré-calculés pour chaque superpixel
        const std::vector<std::pair<int, int>>& pixelCoords = superpixelToPixels[i];
        int numPixels = pixelCoords.size();
        
        if (numPixels <= 1) {
            if (numPixels == 1) {
                int y = pixelCoords[0].first;
                int x = pixelCoords[0].second;
                result.at<cv::Vec3b>(y, x) = I1.at<cv::Vec3b>(y, x);
            }
            continue;
        }

        // Vérification d'annulation pour les superpixels volumineux
        if (numPixels > 1000 && (m_cancelled || cancelRequested)) {
            cancelRequested = true;
            continue;
        }
        
        // OPTIMISATION : Calculer les limites du superpixel pour une matrice plus petite
        int minX = nW, minY = nH, maxX = 0, maxY = 0;
        for (const auto& coord : pixelCoords) {
            minY = std::min(minY, coord.first);
            maxY = std::max(maxY, coord.first);
            minX = std::min(minX, coord.second);
            maxX = std::max(maxX, coord.second);
        }
        
        int width = maxX - minX + 1;
        int height = maxY - minY + 1;

        // OPTIMISATION : Créer une coordMap optimisée pour la région du superpixel
        cv::Mat coordMap = cv::Mat::ones(height, width, CV_32S) * -1;
        for (int p = 0; p < numPixels; p++) {
            int y = pixelCoords[p].first - minY;
            int x = pixelCoords[p].second - minX;
            if (y >= 0 && y < height && x >= 0 && x < width) {
                coordMap.at<int>(y, x) = p;
            }
        }

        // Création d'une matrice d'adjacence optimisée
        cv::Mat adjacencyMatrix = cv::Mat::zeros(numPixels, numPixels, CV_32F);
        
        for (int p = 0; p < numPixels; p++) {
            int y = pixelCoords[p].first;
            int x = pixelCoords[p].second;

            // Vérifier les 4 voisins (simplification pour la performance)
            std::vector<std::pair<int, int>> neighbors = {
                {y-1, x}, {y+1, x}, {y, x-1}, {y, x+1}
            };

            for (const auto& neighbor : neighbors) {
                int ny = neighbor.first;
                int nx = neighbor.second;
                
                if (ny >= minY && ny < minY + height && nx >= minX && nx < minX + width) {
                    int neighborIdx = coordMap.at<int>(ny - minY, nx - minX);
                    if (neighborIdx >= 0) {
                        adjacencyMatrix.at<float>(p, neighborIdx) = 1.0f;
                    }
                }
            }
        }

        // OPTIMISATION : Calculer les degrés avec une boucle vectorisable
        cv::Mat degreeMatrix = cv::Mat::zeros(numPixels, numPixels, CV_32F);
        std::vector<float> degrees(numPixels, 0.0f);
        
        for (int i = 0; i < numPixels; i++) {
            float degree = 0.0f;
            const float* row = adjacencyMatrix.ptr<float>(i);
            for (int j = 0; j < numPixels; j++) {
                degree += row[j];
            }
            degrees[i] = degree;
            degreeMatrix.at<float>(i, i) = degree;
        }

        // Laplacien: L = D - A
        cv::Mat laplacian = degreeMatrix - adjacencyMatrix;

        // OPTIMISATION : Calculer uniquement les vecteurs propres nécessaires
        cv::Mat eigenvalues, eigenvectors;
        cv::eigen(laplacian, eigenvalues, eigenvectors);

        cv::Mat eigenvectorsT;
        cv::transpose(eigenvectors, eigenvectorsT);

        // Récupérer les valeurs RGB des pixels avec vectorisation
        std::vector<cv::Vec3f> pixelValues(numPixels);
        for (int p = 0; p < numPixels; p++) {
            int y = pixelCoords[p].first;
            int x = pixelCoords[p].second;
            cv::Vec3b rgb = I1.at<cv::Vec3b>(y, x);
            pixelValues[p] = cv::Vec3f(rgb[0], rgb[1], rgb[2]);
        }

        // OPTIMISATION : Création et calcul optimisé des signaux RGB
        cv::Mat signalR(numPixels, 1, CV_32F);
        cv::Mat signalG(numPixels, 1, CV_32F);
        cv::Mat signalB(numPixels, 1, CV_32F);

        //non fonctionnel
        
        // float* ptrR = signalR.ptr<float>(0);
        // float* ptrG = signalG.ptr<float>(0);
        // float* ptrB = signalB.ptr<float>(0);

        // for (int p = 0; p < numPixels; p++) {
        //     ptrR[p] = round(pixelValues[p][0] * coeffInv) * m_coefficient;
        //     ptrG[p] = round(pixelValues[p][1] * coeffInv) * m_coefficient;
        //     ptrB[p] = round(pixelValues[p][2] * coeffInv) * m_coefficient;
        // }

        // // AJOUT: Point d'interruption supplémentaire dans les calculs complexes
        // if (eigenvectors.rows > 50 && (m_cancelled || cancelRequested)) {
        //     cancelRequested = true;
        //     continue;
        // }

        // // Remplacez la quantification existante par celle-ci

        // // 1. Créer les signaux sans quantification
        // for (int p = 0; p < numPixels; p++) {
        //     signalR.at<float>(p, 0) = pixelValues[p][0];
        //     signalG.at<float>(p, 0) = pixelValues[p][1];
        //     signalB.at<float>(p, 0) = pixelValues[p][2];
        // }

        // // 2. Transformation dans le domaine spectral (code existant)
        // cv::Mat spectrumR = eigenvectors * signalR;
        // cv::Mat spectrumG = eigenvectors * signalG;
        // cv::Mat spectrumB = eigenvectors * signalB;

        // // 3. NOUVEAU: Appliquer la quantification inspirée JPEG dans le domaine spectral
        // for (int i = 0; i < numPixels; i++) {
        //     // Plus l'indice est élevé, plus on quantifie fortement (fréquences hautes)
        //     float quantFactor = std::max(1.0f, m_coefficient * (1.0f + i * 0.5f / numPixels));
            
        //     // Quantification: division, arrondi, multiplication
        //     spectrumR.at<float>(i, 0) = round(spectrumR.at<float>(i, 0) / quantFactor) * quantFactor;
        //     spectrumG.at<float>(i, 0) = round(spectrumG.at<float>(i, 0) / quantFactor) * quantFactor;
        //     spectrumB.at<float>(i, 0) = round(spectrumB.at<float>(i, 0) / quantFactor) * quantFactor;
        // }

        // // 4. Calcul du signal reconstruit (code existant)
        // cv::Mat reconstructedSignalR = eigenvectorsT * spectrumR;
        // cv::Mat reconstructedSignalG = eigenvectorsT * spectrumG;
        // cv::Mat reconstructedSignalB = eigenvectorsT * spectrumB;


        for (int p = 0; p < numPixels; p++) {
            signalR.at<float>(p, 0) = pixelValues[p][0];
            signalG.at<float>(p, 0) = pixelValues[p][1];
            signalB.at<float>(p, 0) = pixelValues[p][2];
        }

        // DCT
        cv::dct(signalR, signalR);
        cv::dct(signalG, signalG);
        cv::dct(signalB, signalB);

        for (int p = 0; p < numPixels; p++) {
            signalR.at<float>(p, 0) = round(signalR.at<float>(p, 0) / m_coefficient) * m_coefficient;
            signalG.at<float>(p, 0) = round(signalG.at<float>(p, 0) / m_coefficient) * m_coefficient;
            signalB.at<float>(p, 0) = round(signalB.at<float>(p, 0) / m_coefficient) * m_coefficient;
        }

        // IDCT
        cv::idct(signalR, signalR);
        cv::idct(signalG, signalG);
        cv::idct(signalB, signalB);

        cv::Mat reconstructedSignalR = signalR;
        cv::Mat reconstructedSignalG = signalG;
        cv::Mat reconstructedSignalB = signalB;

        // OPTIMISATION : mise à jour de l'image résultat avec accès direct aux données
        for (int p = 0; p < numPixels; p++) {
            int y = pixelCoords[p].first;
            int x = pixelCoords[p].second;
            int r = std::min(255, std::max(0, static_cast<int>(reconstructedSignalR.at<float>(p,0) + 0.5f)));
            int g = std::min(255, std::max(0, static_cast<int>(reconstructedSignalG.at<float>(p,0) + 0.5f)));
            int b = std::min(255, std::max(0, static_cast<int>(reconstructedSignalB.at<float>(p,0) + 0.5f)));

            result.at<cv::Vec3b>(y,x) = cv::Vec3b(r, g, b);
            
            // AJOUT: Point d'interruption dans la boucle de mise à jour des pixels
            if (m_cancelled || cancelRequested) {
                cancelRequested = true;
                continue;
            }
        }

        // AJOUT: Point d'interruption supplémentaire pour l'écriture des résultats
        if (m_cancelled || cancelRequested) {
            cancelRequested = true;
            continue;
        }

        // Signaler la progression de manière moins fréquente
        if (i % 20 == 0) {
            float progress = (static_cast<float>(nbSP) / static_cast<float>(newNbSp));
            int progressValue = 60 + static_cast<int>(progress * 35.0f);
            emit progressUpdated(std::min(95, progressValue));
        }
    }

    // Vérifier si le traitement a été annulé après la boucle parallèle
    if (m_cancelled || cancelRequested) {
        emit statusUpdated("Opération annulée");
        return false;
    }

    if (m_cancelled) return false;

    // Traiter les pixels non assignés
    int nonProcessedPixels = 0;
    
    // OPTIMISATION : utiliser des blocs pour le traitement final
    const int BLOCK_SIZE = 64;
    #pragma omp parallel for reduction(+:nonProcessedPixels) schedule(dynamic)
    for (int yBlock = 0; yBlock < nH; yBlock += BLOCK_SIZE) {
        for (int xBlock = 0; xBlock < nW; xBlock += BLOCK_SIZE) {
            int yMax = std::min(yBlock + BLOCK_SIZE, nH);
            int xMax = std::min(xBlock + BLOCK_SIZE, nW);
            
            for (int y = yBlock; y < yMax; y++) {
                for (int x = xBlock; x < xMax; x++) {
                    int pixelIndex = y * nW + x;
                    int spID = pixelToSuperpixel[pixelIndex];

                    if (spID < 0 || spID >= static_cast<int>(graph.nodes.size()) || graph.nodes[spID].ignore) {
                        result.at<cv::Vec3b>(y, x) = I1.at<cv::Vec3b>(y, x);
                        nonProcessedPixels++;
                    }
                }
            }
        }
    }

    std::cout << "Nombre de pixels non traités: " << nonProcessedPixels << std::endl;
    std::cout << "PSNR entre l'image de base et SDGT : " << PSNR(I1, result) << std::endl;

    emit statusUpdated("Enregistrement de l'image...");
    emit progressUpdated(98);

    cv::imwrite(m_outputPath, result);
    std::cout << "Image sauvegardée: " << m_outputPath << std::endl;

    emit progressUpdated(100);
    emit statusUpdated("Traitement terminé");
    return true;
}
