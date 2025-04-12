#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ImageUtils.h"
#include "SLIC.h"
#include "slicdialog.h"
#include "SDGT.h"
#include "sdgtdialog.h"
#include "customgraphicsview.h"
#include <QFileDialog>
#include <QPixmap>
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QWheelEvent>

void MainWindow::initializeGraphicsView()
{
    // Configuration des vues graphiques
    ui->graphicsViewImageInput->setRenderHint(QPainter::Antialiasing);
    ui->graphicsViewImageInput->setRenderHint(QPainter::SmoothPixmapTransform);
    ui->graphicsViewImageInput->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsViewImageInput->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsViewImageInput->setDragMode(QGraphicsView::ScrollHandDrag);

    ui->graphicsViewImageInput->viewport()->installEventFilter(this);

    ui->graphicsViewImageOutput->setRenderHint(QPainter::Antialiasing);
    ui->graphicsViewImageOutput->setRenderHint(QPainter::SmoothPixmapTransform);
    ui->graphicsViewImageOutput->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsViewImageOutput->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsViewImageOutput->setDragMode(QGraphicsView::ScrollHandDrag);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QString styleSheet = "QPushButton { font-size: 12pt; min-height: 30px; padding: 5px; } "
                        "QLabel { font-size: 11pt; } "
                        "QLineEdit { font-size: 11pt; height: 25px; } "
                        "QComboBox { font-size: 11pt; min-height: 25px; }";
    this->setStyleSheet(styleSheet);

    ui->setupUi(this);
    increaseFontSize(this, 2);
    connect(ui->btnSelectImage, &QPushButton::clicked, this, &MainWindow::onSelectImageClicked);
    connect(ui->btnSelectOutput, &QPushButton::clicked, this, &MainWindow::onSelectOutputFolder);
    connect(ui->btnSLIC, &QPushButton::clicked, this, &MainWindow::onRunSLIC);
    connect(ui->btnSDGT, &QPushButton::clicked, this, &MainWindow::onRunSDGT);

    // Remplacez les QGraphicsView par des CustomGraphicsView
    CustomGraphicsView* customViewInput = new CustomGraphicsView(this);
    CustomGraphicsView* customViewOutput = new CustomGraphicsView(this);

    // Copiez la géométrie et le nom des objets
    customViewInput->setObjectName(ui->graphicsViewImageInput->objectName());
    customViewInput->setGeometry(ui->graphicsViewImageInput->geometry());
    customViewOutput->setObjectName(ui->graphicsViewImageOutput->objectName());
    customViewOutput->setGeometry(ui->graphicsViewImageOutput->geometry());

    // Remplacez les widgets dans le layout
    QLayout* layoutInput = ui->graphicsViewImageInput->parentWidget()->layout();
    QLayout* layoutOutput = ui->graphicsViewImageOutput->parentWidget()->layout();

    if (layoutInput) {
        layoutInput->replaceWidget(ui->graphicsViewImageInput, customViewInput);
    }
    if (layoutOutput) {
        layoutOutput->replaceWidget(ui->graphicsViewImageOutput, customViewOutput);
    }

    // Supprimez les anciens widgets
    delete ui->graphicsViewImageInput;
    delete ui->graphicsViewImageOutput;

    // Mettez à jour les pointeurs
    ui->graphicsViewImageInput = customViewInput;
    ui->graphicsViewImageOutput = customViewOutput;

    initializeGraphicsView();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSelectImageClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Sélectionner une image"), "", tr("Images (*.png *.jpg *.bmp)"));
    if (!filePath.isEmpty()) {
        showInputImage(filePath);
        imagePath = filePath;
    }
}

void MainWindow::onSelectOutputFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Choisir un dossier de sortie"));
    qDebug() << "Dossier sélectionné:" << folderPath;
    if (!folderPath.isEmpty()) {
        ui->lineEditOutputPath->setText(folderPath);
    }
}

void MainWindow::onRunSLIC()
{
    QString folderPath = ui->lineEditOutputPath->text();

    if (imagePath.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez d'abord sélectionner une image.");
        return;
    }

    if (folderPath.isEmpty() || folderPath == "Dossier de sortie") {
        QMessageBox::warning(this, "Erreur", "Veuillez choisir un dossier de sortie.");
        return;
    }

    qDebug() << "Compression SLIC lancée ";
    qDebug() << "Image sélectionnée :" << imagePath;
    qDebug() << "Dossier de sortie :" << folderPath;

    SLICDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        resetZooms();
        // Récupérer les paramètres choisis
        int superpixels = dlg.getSuperpixels();
        int compacite = dlg.getCompacite();
        bool showBorders = dlg.showBorders();
        QColor borderColor = dlg.getColor();

        QByteArray byteArray = imagePath.toUtf8();  // Convertit en QByteArray (byte array)
        char* imagePathPtr = byteArray.data();

        QFileInfo fileInfo(imagePath);
        QString baseName = fileInfo.completeBaseName();  // Nom sans extension
        QString extension = fileInfo.suffix();           // Extension de l'image (par exemple "jpg")

        // Créer la nouvelle chaîne avec le format spécifié
        QString imageOutPath = folderPath + "/" + baseName + "_SLIC_" + QString::number(superpixels)
                               + "_m" + QString::number(compacite) + "." + extension;

        // Convertir en QByteArray puis char* si nécessaire
        QByteArray byteArray2 = imageOutPath.toUtf8();
        char* imageOutPathPtr = byteArray2.data();

        // Affichage du chemin de sortie pour vérification
        qDebug() << "Chemin de sortie:" << imageOutPath;


        qDebug() << "Superpixels:" << superpixels;
        qDebug() << "Compacité:" << compacite;
        bool done = SLIC(imagePathPtr, imageOutPathPtr, superpixels, compacite, showBorders, borderColor, this);
        if (done){
            showFinalImage(imageOutPathPtr);
            double psnr = PSNR(imagePathPtr, imageOutPathPtr);
            ui->labelPSNRValue->setText(QString::number(psnr, 'd', 2) + " dB");
            QFileInfo originalInfo(imagePath);
            QFileInfo compressedInfo(imageOutPath);

            qint64 originalSize = originalInfo.size();
            qint64 compressedSize = compressedInfo.size();

            if (originalSize > 0) {
                double compressionRate = (double)originalSize/compressedSize;
                ui->labelCompressionValue->setText(QString::number(compressionRate, 'f', 2));
            } else {
                ui->labelCompressionValue->setText("--");
            }

        }

    }


}

