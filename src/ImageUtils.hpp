#ifndef IMAGEUTILS_HPP
#define IMAGEUTILS_HPP
#include <opencv2/opencv.hpp>
#include <cstring>


char* stringduplicate(const char* source);
double PSNR(const cv::Mat& I1, const cv::Mat& I2);
void transformeNomImage(char *cheminImage, const char *nouvelleExtension, const char *prefixe, const char *suffixe, char *resultat);

#endif