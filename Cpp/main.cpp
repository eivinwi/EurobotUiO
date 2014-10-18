/*
 * File:   main.cpp
 * Author: Eivind
 *
 * Created on 26. september 2014, 14:15
 */

//#include <QApplication>
//#include "MainWindow.h"
#include "Serial.h"
#include "SpeedControl.h"

int main(int argc, char *argv[]) {
    // initialize resources, if needed
    // Q_INIT_RESOURCE(resfile);
  /*  QApplication app(argc, argv);
    app.setOrganizationName("EurobotUiO");
    app.setApplicationName("Locomotion Control");
	    
    
    MainWindow mainWin;
    mainWin.show();
    return app.exec();*/

    //Serial *s = new Serial;
    SpeedControl *s = new SpeedControl;

//    s->testProg();
//    s->close();
    uint8_t sp = 0xfa;
    //s->setSpeed2(sp);
  //  s->getVoltage();
    s->resetEncoders();
    s->getEncoders();
    while(1) {
/*        s->setSpeed(LEFT, 1, 200);
        s->setSpeed(RIGHT, 1, 200);
        s->updateSpeeds();
        usleep(900000);
        s->setSpeed(LEFT, 0, 200);
        s->setSpeed(RIGHT, 0, 200);
        s->updateSpeeds();
        usleep(900000);
        s->setSpeed(LEFT, 0, 0);
        s->setSpeed(RIGHT, 0, 0);
        s->updateSpeeds();
        usleep(900000);*/

        s->setSpeed1(sp);
        usleep(500000);
    }
    return 0;
}
