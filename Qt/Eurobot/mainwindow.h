#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "speedcontrol.h"
#include <QMainWindow>
#include <string>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

//    void speedChangeL(int s);
   // SpeedControl *s;

public slots:
      void speedChangeL();
      void speedChangeR();
      void speedChangeBoth();
      void changeAcceleration();
      void stopButton();
      void resetButton();
      void updateVi();
      void updateEncoders();
      void changeMode();
      void changeReg();
      void changeTimeout();
      void marioPlease();

private:
    Ui::MainWindow *ui;
    SpeedControl *s;
    int counter = 0;

};

#endif // MAINWINDOW_H
