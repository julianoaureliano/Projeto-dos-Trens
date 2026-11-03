#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QPoint>
#include "trem.h"
class QSlider;
class MalhaWidget;
namespace Ui { class MainWindow; }
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void updateInterface(int id, int x, int y);
private:
    Ui::MainWindow *ui;
    Malha malha;
    Trem *trens[6];
    QSlider *controles[6];
    MalhaWidget *desenho;
};
#endif
