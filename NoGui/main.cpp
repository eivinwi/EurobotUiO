/*
 * File:   main.cpp
 * Author: Eivind
 *
 * Created on 26. september 2014, 14:15
 */
//#include "serial.h"
#include "motorcom.h"
#include "communication.h"
#include "poscontrol.h"
#include "printing.h"
#include <string>
#include <cstring>
#include <pthread.h>

int findNumber(std::string s);
void printHelp();
void getKeyboardInput();
bool checkArguments(int argc, char *argv[]);

bool quit;
MotorCom *m;
Communication *c;

int main(int argc, char *argv[]) {
    PRINTLINE("SETUP: creating MotorCom");
	m = new MotorCom;
    PRINTLINE("SETUP: creating Communication");
    c = new Communication;

    if(!checkArguments(argc, argv)) {
    	return -1;
    }

    PRINTLINE("SETUP: starting serial");
    m->startSerial();
    m->flush();


    PRINTLINE("SETUP: done. starting input-loop");
    quit = false;
    while(true) {
        getKeyboardInput();
        if(quit) {
            break;
        }
    }

/*    m->resetEncoders();
    Poscontrol *p = new Poscontrol(m);

    p->testDrive(35, 100);*/
    PRINTLINE("Exiting");
    return 0;
}


/* return:
 *  	true - if good/no arguments
 *		false - if invalid argument exists
*/
bool checkArguments(int argc, char *argv[]) {
	PRINTLINE("READING Arguments");
	m->serialSimDisable(); //just because
    if(argc < 2) {
    	PRINTLINE("No arguments - expecting serial at: /dev/ttyACM0");
    	m->setSerialPort("ttyACM0");
    } else {
    	PRINT("Arguments: ");
    	for(int i = 0; i < argc; i++) {
    		PRINT(argv[i]);
    	}
    	PRINTLINE("");


    	for(int i = 1; i < argc; i++) {
			if(strcmp(argv[i], "sim") == 0) {
				PRINTLINE("Simulating serial.");	
				m->serialSimEnable();
			}
			else if(strcmp(argv[i], "ttyUSB0") == 0) {
				PRINTLINE("Opening serial on: /dev/" << argv[i]);
				m->setSerialPort(argv[1]);
			}
			else if(strcmp(argv[i], "ttyS0") == 0) {
				PRINTLINE("Opening serial on: /dev/" << argv[i]);
				m->setSerialPort(argv[1]);
			}
			else if(strcmp(argv[i], "ttyACM1") == 0) {
				PRINTLINE("Opening serial on: /dev/" << argv[i]);
				m->setSerialPort(argv[1]);
			}
			else if(strcmp(argv[i], "ttyUSB1") == 0) {
				PRINTLINE("Opening serial on: /dev/" << argv[i]);
				m->setSerialPort(argv[1]);
			} else {
				PRINTLINE("Invalid argument: " << argv[i]);
				return false;
			}
		}
    }
    return true;
}


void getKeyboardInput() {
    std::string input;
    PRINT("Write cmd: ");
    std::getline(std::cin, input);

    if(input.length() > 1) {
        std::string sub2 = input.substr(0,2);

        if(sub2 == "sl") {
            int speed = findNumber(input);
            if(speed > -1) {
                m->setSpeedL(speed);
                PRINTLINE("SpeedL set to:" << speed);
            }
        } else if(sub2 == "sr") {
            int speed = findNumber(input);
            if(speed > -1) {
                m->setSpeedR(speed);
                PRINTLINE("SpeedR set to:" << speed);
            }
        } else if(sub2 == "sb") {
            int speed = findNumber(input);
            if(speed > -1) {
                m->setSpeedBoth(speed);
                PRINTLINE("Speeds set to:" << speed);
            }
        }
        else if(sub2 == "gl") {
            PRINTLINE("SpeedL:" << m->getSpeedL());
        } 
        else if(sub2 == "gr") {
            PRINTLINE("SpeedR:" << m->getSpeedR());
        } 
        else if(sub2 == "vo") {
            PRINTLINE("Volt:" << m->getVoltage());
            m->getVoltage();
        } 
        else if(sub2 == "re") {
            m->resetEncoders();
            PRINTLINE("Encoders reset.");
        } 
        else if(sub2 == "el") {
            PRINTLINE("EncL:" << m->getEncL());
        }
        else if(sub2 == "er") {
            PRINTLINE("EncR:" << m->getEncR());
        }
        else if(sub2 == "eb") {
            PRINTLINE("EncL:" << m->getEncL());
            PRINTLINE("EncR:" << m->getEncR());
        } 
        else if(sub2 == "ve") {
            PRINTLINE("Version:" << m->getVersion());
        }
        else if(sub2 == "ps") {
            m->flush();
            PRINTLINE("Serial flushed.");
        }
    } 
    else if(input == "h") {
        printHelp();
    } else if(input == "q") {
        quit = true;
    }
    else {
        PRINTLINE("Input too short: " << input.length());
    }
}

int findNumber(std::string s) {
    if(s.length() > 3) {
        std::string sub = s.substr(2, s.length()-2);
        int speed = atoi(sub.c_str());
        if(speed >= 0 && speed <= 255) {
            PRINTLINE("Setting speed: " << speed);
            return speed;
        } else { 
            PRINTLINE("Invalid speed (must be 0-255).");
        }
    } else {
        PRINTLINE("Must specify speed (0-255).");
    }
    return -1;
}

void printHelp() {
    PRINTLINE("----- Commands ---------");
    PRINTLINE("sl(0-255): setSpeedLeft");
    PRINTLINE("sr(0-255): setSpeedRight");
    PRINTLINE("gl: getSpeedLeft");
    PRINTLINE("gr: getSpeedRight");
    
    PRINTLINE("el: getEnc1");
    PRINTLINE("er: getEnc2");
    PRINTLINE("eb: getEncoders");
    
    PRINTLINE("re: resetEncoders");
    PRINTLINE("vo: getVoltage");
    PRINTLINE("ve: getVersion");
    PRINTLINE("ps: PRINTSerial");
    PRINTLINE("h: this menu");
    PRINTLINE("q: quit");
    PRINTLINE("------------------------");
}

