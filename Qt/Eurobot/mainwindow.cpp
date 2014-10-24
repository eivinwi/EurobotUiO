#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <bitset>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    counter = 0;

    s = new SpeedControl();

    //Connect sliders to setX()-functions
    connect(ui->slider_speedL, SIGNAL(valueChanged(int)), this, SLOT(speedChangeL()));
    connect(ui->slider_speedR, SIGNAL(valueChanged(int)), this, SLOT(speedChangeR()));
    connect(ui->slider_both, SIGNAL(valueChanged(int)), this, SLOT(speedChangeBoth()));

    connect(ui->spin_accel, SIGNAL(valueChanged(int)), this, SLOT(changeAcceleration()));

    connect(ui->combo_mode, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMode()));

    connect(ui->check_reg, SIGNAL(stateChanged(int)), this, SLOT(changeReg()));
    connect(ui->check_time, SIGNAL(stateChanged(int)), this, SLOT(changeTimeout()));

    connect(ui->button_stop, SIGNAL(clicked()), this, SLOT(stopButton()));
    connect(ui->button_reset, SIGNAL(clicked()), this, SLOT(resetButton()));

    //placeholder
    connect(ui->button_refresh, SIGNAL(clicked()), this, SLOT(updateVi()));


    //Set defaults
    s->setAcceleration(5);
    s->setMode(0);
    s->enableReg(true);
    s->enableTimeout(false);
    s->flush();

    //First-time display setup
    ui->display_version->display(s->getVersion());
    s->flush();
    ui->display_accel->setValue(s->getAcceleration());
    ui->combo_mode->setCurrentIndex(0);
    ui->check_reg->setChecked(true);
    ui->check_time->setChecked(false);
    s->flush();
    ui->display_error->display(s->getError());

   // s->setSpeedL(0xFF);
    s->flush();
    updateVi();
}


void MainWindow::updateVi() {
   long vi = s->getVi();
   uint8_t volt = ((vi & 0x00FF0000) >> 16);
   uint8_t curL = ((vi & 0x0000FF00) >> 8);
   uint8_t curR = (vi & 0x000000FF);

   ui->display_voltage->setValue(volt);
   ui->display_currentL->setValue(curL);
   ui->display_currentR->setValue(curR);

   ui->display_accel->setValue(s->getAcceleration());
}


void MainWindow::speedChangeL() {
    int val = ui->slider_speedL->value();
    ui->display_speedL->setValue(val);
    s->setSpeedL(val);
}

void MainWindow::speedChangeR() {
    int val = ui->slider_speedR->value();
    ui->display_speedR->setValue(val);
    s->setSpeedR(val);
}

void MainWindow::speedChangeBoth() {
    int val = ui->slider_both->value();
    ui->display_speedL->setValue(val);
    ui->display_speedR->setValue(val);
    s->setSpeedBoth(val);
}

void MainWindow::changeAcceleration() {
    s->setAcceleration(ui->spin_accel->value());
}

void MainWindow::stopButton() {
    //QApplication::quit();
    ui->display_speedL->setValue(128);
    ui->display_speedR->setValue(128);
    s->setSpeedBoth(128);
}

void MainWindow::resetButton() {
    s->resetEncoders();
}

void MainWindow::changeMode() {
    int val = ui->combo_mode->currentIndex();
    s->setMode(val);
}

void MainWindow::changeReg() {
    s->enableReg(ui->check_reg->isEnabled());
}

void MainWindow::changeTimeout() {
    s->enableTimeout(ui->check_time->isEnabled());
}

MainWindow::~MainWindow()
{
    delete ui;
}
