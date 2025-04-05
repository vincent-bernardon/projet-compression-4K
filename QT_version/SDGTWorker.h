#ifndef SDGTWORKER_H
#define SDGTWORKER_H

#include <QThread>
#include <QColor>
#include <opencv2/opencv.hpp>
#include "Superpixel.h"
#include "SDGT.h"

class SDGTWorker : public QThread
{
    Q_OBJECT

public:
    SDGTWorker(QObject *parent = nullptr);
    ~SDGTWorker();

    void setParameters(const char* inputPath, const char* outputPath,
                       int initialSuperpixels, int compactness,
                       int finalSuperpixels, float coefficient);
public slots:
    void cancel() {
        m_cancelled = true;
    }
    
protected:
    void run() override;

signals:
    void progressUpdated(int value);
    void statusUpdated(const QString &message);
    void finished(bool success);

private:
    bool processImage();

    std::string m_inputPath;
    std::string m_outputPath;
    int m_initialSuperpixels;
    int m_compactness;
    int m_finalSuperpixels;
    float m_coefficient;
    volatile bool m_cancelled;
};

#endif // SDGTWORKER_H
