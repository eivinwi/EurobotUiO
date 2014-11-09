/*
 * File:   main.cpp
 * Author: Eivind
 *
 * Created on 26. september 2014, 14:15
 */
//#include "serial.h"

//TODO: own method for getdegrees/getpositions to the readloop
 //TODO: negative angle adjustments


#include "motorcom.h"
#include "communication.h"
#include "poscontrol.h"
#include "printing.h"
#include <string>
#include <cstring>
#include <pthread.h>

int findNumber(std::string s);
int findAngle(std::string s);
int findPosition(std::string s);
void goToXYR(int x, int y, int z);
void printHelp();
void getKeyboardInput();
bool checkArguments(int argc, char *argv[]);
void drive();

int coords[3];

bool quit;
MotorCom *m;
Communication *c;
PosControl *p;

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
    m->resetEncoders();
    usleep(10000);
    m->flush();

    p = new PosControl(m);

    PRINTLINE("SETUP: done. starting input-loop");
    quit = false;
    while(true) {
        getKeyboardInput();
        if(quit) {
            break;
        }
    }

    PRINTLINE("MAIN: Exiting");
    return 0;
}

void drive() {
    bool done = false;
    while(!done) {
        done = p->controlLoop();
        usleep(4000);
    }
    PRINTLINE("Drive done");
    usleep(2000000);
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
    		PRINT(argv[i] << " ");
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

        if(sub2 == "gx") {
            int pos = findPosition(input);
            goToXYR(pos, coords[1], coords[2]);
        } if(sub2 == "gy") {
            int pos = findPosition(input);
            goToXYR(coords[0], pos, coords[2]);
        } else if(sub2 == "gr") {
            int rot = findAngle(input);
            goToXYR(coords[0], coords[1], rot);
        } else if(sub2 == "re") {
            PRINTLINE("Setting current position as 0,0,0");
            p->resetPosition();
        }
        else if(sub2 == "sl") {
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
        else if(sub2 == "le") {
            PRINTLINE("SpeedL:" << m->getSpeedL());
        } 
        else if(sub2 == "lr") {
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
            return speed;
        } else { 
            PRINTLINE("Invalid speed (must be 0-255).");
        }
    } else {
        PRINTLINE("Must specify speed (0-255).");
    }
    return -1;
}

int findAngle(std::string s) {
    if(s.length() > 3) {
        std::string sub = s.substr(2, s.length()-2);
        int speed = atoi(sub.c_str());
        if(speed >= 0 && speed <= 360) {
            return speed;
        } else { 
            PRINTLINE("Invalid angle(must be 0-360).");
        }
    } else {
        PRINTLINE("Must specify angle (0-360).");
    }
    return 0;
}

int findPosition(std::string s) {
    if(s.length() > 3) {
        std::string sub = s.substr(2, s.length()-2);
        int pos = atoi(sub.c_str());
        if(pos >= 0 && pos <= 300) {
            return pos;
        } else { 
            PRINTLINE("Invalid position(must be 0-300).");
        }
    } else {
        PRINTLINE("Must specify position (0-300).");
    }
    return 0;
}


void goToXYR(int x, int y, int r) {
    if(coords[0] > 3000 || coords[1] > 2000 || coords[2] > 360) {
        PRINTLINE("MAIN: gotoXYR-values too high, aborting goToXYR");
    } else {
        int changed = 0;
        if(x != coords[0]) changed++;
        if(y != coords[1]) changed++;
        if(r != coords[2]) changed++;

        if(changed > 1) {
            PRINTLINE("MAIN: goToXYR cannot change more than 1 position at a time");
        } else {
            PRINTLINE("MAIN: goToXYR attempting to reach: " << x << "," << y << "," << r);
            p->setGoalPos(x, y, r);
            drive();
            coords[0] = x;
            coords[1] = y;
            coords[2] = r;
        }
    }
}

/*
void goToXYR(std::string input) {
    std::string sub = input.substr(2, input.length()-2);
    PRINTLINE("MAIN: goToXYR with arguments: " << sub);
    std::string delimiter = ",";    

    int coords[3];

    std::string token;
    size_t pos = 0;
    int i = 0;
    while( (pos = sub.find(delimiter)) != std::string::npos) {
        if(i > 1) break;
        token = sub.substr(0, pos);
        coords[i] = atoi(token.c_str());
        sub.erase(0, pos+1);
        i++;
    }
    if(sub.length() > 0) {
        PRINTLINE("sub" << sub)
        coords[i] = atoi(sub.c_str());
        i++;
    } 
    while(i < 3) {
        coords[i] = 0;
        i++;
    }

    PRINTLINE("tokenizer done: [" << coords[0] << "][" << coords[1] << "][" << coords[2] << "]");

    if(coords[0] > 1000 || coords[1] > 1000 || coords[2] > 360) {
        PRINTLINE("Values too high, aborting goToXYR");
    } else {
        p->setGoalPos(coords[0], coords[1], coords[2]);
        drive();
    }
}*/



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
