#ifndef IMAGEUTILS_HPP
#define IMAGEUTILS_HPP
#include <opencv2/opencv.hpp>
#include <cstring>
#include <QWidget>
char* stringduplicate(const char* source);
double PSNR(char* imagePath1, char* imagePath2);
double PSNR(const cv::Mat& I1, const cv::Mat& I2);
void transformeNomImage(char *cheminImage, const char *nouvelleExtension, const char *prefixe, const char *suffixe, char *resultat);
void traceCourbesPSNRSuperpixels(char *imagePath);
void traceCourbesPSNRCompacite(char *imagePath);
void traceCourbesPSNRSuperpixelsAVG(std::vector<std::string> &imagePaths);
void traceCourbesTauxSuperpixelsAVG(std::vector<std::string> &imagePaths);
void getAllImagesInFolder(std::string folderPath, std::vector<std::string>& imagePaths);
void genererImageSLIC(std::vector<std::string> &imagePaths);

void genererImageSDGT(std::vector<std::string> &imagePaths, int K, QWidget *parentWidget);
void compareCompressionRatesCurves(std::vector<std::string> &imagePaths);
void comparePSNRCurves(std::vector<std::string> &imagePaths);

void traceCourbesTauxSuperpixelsAVGCustom(std::vector<std::string> &imagePaths, const std::string& customTitle);

void traceCourbesPSNRCoeffSDGT(QWidget* parent, char* imagePath);
void traceCourbesTauxSDGTreductionPercentage(QWidget* parent, char* imagePath);
void traceCourbesPSNRSDGTreductionPercentage(QWidget* parent, char* imagePath);


#endif
