#include <opencv2/opencv.hpp>
#include <iostream>
#include "timer.hpp"
#include "ImageUtils.hpp"
#include "SLIC.hpp"
#include "SDGT.hpp"
#include "global.hpp"

int main() {
    //lire une image
    // char imagePath[250] = "../src/image/wolverine.png";
    // cv::Mat image = cv::imread(imagePath);

    // char imageOut[250] = {0};
    // char imageOutThreaded[250] = {0};
 
    //test SLIC
    std::cout<<">----------SLIC----------<"<<std::endl;
    Timer timer;
    //K c'est le nombre de superpixels
    //M c'est le compacité, autrement dis le poids de la distance spatiale par rapport à la distance de couleur
    // SLIC(stringduplicate(imagePath), imageOut, KMIN, 10); // 29.7 de PSNR avec ces paramètres sur l'image test4k.png
    // timer.stop();
    // std::cout << "imageOut: " << imageOut << std::endl;
    // std::cout << "Time: " << timer.elapsed() << "s" << std::endl; 
    // cv::Mat modifiedImage = cv::imread(imageOut);

    // if(modifiedImage.empty() || image.empty()) {
    //     std::cerr << "Could not open or find the image" << std::endl;
    //     return 1;
    // }

    // std::cout << "PSNR: " << PSNR(image, modifiedImage) <<" dB"<< std::endl;

    // traceCourbesPSNRSuperpixels(imagePath);
    // traceCourbesPSNRCompacite(imagePath);

    std::vector<std::string> imagePaths;
    getAllImagesInFolder("../src/image", imagePaths);

    //afficher les paths des images 
    // for (std::string imagePath : imagePaths) {
    //     std::cout << imagePath << std::endl;
    // }

    //généré les images pour faire les courbes moyenne de PSNR en fonction de K et de taux de compression en fonction de K
    // genererImageSLIC(imagePaths);

    //tracer la courbe des PSNR en fonction de K moyen
    // traceCourbesPSNRSuperpixelsAVG(imagePaths);

    //tracer la courbe des taux de compression en fonction de K moyen
    traceCourbesTauxSuperpixelsAVG(imagePaths);

    timer.stop();
    std::cout << "Time: " << timer.elapsed() << "s" << std::endl; 



/*     //char imagePath[250] = "../src/image/test/OIP.png";
    char imagePath[250] = "../src/image/sunrise-sunflowers.png";

    Timer timer;
    timer.reset();
    cv::Mat imageModified = SDGT(imagePath, 120000, 10, 75000);
    std::cout << " temps pris pour créer le graph : " << timer.elapsed() << std::endl; */

    // TEST DISTANCE LAB
/* 
    struct TestPair {
        cv::Vec3f color1;
        cv::Vec3f color2;
        float expected;
    };
    
    std::vector<TestPair> testPairs = {
        {{50.0000f, 2.6772f, -79.7751f}, {50.0000f, 0.0000f, -82.7485f}, 2.0425f},
        {{50.0000f, 3.1571f, -77.2803f}, {50.0000f, 0.0000f, -82.7485f}, 2.8615f},
        {{50.0000f, 2.8361f, -74.0200f}, {50.0000f, 0.0000f, -82.7485f}, 3.4412f},
        {{50.0000f, -1.3802f, -84.2814f}, {50.0000f, 0.0000f, -82.7485f}, 1.0000f},
        {{50.0000f, -1.1848f, -84.8006f}, {50.0000f, 0.0000f, -82.7485f}, 1.0000f},
        {{50.0000f, -0.9009f, -85.5211f}, {50.0000f, 0.0000f, -82.7485f}, 1.0000f},
        {{50.0000f, 0.0000f, 0.0000f}, {50.0000f, -1.0000f, 2.0000f}, 2.3669f},
        {{50.0000f, -1.0000f, 2.0000f}, {50.0000f, 0.0000f, 0.0000f}, 2.3669f},
        {{50.0000f, 2.4900f, -0.0010f}, {50.0000f, -2.4900f, 0.0009f}, 7.1792f},
        {{50.0000f, 2.4900f, -0.0010f}, {50.0000f, -2.4900f, 0.0010f}, 7.1792f},
        {{50.0000f, 2.4900f, -0.0010f}, {50.0000f, -2.4900f, 0.0011f}, 7.2195f},
        {{50.0000f, 2.4900f, -0.0010f}, {50.0000f, -2.4900f, 0.0012f}, 7.2195f},
        {{50.0000f, -0.0010f, 2.4900f}, {50.0000f, 0.0009f, -2.4900f}, 4.8045f},
        {{50.0000f, -0.0010f, 2.4900f}, {50.0000f, 0.0010f, -2.4900f}, 4.8045f},
        {{50.0000f, -0.0010f, 2.4900f}, {50.0000f, 0.0011f, -2.4900f}, 4.7461f},
        {{50.0000f, 2.5000f, 0.0000f}, {50.0000f, 0.0000f, -2.5000f}, 4.3065f},
        {{50.0000f, 2.5000f, 0.0000f}, {73.0000f, 25.0000f, -18.0000f}, 27.1492f},
        {{50.0000f, 2.5000f, 0.0000f}, {61.0000f, -5.0000f, 29.0000f}, 22.8977f},
        {{50.0000f, 2.5000f, 0.0000f}, {56.0000f, -27.0000f, -3.0000f}, 31.9030f},
        {{50.0000f, 2.5000f, 0.0000f}, {58.0000f, 24.0000f, 15.0000f}, 19.4535f},
        {{50.0000f, 2.5000f, 0.0000f}, {50.0000f, 3.1736f, 0.5854f}, 1.0000f},
        {{50.0000f, 2.5000f, 0.0000f}, {50.0000f, 3.2972f, 0.0000f}, 1.0000f},
        {{50.0000f, 2.5000f, 0.0000f}, {50.0000f, 1.8634f, 0.5757f}, 1.0000f},
        {{50.0000f, 2.5000f, 0.0000f}, {50.0000f, 3.2592f, 0.3350f}, 1.0000f},
        {{60.2574f, -34.0099f, 36.2677f}, {60.4626f, -34.1751f, 39.4387f}, 1.2644f},
        {{63.0109f, -31.0961f, -5.8663f}, {62.8187f, -29.7946f, -4.0864f}, 1.2630f},
        {{61.2901f, 3.7196f, -5.3901f}, {61.4292f, 2.2480f, -4.9620f}, 1.8731f},
        {{35.0831f, -44.1164f, 3.7933f}, {35.0232f, -40.0716f, 1.5901f}, 1.8645f},
        {{22.7233f, 20.0904f, -46.6940f}, {23.0331f, 14.9730f, -42.5619f}, 2.0373f},
        {{36.4612f, 47.8580f, 18.3852f}, {36.2715f, 50.5065f, 21.2231f}, 1.4146f},
        {{90.8027f, -2.0831f, 1.4410f}, {91.1528f, -1.6435f, 0.0447f}, 1.4441f},
        {{90.9257f, -0.5406f, -0.9208f}, {88.6381f, -0.8985f, -0.7239f}, 1.5381f},
        {{6.7747f, -0.2908f, -2.4247f}, {5.8714f, -0.0985f, -2.2286f}, 0.6377f},
        {{2.0776f, 0.0795f, -1.1350f}, {0.9033f, 0.0636f, -0.5514f}, 0.9082f}
    };
    
    for (const auto& pair : testPairs) {
        float result = getDistanceLAB(pair.color1, pair.color2, 1.0f, 1.0f, 1.0f);
        if (std::abs(result - pair.expected) > 0.01 ){
            std::cout << "Test: (" 
            << pair.color1[0] << "," << pair.color1[1] << "," << pair.color1[2] << ") vs ("
            << pair.color2[0] << "," << pair.color2[1] << "," << pair.color2[2] << ")\n"
            << "Résultat: " << result << ", Attendu: " << pair.expected 
            << ", Différence: " << std::abs(result - pair.expected) << std::endl;
        }

    }
  */

    return 0;
}