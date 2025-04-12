#include "sdgtdialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

SDGTDialog::SDGTDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Paramètres SLIC");

    QVBoxLayout *layout = new QVBoxLayout(this);

    // Paramètre Superpixels
    layout->addWidget(new QLabel("Choisir un nombre de Superpixels :"));
    spinSuperpixels = new QSpinBox();
    spinSuperpixels->setRange(1, 1000000);  // Limites des superpixels
    spinSuperpixels->setValue(10000);       // Valeur par défaut
    layout->addWidget(spinSuperpixels);

    // Paramètre Compacité
    layout->addWidget(new QLabel("Choisir la compacité :"));
    spinCompacite = new QSpinBox();
    spinCompacite->setRange(1, 1000);   // Limites de la compacité
    spinCompacite->setValue(40);      // Valeur par défaut
    layout->addWidget(spinCompacite);

    // Paramètre Superpixels Finaux
    layout->addWidget(new QLabel("Choisir un pourcentage de reduction du nombre de superpixels :"));
    spinSuperpixelsFinal = new QSpinBox();
    spinSuperpixelsFinal->setRange(0, 100);  // Limites des superpixels
    spinSuperpixelsFinal->setValue(0);       // Valeur par défaut
    layout->addWidget(spinSuperpixelsFinal);

    // Paramètre Superpixels Finaux
    layout->addWidget(new QLabel("Choisir le coefficient de quantification :"));
    spinCoeff = new QSpinBox();
    spinCoeff->setRange(1, 1000);  // Limites des superpixels
    spinCoeff->setValue(20);       // Valeur par défaut
    layout->addWidget(spinCoeff);

    // Boutons OK et Fermer
    QPushButton *btnOk = new QPushButton("OK");
    QPushButton *btnClose = new QPushButton("Annuler");

    layout->addWidget(btnOk);
    layout->addWidget(btnClose);

    // Connexions des boutons
    connect(btnOk, &QPushButton::clicked, this, &SDGTDialog::accept);
    connect(btnClose, &QPushButton::clicked, this, &SDGTDialog::reject);
}

int SDGTDialog::getSuperpixels() const
{
    return spinSuperpixels->value();
}

int SDGTDialog::getCompacite() const
{
    return spinCompacite->value();
}

int SDGTDialog::getSuperpixelsFinal() const
{
    return spinSuperpixelsFinal->value();
}

float SDGTDialog::getCoeff() const
{
    return spinCoeff->value();
}

