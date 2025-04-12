#ifndef SLICWORKER_H
#define SLICWORKER_H

#include <QThread>
#include <QColor>
#include <opencv2/opencv.hpp>
#include "Superpixel.h"

class SLICWorker : public QThread
{
    Q_OBJECT

public:
    SLICWorker(QObject *parent = nullptr);
    ~SLICWorker();

    void setParameters(const char* inputPath, const char* outputPath,
                       int superpixels, int compactness,
                       bool drawBorders, const QColor &borderColor);

protected:
    void run() override; // Méthode exécutée dans le thread

signals:
    void progressUpdated(int value);
    void statusUpdated(const QString &message);
    void finished(bool success);

private:
    bool processImage();
    bool performSLIC(const cv::Mat &image, std::vector<Superpixel> &superpixels,
                     std::vector<std::pair<float, int>> &pixels,
                     int S, int m, int nH, int nW, float initialDelta);

    std::string m_inputPath;
    std::string m_outputPath;
    int m_superpixels;
    int m_compactness;
    bool m_drawBorders;
    QColor m_borderColor;
    volatile bool m_cancelled;
};

#endif // SLICWORKER_H
