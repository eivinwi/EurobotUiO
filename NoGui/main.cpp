/*
 * File:   main.cpp
 * Author: Eivind
 *
 * Created on 26. september 2014, 14:15
 */
//#include "serial.h"
#include "motorcom.h"
#include "communication.h"
#include <string>
#include <cstring>
#include <pthread.h>

int findNumber(std::string s);
void printHelp();
void getKeyboardInput();

bool quit;
MotorCom *m;
Communication *c;

int main(int argc, char *argv[]) {
    std::cout << "CREATING SpeedControl\n";
	m = new MotorCom;
    std::cout << "CREATING Communication\n";
    c = new Communication;

    std::cout << "READING Arguments\n";
    if(argc < 2) {
    	std::cout << "No arguments - expecting serial at: /dev/ACM0\n";
    	m->setSerialPort("ttyACM0");
    } else {
    	std::cout << "Arguments: ";
    	for(int i = 0; i < argc; i++) {
    		std::cout << argv[i];
    	}
    	std::cout << '\n';

        if(strcmp(argv[0], "valgrind") == 0) {
            std::cout << "Valgrind is running; debugging.\n";
        }
		else if(strcmp(argv[1], "sim") == 0) {
			std::cout << "Simulating serial.\n";	
			m->serialSimEnable();
		}
		else if(strcmp(argv[1], "ttyUSB0") == 0) {
			std::cout << "Opening serial on: /dev/" << argv[1] << '\n';
			m->setSerialPort(argv[1]);
		}
		else if(strcmp(argv[1], "ttyS0") == 0) {
			std::cout << "Opening serial on: /dev/" << argv[1] << '\n';
			m->setSerialPort(argv[1]);
		}
		else if(strcmp(argv[1], "ttyACM1") == 0) {
			std::cout << "Opening serial on: /dev/" << argv[1] << '\n';
			m->setSerialPort(argv[1]);
		}
		else if(strcmp(argv[1], "ttyUSB1") == 0) {
			std::cout << "Opening serial on: /dev/" << argv[1] << '\n';
			m->setSerialPort(argv[1]);
		} else {
			std::cout << "Invalid arguments: /dev/" << argv[1] << '\n';
			return 0;
		}
    }

    std::cout << "STARTING Serial\n";
    m->startSerial();


    std::cout << "STARTING input-loop\n";
    quit = false;
    while(true) {
        getKeyboardInput();
        if(quit) {
            break;
        }
    }
    std::cout << "DONE";
    return 0;
}

void getKeyboardInput() {
    std::string input;
    std::cout << "Write cmd: ";
    std::getline(std::cin, input);

    if(input.length() > 1) {
        std::string sub2 = input.substr(0,2);

        if(sub2 == "sl") {
            int speed = findNumber(input);
            if(speed > -1) {
                m->setSpeedL(speed);
                std::cout << "SpeedL set to:" << speed << '\n';
            }
        } else if(sub2 == "sr") {
            int speed = findNumber(input);
            if(speed > -1) {
                m->setSpeedR(speed);
                std::cout << "SpeedR set to:" << speed << '\n';
            }
        } else if(sub2 == "sb") {
            int speed = findNumber(input);
            if(speed > -1) {
                m->setSpeedBoth(speed);
                std::cout << "Speeds set to:" << speed << '\n';
            }
        }
        else if(sub2 == "gl") {
            std::cout << "SpeedL:" << m->getSpeedL() << '\n';
        } 
        else if(sub2 == "gr") {
            std::cout << "SpeedR:" << m->getSpeedR() << '\n';
        } 
        else if(sub2 == "vo") {
            std::cout << "Volt:" << m->getVoltage() << '\n';
            m->getVoltage();
        } 
        else if(sub2 == "re") {
            m->resetEncoders();
            std::cout << "Encoders reset.\n";
        } 
        else if(sub2 == "el") {
            std::cout << "EncL:" << m->getEncL() << '\n';
        }
        else if(sub2 == "er") {
            std::cout << "EncR:" << m->getEncR() << '\n';
        }
        else if(sub2 == "eb") {
            std::cout << "EncL:" << m->getEncL() << '\n';
            std::cout << "EncR:" << m->getEncR() << '\n';
        } 
        else if(sub2 == "ve") {
            std::cout << "Version:" << m->getVersion() << '\n';
        }
        else if(sub2 == "ps") {
            m->flush();
            std::cout << "Serial flushed.\n";
        }
    } 
    else if(input == "h") {
        printHelp();
    } else if(input == "q") {
        quit = true;
    }
    else {
        std::cout << "Input too short: " << input.length() << '\n';
    }
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

