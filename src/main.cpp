#include <opencv2/opencv.hpp>
#include <iostream>
#include "timer.hpp"
#include "ImageUtils.hpp"
#include "SLIC.hpp"

int main() {
    //lire une image
    char imagePath[250] = "../src/image/test4k.png";
    cv::Mat image = cv::imread(imagePath);

    char imageOut[250] = {0};
    char imageOutThreaded[250] = {0};

    //test SLIC
    std::cout<<">----------SLIC----------<"<<std::endl;
    Timer timer;
    SLIC(stringduplicate(imagePath), imageOut, 48000, 10); // 29.7 de PSNR avec ces paramètres sur l'image test4k.png
    timer.stop();
    std::cout << "imageOut: " << imageOut << std::endl;
    std::cout << "Time: " << timer.elapsed() << "s" << std::endl; 
    cv::Mat modifiedImage = cv::imread(imageOut);

    if(modifiedImage.empty() || image.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return 1;
    }

    std::cout << "PSNR: " << PSNR(image, modifiedImage) <<" dB"<< std::endl;


    return 0;
}