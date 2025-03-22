#ifndef IMAGEUTILS_HPP
#define IMAGEUTILS_HPP
#include <opencv2/opencv.hpp>
#include <cstring>
#include "SLIC.hpp"


char* stringduplicate(const char* source);
double PSNR(const cv::Mat& I1, const cv::Mat& I2);
void transformeNomImage(char *cheminImage, const char *nouvelleExtension, const char *prefixe, const char *suffixe, char *resultat);
void traceCourbesPSNRSuperpixels(char *imagePath);
void traceCourbesPSNRCompacite(char *imagePath);
void traceCourbesPSNRSuperpixelsAVG(std::vector<std::string> imagePaths);
void traceCourbesTauxSuperpixelsAVG(std::vector<std::string> imagePaths);
void getAllImagesInFolder(std::string folderPath, std::vector<std::string>& imagePaths);

#endif