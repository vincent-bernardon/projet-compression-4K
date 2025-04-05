#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSelectImageClicked();
    void onSelectOutputFolder();
    void onRunSLIC();
    void onRunSDGT();
    void showFinalImage(const QString &imagePath);
    void showInputImage(const QString &imagePath);
    void initializeGraphicsView();
    bool eventFilter(QObject *watched, QEvent *event) override;
    void safeFit(QGraphicsView* view);
    void resetZooms();



private:
    Ui::MainWindow *ui;
    QString imagePath;
};
#endif // MAINWINDOW_H
