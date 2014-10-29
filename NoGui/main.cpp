/*
 * File:   main.cpp
 * Author: Eivind
 *
 * Created on 26. september 2014, 14:15
 */
#include "serial.h"
#include "speedcontrol.h"
#include <string>

int findNumber(std::string s);
void printHelp();

int main(int argc, char *argv[]) { 
    std::cout << "argc: " << argc << '\n';

    SpeedControl *s = new SpeedControl;
    
  //  s->getVoltage();
  //  s->getEncoders();
  //  s->setMode(0);
  //  s->resetEncoders();
  //  s->enableReg(true);
  //  s->enableTimeout(false);
  //  s->setAccel(5);

    std::string input;
    usleep(10000);
   // s->flush();
    while(1) {
        std::cout << "Write cmd: ";
        std::getline(std::cin, input);

        if(input.length() > 1) {
            std::string sub2 = input.substr(0,2);

            if(sub2 == "sl") {
                int speed = findNumber(input);
                if(speed > -1) {
                    s->setSpeedL(speed);
                    std::cout << "SpeedL set to:" << speed << '\n';
                }
            } else if(sub2 == "sr") {
                int speed = findNumber(input);
                if(speed > -1) {
                    s->setSpeedR(speed);
                    std::cout << "SpeedR set to:" << speed << '\n';
                }
            } else if(sub2 == "sb") {
                int speed = findNumber(input);
                if(speed > -1) {
                    s->setSpeedBoth(speed);
                    std::cout << "Speeds set to:" << speed << '\n';
                }
            }
            else if(sub2 == "gl") {
                std::cout << "SpeedL:" << s->getSpeedL() << '\n';
            } 
            else if(sub2 == "gr") {
                std::cout << "SpeedR:" << s->getSpeedR() << '\n';
            } 
            else if(sub2 == "vo") {
                std::cout << "Volt:" << s->getVoltage() << '\n';
                s->getVoltage();
            } 
            else if(sub2 == "re") {
                s->resetEncoders();
                std::cout << "Encoders reset.\n";
            } 
            else if(sub2 == "el") {
                std::cout << "EncL:" << s->getEncL() << '\n';
            }
            else if(sub2 == "er") {
                std::cout << "EncR:" << s->getEncR() << '\n';
            }
            else if(sub2 == "eb") {
                std::cout << "EncL:" << s->getEncL() << '\n';
                std::cout << "EncR:" << s->getEncR() << '\n';
            } 
            else if(sub2 == "ve") {
                std::cout << "Version:" << s->getVersion() << '\n';
            }
            else if(sub2 == "ps") {
                s->flush();
                std::cout << "Serial flushed.\n";
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

