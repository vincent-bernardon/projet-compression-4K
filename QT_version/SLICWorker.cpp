#include "SLICWorker.h"

#define SLIC_THRESHOLD 2.0

SLICWorker::SLICWorker(QObject *parent)
    : QThread(parent), m_cancelled(false)
{
}

SLICWorker::~SLICWorker()
{
    m_cancelled = true;
    wait(); // Attendre que le thread se termine
}

void SLICWorker::setParameters(const char *inputPath, const char *outputPath,
                               int superpixels, int compactness,
                               bool drawBorders, const QColor &borderColor)
{
    m_inputPath = inputPath;
    m_outputPath = outputPath;
    m_superpixels = superpixels;
    m_compactness = compactness;
    m_drawBorders = drawBorders;
    m_borderColor = borderColor;
}

void SLICWorker::run()
{
    bool success = processImage();
    emit finished(success);
}

bool SLICWorker::processImage()
{
    emit statusUpdated("Chargement de l'image...");
    emit progressUpdated(1);

    cv::Mat image = cv::imread(m_inputPath);
    if (image.empty()) {
        emit statusUpdated("Erreur: Impossible de charger l'image");
        return false;
    }

    int nH = image.rows;
    int nW = image.cols;

    emit statusUpdated("Initialisation des superpixels...");
    emit progressUpdated(5);

    int S = round(sqrt(nW * nH / m_superpixels));
    std::vector<Superpixel> superpixels;
    int nS = S/2;

    for(int i = nS; i < nH; i+=S) {
        for(int j = nS; j < nW; j+=S) {
            Superpixel sp;
            sp.x = round(i);
            sp.y = round(j);
            sp.nbPixels = 0;
            sp.rgb = image.at<cv::Vec3b>(sp.x, sp.y);
            superpixels.push_back(sp);
        }
    }

    std::vector<std::pair<float, int>> pixels(nH * nW, std::make_pair(std::numeric_limits<float>::infinity(), -1));

    emit statusUpdated("Calcul des superpixels en cours...");
    float initialDelta = 0.0f;

    bool converged = performSLIC(image, superpixels, pixels, S, m_compactness, nH, nW, initialDelta);

    if (m_cancelled) return false;

    emit statusUpdated("Création de l'image finale...");
    emit progressUpdated(90);

    cv::Mat resultImage = image.clone();

    for(int i = 0; i < nH; i++) {
        for(int j = 0; j < nW; j++) {
            int sp_idx = pixels[i * nW + j].second;
            resultImage.at<cv::Vec3b>(i, j) = superpixels[sp_idx].rgb;
        }

        if (i % 20 == 0) {
            if (m_cancelled) return false;
            emit progressUpdated(90 + (i * 5) / nH);
        }
    }

    if (m_drawBorders) {
        emit statusUpdated("Dessin des bordures...");
        emit progressUpdated(95);

        cv::Vec3b cvBorderColor(m_borderColor.blue(), m_borderColor.green(), m_borderColor.red());
        for(int i = 1; i < nH-1; i++) {
            for(int j = 1; j < nW-1; j++) {
                int current = pixels[i * nW + j].second;

                if (pixels[(i-1) * nW + j].second != current ||
                    pixels[(i+1) * nW + j].second != current ||
                    pixels[i * nW + j-1].second != current ||
                    pixels[i * nW + j+1].second != current) {
                    resultImage.at<cv::Vec3b>(i, j) = cvBorderColor;
                }
            }

            if (i % 20 == 0) {
                if (m_cancelled) return false;
                emit progressUpdated(95 + (i * 5) / nH);
            }
        }
    }

    emit statusUpdated("Enregistrement de l'image...");
    emit progressUpdated(99);

    cv::imwrite(m_outputPath, resultImage);

    emit progressUpdated(100);
    emit statusUpdated("Traitement terminé");
    return true;
}

bool SLICWorker::performSLIC(const cv::Mat &image, std::vector<Superpixel> &superpixels,
                             std::vector<std::pair<float, int>> &pixels,
                             int S, int m, int nH, int nW, float initialDelta)
{
    float deltaMax = 0.0f;

    // Remplacer la récursion par une boucle
    while (!m_cancelled) {
        // pour chaque superpixel, on calcule la distance entre lui et les pixels de son voisinage
        int superpixelsSize = superpixels.size();
        for (size_t sp_idx = 0; sp_idx < superpixelsSize && !m_cancelled; sp_idx++) {
            Superpixel& sp = superpixels[sp_idx];

            int minI = std::max(0, sp.x-S);
            int maxI = std::min(nH, sp.x+S);
            int minJ = std::max(0, sp.y-S);
            int maxJ = std::min(nW, sp.y+S);

            for (int i = minI; i < maxI; i++) {
                for (int j = minJ; j < maxJ; j++) {
                    cv::Vec3b pixelColor = image.at<cv::Vec3b>(i, j);
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

        if (m_cancelled) return false;

        // vecteur pour calculer et stocker la nouvelle couleur des superpixels
        std::vector<std::pair<cv::Vec3f, int>> newSuperpixelValues(superpixels.size(),
                                                                   std::make_pair(cv::Vec3f(0.0f, 0.0f, 0.0f), 0));

        // pour chaque pixel, on récupère le superpixel associé et on ajoute sa couleur à la somme
        for (int i = 0; i < nH && !m_cancelled; i++) {
            for (int j = 0; j < nW; j++) {
                int sp_idx = pixels[i * nW + j].second;

                if (sp_idx >= 0 && sp_idx < superpixelsSize) {
                    cv::Vec3f color = (cv::Vec3f)image.at<cv::Vec3b>(i, j);
                    newSuperpixelValues[sp_idx].first += color;
                    newSuperpixelValues[sp_idx].second++;
                }
            }
        }

        if (m_cancelled) return false;

        // on divise la somme par le nombre de pixels pour obtenir la nouvelle couleur des superpixels
        for(auto & newSp : newSuperpixelValues) {
            if (newSp.second > 0) {
                newSp.first /= newSp.second;
            }
        }

        deltaMax = 0.0f;

        // on met à jour la couleur des superpixels avec celle calculée
        for (size_t sp_idx = 0; sp_idx < superpixelsSize; sp_idx++) {
            float delta = cv::norm(cv::Vec3f(superpixels[sp_idx].rgb) - newSuperpixelValues[sp_idx].first);
            deltaMax = std::max(deltaMax, delta);
            superpixels[sp_idx].rgb = newSuperpixelValues[sp_idx].first;
            superpixels[sp_idx].nbPixels = newSuperpixelValues[sp_idx].second;
        }

        // Mettre à jour la valeur initiale de deltaMax si nécessaire
        if (initialDelta <= 0) {
            initialDelta = deltaMax;
        }

        // Calculer et émettre la progression
        int progressValue = std::min(89, static_cast<int>((1.0f - std::min(1.0f, deltaMax/initialDelta)) * 90.0f));
        emit progressUpdated(progressValue);

        // si la différence est inférieure au seuil, on arrête
        if (deltaMax < SLIC_THRESHOLD) {
            return true;
        }
    }

    return false;
}
