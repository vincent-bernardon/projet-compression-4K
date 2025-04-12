#include "slicdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QColorDialog>

SLICDialog::SLICDialog(QWidget *parent)
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

    // Option d'affichage des bordures
    checkBoxShowBorders = new QCheckBox("Afficher les bordures des superpixels");
    checkBoxShowBorders->setChecked(false); // Coché par défaut
    layout->addWidget(checkBoxShowBorders);

    layout->addWidget(new QLabel("Couleur des contours :"));
    colorButton = new SelectColorButton(this);
    colorButton->setColor(Qt::red); // Couleur par défaut
    colorButton->setText("Changer la couleur");
    layout->addWidget(colorButton);


    // Boutons OK et Fermer
    QPushButton *btnOk = new QPushButton("OK");
    QPushButton *btnClose = new QPushButton("Annuler");

    layout->addWidget(btnOk);
    layout->addWidget(btnClose);

    // Connexions des boutons
    connect(btnOk, &QPushButton::clicked, this, &SLICDialog::accept);
    connect(btnClose, &QPushButton::clicked, this, &SLICDialog::reject);
}

int SLICDialog::getSuperpixels() const
{
    return spinSuperpixels->value();
}

int SLICDialog::getCompacite() const
{
    return spinCompacite->value();
}

QColor SLICDialog::getColor() const
{
    return colorButton->getColor();
}

bool SLICDialog::showBorders() const
{
    return checkBoxShowBorders->isChecked();
}

