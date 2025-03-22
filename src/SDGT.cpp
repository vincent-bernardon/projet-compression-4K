#include <opencv2/opencv.hpp>
#include "SLIC.hpp"
#include "SDGT.hpp"
#include "Superpixel.hpp"

/* 
void testRGBtoLABtoRGB() {
    Superpixel testPixel;
    
    // Choose a test RGB value
    testPixel.rgb = cv::Vec3b(128, 64, 192);
    cv::Vec3b originalRGB = testPixel.rgb;
    
    std::cout << "=== Testing RGB->LAB->RGB Conversion ===" << std::endl;
    std::cout << "Original RGB: [" 
              << static_cast<int>(originalRGB[0]) << ", " 
              << static_cast<int>(originalRGB[1]) << ", " 
              << static_cast<int>(originalRGB[2]) << "]" << std::endl;
    
    // Convert to LAB
    testPixel.setLAB();
    std::cout << "LAB values: [" 
              << std::fixed << std::setprecision(2) << testPixel.lab[0] << ", " 
              << std::fixed << std::setprecision(2) << testPixel.lab[1] << ", " 
              << std::fixed << std::setprecision(2) << testPixel.lab[2] << "]" << std::endl;
    
    // Convert back to RGB
    testPixel.setRGBfromLAB();
    std::cout << "Converted RGB: [" 
              << static_cast<int>(testPixel.rgb[0]) << ", " 
              << static_cast<int>(testPixel.rgb[1]) << ", " 
              << static_cast<int>(testPixel.rgb[2]) << "]" << std::endl;
    
    // Calculate difference
    int diffR = std::abs(static_cast<int>(originalRGB[0]) - static_cast<int>(testPixel.rgb[0]));
    int diffG = std::abs(static_cast<int>(originalRGB[1]) - static_cast<int>(testPixel.rgb[1]));
    int diffB = std::abs(static_cast<int>(originalRGB[2]) - static_cast<int>(testPixel.rgb[2]));
    
    std::cout << "Difference: [" << diffR << ", " << diffG << ", " << diffB << "]" << std::endl;
    std::cout << "=======================================" << std::endl;
}
 */

cv::Mat SDGT(char* imagePath , int K = 100, int m = 10, int mp = 10){
    if(K < mp){
        std::cout << "nombre de superpixels inférieur au nouveau nombre" << std::endl;
        exit(1);
    }


    cv::Mat I1 = cv::imread(imagePath);

    int nH = I1.rows;
    int nW = I1.cols;

    std::cout << "taille de l'image : " << nH * nW << std::endl;

    //vérifier si l'image est chargée correctement
    if (I1.empty()) {
        std::cerr << "Could not open or find the image : %d"<< imagePath << std::endl;
        exit(1);
    }
    // std::cout << "Image chargée avec succès" << std::endl;

    int S = round(sqrt(nW * nH / K));
    std::vector<Superpixel> superpixels; 

    int nS = S/2;

    int nbSuperpixelPerLines = ceil((float)(nW - nS) / S);
    int nbSuperpixelPerCols = ceil((float)(nH - nS) / S);

    for(int i = nS; i < nH; i+=S) {
        for(int j = nS; j < nW; j+=S) {
            Superpixel sp;
            sp.x = round(i);
            sp.y = round(j);
            sp.nbPixels = 0;
            sp.rgb = I1.at<cv::Vec3b>(sp.x, sp.y);
            superpixels.push_back(sp);
        }
    }

    // vecteur pour stocker la distance entre chaque pixel et le superpixel associé
    std::vector<std::pair<float, int>> pixels(nH * nW, std::make_pair(std::numeric_limits<float>::infinity(), -1));

    SLIC_RECURSIVE(I1, superpixels, pixels, S, m, nH, nW);

    cv::Mat slicResult = I1.clone();


    //vecteur pour stocker a quel superpixel le pixel est associé (sans la distance)
    std::vector<int> pixelToSuperpixel(nH * nW);
    for (int i = 0; i < nH * nW; i++) {
        pixelToSuperpixel[i] = pixels[i].second;
    }
    
    pixels.clear();

    // Color each pixel with its superpixel's color
    for (int y = 0; y < nH; y++) {
        for (int x = 0; x < nW; x++) {
            int pixelIndex = y * nW + x;
            int spID = pixelToSuperpixel[pixelIndex];
                
            if (spID >= 0 && spID < superpixels.size()) {
                slicResult.at<cv::Vec3b>(y, x) = superpixels[spID].rgb;
            }
        }
    }

    std::cout << "PSNR entre l'image de base et slic : " << PSNR(I1, slicResult) <<std::endl;
    
    std::string slicOutputPath = std::string(imagePath);
    size_t lastDot = slicOutputPath.find_last_of('.');
    if (lastDot != std::string::npos) {
        slicOutputPath = slicOutputPath.substr(0, lastDot) + "_slic" + slicOutputPath.substr(lastDot);
    } else {
        slicOutputPath += "_slic.png";
    }
    cv::imwrite(slicOutputPath, slicResult);
    std::cout << "SLIC visualization saved to: " << slicOutputPath << std::endl;


    //calculer la couleur lab (passe de rgb à xyz à lab)
    computeLAB(superpixels);

    // créer graph
    Graph graph(superpixels, pixelToSuperpixel, nbSuperpixelPerLines, nbSuperpixelPerCols);
      
    std::cout << "\n=== Graph crée ===\n";

    int nbOfSuperPixelInGraph = superpixels.size() ;
    while( nbOfSuperPixelInGraph > mp){
        graph.mergeClosestSuperpixel(pixelToSuperpixel);
        nbOfSuperPixelInGraph--;
    }
    std::cout << "\n=== Graph réduit ===\n";

    int maxPixelCount = 0;
    int maxPixelSuperpixelID = -1;

    for (size_t i = 0; i < graph.nodes.size(); i++) {
        const GraphNode& node = graph.nodes[i];
        if (node.ignore) continue;
        
        if (node.superpixel->nbPixels > maxPixelCount) {
            maxPixelCount = node.superpixel->nbPixels;
            maxPixelSuperpixelID = i;
            node.superpixel->setRGBfromLAB();
        }
    }

    cv::Mat result = I1.clone();

    for (int y = 0; y < nH; y++) {
        for (int x = 0; x < nW; x++) {
            int pixelIndex = y * nW + x;
            int spID = pixelToSuperpixel[pixelIndex];
            result.at<cv::Vec3b>(y, x) = graph.nodes[spID].superpixel->rgb;
        }
    }
    
    std::cout << "PSNR entre l'image de base et SDGT : " << PSNR(I1, result) <<std::endl;

    std::string outputPath = std::string(imagePath);
    lastDot = outputPath.find_last_of('.');
    if (lastDot != std::string::npos) {
        outputPath = outputPath.substr(0, lastDot) + "_superpixels" + outputPath.substr(lastDot);
    } else {
        outputPath += "_superpixels.png";
    }
    cv::imwrite(outputPath, result);
    std::cout << "Superpixel visualization saved to: " << outputPath << std::endl;
    

    // faire 2.2 du pa

    return I1;
}