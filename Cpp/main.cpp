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
  //  s->setMode(0);
  //  s->resetEncoders();
  //  s->enableReg(true);
  //  s->enableTimeout(false);
  //  s->setAccel(5);

    std::string input;
    usleep(10000);
    s->flush();
    while(1) {
        std::cout << "Write cmd: ";
        std::getline(std::cin, input);

        if(input.length() > 1) {
            std::string sub2 = input.substr(0,2);

            if(sub2 == "sl") {
                int speed = 0;//= findNumber(input);
                if(speed > -1) {
                    s->setSpeed1(speed);
                }
            } else if(sub2 == "sr") {
                int speed = 0;//findNumber(input);
                if(speed > -1) {
                    s->setSpeed2(speed);
                }
            } else if(sub2 == "sb") {
                int speed = 0;//findNumber(input);
                if(speed > -1) {
                    s->setSpeedBoth(speed);
                }
            }
            else if(sub2 == "gl") {
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
            else if(sub2 == "el") {
                s->getEnc1();
            }
            else if(sub2 == "er") {
                s->getEnc2();
            }
            else if(sub2 == "eb") {
                s->getEncoders();
            } 
            else if(sub2 == "ve") {
                s->getVersion();
            }
            else if(sub2 == "ps") {
                s->flush();
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
    
    std::cout << "el: getEnc1\n";
    std::cout << "er: getEnc2\n";
    std::cout << "eb: getEncoders\n";
    
    std::cout << "re: resetEncoders\n";
    std::cout << "vo: getVoltage\n";
    std::cout << "ve: getVersion\n";
    std::cout << "ps: printSerial\n";
    std::cout << "h: this menu\n";
    std::cout << "q: quit\n";
    std::cout << "------------------------\n";
}

