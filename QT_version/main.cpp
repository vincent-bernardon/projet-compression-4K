#include "mainwindow.h"

#include "ImageUtils.h"
#include <QApplication>
#include <QTimer>
#include "SDGT.h"
#include <opencv2/opencv.hpp>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFont appFont = QApplication::font();
    appFont.setPointSize(appFont.pointSize() + 2); // Augmente de 2 points
    QApplication::setFont(appFont);
    qputenv("QT_SCALE_FACTOR", "1.2");
    MainWindow w;
    w.show();
    QTimer::singleShot(5, &w, &QMainWindow::showMaximized);
    return a.exec();

    // // Chemin de l'image d'entrée
    // char imagePath[250] = "../../src/image/gladiator-2.jpg";
    
    // // Chemin de l'image de sortie
    // char outputPath[250] = "../../src/image/gladiator-2_sdgt.jpg";

    // // Si vous voulez exécuter sans l'interface Qt
    // // Vous devez créer une QApplication minimale pour utiliser la fonction Qt SDGT
    // QApplication a(argc, argv);
    // QWidget parentWidget;

    // bool success = SDGT(&parentWidget, imagePath, outputPath, 100000, 10,0 , 20);
    
    // if (success) {
    //     std::cout << "SDGT appliqué avec succès!" << std::endl;
    //     std::cout << "Image sauvegardée à: " << outputPath << std::endl;
    // } else {
    //     std::cerr << "Erreur lors de l'application de SDGT" << std::endl;
    // }

    // QApplication a(argc, argv);
    // QWidget parentWidget;

    // std::vector<std::string> imagePaths;
    // getAllImagesInFolder("../../src/image", imagePaths);

    // // afficher les paths des images 
    // for (std::string imagePath : imagePaths) {
    //     std::cout << imagePath << std::endl;
    // }

    // Compare for a specific K value

    // compareCompressionRatesCurves(imagePaths);

    // comparePSNRCurves(imagePaths);


    

    
    return 0;
    



}
