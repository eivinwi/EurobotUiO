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
#include <string>

int findNumber(std::string s);
void printHelp();

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
    
  //  s->getVoltage();
  //  s->getEncoders();
    
    std::string input;
    while(1) {
        std::cout << "Write cmd: ";
        std::getline(std::cin, input);

        if(input.length() > 1) {
            std::string sub2 = input.substr(0,2);

            if(sub2 == "sl") {
                int speed = findNumber(input);
                if(speed > 0) {
                    s->setSpeed1(speed);
                }
            } else if(sub2 == "sr") {
                int speed = findNumber(input);
                if(speed > 0) {
                    s->setSpeed2(speed);
                }
            } else if(sub2 == "gl") {
                s->getSpeed1();
            } 
            else if(sub2 == "gr") {
                s->getSpeed2();
            } 
            else if(sub2 == "vo") {
                s->getVoltage();
            } 
            else if(sub2 == "re") {
                s->resetEncoders();
            } 
            else if(sub2 == "ge") {
                s->getEncoders();
            } 
        } 
        else if(input == "h") {
            printHelp();
        } else if(input == "q") {
            break;
        }
        else {
            std::cout << "Input too short: " << input.length() << '\n';
        }
    }
    return 0;
}

int findNumber(std::string s) {
    if(s.length() > 3) {
        std::string sub = s.substr(2, s.length()-2);
        int speed = atoi(sub.c_str());
        if(speed >= 0 && speed <= 255) {
            std::cout << "Setting speed: " << speed << '\n';
            return speed;
        } else { 
            std::cout << "Invalid speed (must be 0-255).\n";
        }
    } else {
        std::cout << "Must specify speed (0-255).\n";
    }
    return -1;
}

void printHelp() {
    std::cout << "----- Commands ---------\n";
    std::cout << "sl(0-255): setSpeedLeft\n";
    std::cout << "sr(0-255): setSpeedRight\n";
    std::cout << "gl: getSpeedLeft\n";
    std::cout << "gr: getSpeedRight\n";
    std::cout << "ge: getEncoders\n";
    std::cout << "re: resetEncoders\n";
    std::cout << "vo: getVoltage\n";
    std::cout << "h: this menu\n";
    std::cout << "q: quit\n";
    std::cout << "------------------------\n";
}

