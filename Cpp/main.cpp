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
    return 0;
}
