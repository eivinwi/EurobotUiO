  /*  PRINTLINE("SETUP: done. starting input-loop");
    quit = false;
    while(true) {
        getKeyboardInput();
        if(quit) {
            break;
        }
    }*/




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
        } else if(sub2 == "sl") {
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
        } else if(sub2 == "le") {
            PRINTLINE("SpeedL:" << m->getSpeedL());
        } 
        else if(sub2 == "lr") {
            PRINTLINE("SpeedR:" << m->getSpeedR());
        } 
        else if(sub2 == "vo") {
            PRINTLINE("Volt:" << m->getVoltage());
            m->getVoltage();
        } else if(sub2 == "re") {
            m->resetEncoders();
            PRINTLINE("Encoders reset.");
        } else if(sub2 == "el") {
            PRINTLINE("EncL:" << m->getEncL());
        } else if(sub2 == "er") {
            PRINTLINE("EncR:" << m->getEncR());
        } else if(sub2 == "eb") {
            PRINTLINE("EncL:" << m->getEncL());
            PRINTLINE("EncR:" << m->getEncR());
        } else if(sub2 == "ve") {
            PRINTLINE("Version:" << m->getVersion());
        } else if(sub2 == "ps") {
            m->flush();
            PRINTLINE("Serial flushed.");
        }
    } else if(input == "h") {
        printHelp();
    } else if(input == "q") {
        quit = true;
    } else {
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