void MainWindow::onRunSDGT()
{
    QString folderPath = ui->lineEditOutputPath->text();

    if (imagePath.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez d'abord sélectionner une image.");
        return;
    }

    if (folderPath.isEmpty() || folderPath == "Dossier de sortie" ) {
        QMessageBox::warning(this, "Erreur", "Veuillez choisir un dossier de sortie.");
        return;
    }

    qDebug() << "Compression SGT lancée ";
    qDebug() << "Image sélectionnée :" << imagePath;
    qDebug() << "Dossier de sortie :" << folderPath;

    SDGTDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        resetZooms();
        // Récupérer les paramètres choisis
        int superpixels = dlg.getSuperpixels();
        int superpixelsFinal = dlg.getSuperpixelsFinal();
        int compacite = dlg.getCompacite();
        float coeff = dlg.getCoeff();

        QByteArray byteArray = imagePath.toUtf8();  // Convertit en QByteArray (byte array)
        char* imagePathPtr = byteArray.data();

        QFileInfo fileInfo(imagePath);
        QString baseName = fileInfo.completeBaseName();  // Nom sans extension
        QString extension = fileInfo.suffix();           // Extension de l'image (par exemple "jpg")

        // Créer la nouvelle chaîne avec le format spécifié
        QString imageOutPath = folderPath + "/" + baseName + "_SDGT_" + QString::number(superpixels * (1-superpixelsFinal))
                               + "_m" + QString::number(compacite) + "_c" + QString::number(coeff) + "." + extension;

        // Convertir en QByteArray puis char* si nécessaire
        QByteArray byteArray2 = imageOutPath.toUtf8();
        char* imageOutPathPtr = byteArray2.data();

        // Affichage du chemin de sortie pour vérification
        qDebug() << "Chemin de sortie:" << imageOutPath;

        bool done = SDGT(this, imagePathPtr, imageOutPathPtr, superpixels, compacite, superpixelsFinal, coeff);

        if(done){
            showFinalImage(imageOutPathPtr);
            double psnr = PSNR(imagePathPtr, imageOutPathPtr);
            ui->labelPSNRValue->setText(QString::number(psnr, 'd', 2) + " dB");
            QFileInfo originalInfo(imagePath);
            QFileInfo compressedInfo(imageOutPath);

            qint64 originalSize = originalInfo.size();
            qint64 compressedSize = compressedInfo.size();

            if (originalSize > 0) {
                double compressionRate = (double)originalSize / compressedSize;
                ui->labelCompressionValue->setText(QString::number(compressionRate, 'f', 2) );
            } else {
                ui->labelCompressionValue->setText("--");
            }

        }

    }
}

void MainWindow::showInputImage(const QString &imagePath) {
    QGraphicsScene* scene = new QGraphicsScene();
    QPixmap pixmap(imagePath);
    scene->addPixmap(pixmap);
    ui->graphicsViewImageInput->setScene(scene);
    ui->graphicsViewImageInput->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    safeFit(ui->graphicsViewImageOutput);
}

void MainWindow::showFinalImage(const QString &imagePath) {
    QGraphicsScene* scene = new QGraphicsScene();
    QPixmap pixmap(imagePath);
    scene->addPixmap(pixmap);
    ui->graphicsViewImageOutput->setScene(scene);
    ui->graphicsViewImageOutput->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    safeFit(ui->graphicsViewImageInput);

}

void MainWindow::safeFit(QGraphicsView* view) {
    if (view && view->scene() && !view->scene()->items().isEmpty()) {
        view->fitInView(view->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
}

void MainWindow::resetZooms() {
    ui->graphicsViewImageInput->resetTransform();
    ui->graphicsViewImageOutput->resetTransform();
}


bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched == ui->graphicsViewImageInput->viewport() ||
         watched == ui->graphicsViewImageOutput->viewport()) &&
        event->type() == QEvent::Wheel) {

        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        QGraphicsView *view = qobject_cast<QGraphicsView*>(watched->parent());

        if (view) {
            if (wheelEvent->angleDelta().y() > 0) {
                view->scale(1.2, 1.2);  // Zoom avant
            } else {
                view->scale(0.8, 0.8);  // Zoom arrière
            }
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}


void MainWindow::increaseFontSize(QWidget *widget, int increment)
{
    QFont font = widget->font();
    font.setPointSize(font.pointSize() + increment);
    widget->setFont(font);
    
    // Appliquer récursivement à tous les widgets enfants
    for (QObject *child : widget->children()) {
        QWidget *childWidget = qobject_cast<QWidget*>(child);
        if (childWidget) {
            increaseFontSize(childWidget, increment);
        }
    }
}