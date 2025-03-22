#ifndef SLIC_HPP
#define SLIC_HPP

#include <opencv2/opencv.hpp>
#include "Superpixel.hpp"
#include <vector>
#include <iostream>
#include "ImageUtils.hpp"


void SLIC_RECURSIVE(const cv::Mat & I1, std::vector<Superpixel> & superpixels, std::vector<std::pair<float, int>> & pixels, int S, int m, int nH, int nW);
void SLIC(char* imagePath , char* imgOutName, int K, int m);
cv::Mat SLICWithoutSaving(char* imagePath , int K, int m);
std::vector<Superpixel> computeSlicSuperpixels(char* imagePath, int K, int m, int& nbSuperpixelPerLines, int& nbSuperpixelPerCols);

#endif