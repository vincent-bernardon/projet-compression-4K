#include <opencv2/opencv.hpp>
#include "SLIC.h"
#include "SDGT.h"
#include "Superpixel.h"
#include <QProgressDialog>
#include <QApplication>
#include <omp.h>
#include <atomic>
#include <QTimer>
#include "SDGT.h"
#include "SDGTWorker.h"
#include <QProgressDialog>
#include <QEventLoop>

bool SDGT(QWidget* parent, char* imagePath, char* imageOutPath, int K, int m, int reductionPercentage, float coeff) {
    // Créer une boîte de dialogue de progression
    QProgressDialog progressDialog("Initialisation...", "Annuler", 0, 100, parent);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(0);
    progressDialog.setValue(0);
    progressDialog.show();

    // Créer et configurer le worker thread
    SDGTWorker* worker = new SDGTWorker(parent);
    worker->setParameters(imagePath, imageOutPath, K, m, reductionPercentage, coeff);

    // Configurer les connections
    QObject::connect(worker, &SDGTWorker::progressUpdated, &progressDialog, &QProgressDialog::setValue);
    QObject::connect(worker, &SDGTWorker::statusUpdated, &progressDialog, &QProgressDialog::setLabelText);
    QObject::connect(&progressDialog, &QProgressDialog::canceled, worker, &SDGTWorker::cancel);


    // Boucle d'événements pour attendre la fin du thread
    QEventLoop loop;
    bool success = false;

    QObject::connect(worker, &SDGTWorker::finished, [&loop, &success](bool result) {
        success = result;
        loop.quit();
    });

    // Démarrer le thread
    worker->start();

    // Attendre que le traitement soit terminé
    loop.exec();

    // Nettoyage
    worker->wait();
    delete worker;

    // Afficher le résultat
    if (!success && !progressDialog.wasCanceled()) {
        std::cerr << "Erreur lors du traitement SDGT" << std::endl;
    }

    return success;
}

