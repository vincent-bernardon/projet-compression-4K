#ifndef SLIC_HPP
#define SLIC_HPP

#include <opencv2/opencv.hpp>
#include "Superpixel.h"
#include <vector>

#include <QProgressDialog>
#include <QWidget>

bool SLIC_RECURSIVE(const cv::Mat & I1, std::vector<Superpixel> & superpixels, std::vector<std::pair<float, int>> & pixels,
    int S, int m, int nH, int nW, QProgressDialog* progressDialog = nullptr, float initialDelta = 0);

bool SLIC(char* imagePath, char* imgOutName, int K, int m, bool drawBorders = false,
          const QColor& borderColor = QColor(Qt::black), QWidget* parent = nullptr);

cv::Mat SLICWithoutSaving(char* imagePath , int K, int m);
std::vector<Superpixel> computeSlicSuperpixels(char* imagePath, int K, int m, int& nbSuperpixelPerLines, int& nbSuperpixelPerCols);

#endif
