#ifndef SDGTDIALOG_H
#define SDGTDIALOG_H

#include <QDialog>
#include <QSpinBox>

class SDGTDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SDGTDialog(QWidget *parent = nullptr);
    int getSuperpixels() const;
    int getCompacite() const;
    int getSuperpixelsFinal() const;
    float getCoeff() const;


signals:
    void parametersSelected(int superpixels, int compacite);

private:
    QSpinBox *spinSuperpixels;
    QSpinBox *spinCompacite;
    QSpinBox *spinSuperpixelsFinal;
    QSpinBox *spinCoeff;
};
#endif // SLICDIALOG_H
