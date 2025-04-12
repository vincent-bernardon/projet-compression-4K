#ifndef SLICDIALOG_H
#define SLICDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QColor>
#include <QCheckBox>
#include "selectorcolorbutton.h"

class SLICDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SLICDialog(QWidget *parent = nullptr);
    int getSuperpixels() const;
    int getCompacite() const;
    QColor getColor() const;
    bool showBorders() const;


    signals:
        void parametersSelected(int superpixels, int compacite);

    private:
        QSpinBox *spinSuperpixels;
        QSpinBox *spinCompacite;
        SelectColorButton *colorButton;
        QCheckBox *checkBoxShowBorders;
    };
#endif // SLICDIALOG_H
