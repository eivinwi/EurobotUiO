/* 
 * File: motorcom.h
 * Author: Eivind Wikheim
 *
 * Implements a interface for working with the MD49 motor controller, either via serial
 * or serialsim (decided in main.cpp based on program arguments). 
 *
 * TODO: some of the methods should be private
 */

#ifndef LIFTCOM_H
#define	LIFTCOM_H

#include "serial.h"
#include "protocol.h"
#include "printing.h"


#define BOTTOM 0
#define MIDDLE 1
#define TOP 2

#define ARD_BOTTOM 1
#define ARD_TOP 0
#define ARD_MIDDLE 2
#define GET 3

class LiftCom {
public:
    LiftCom();
    ~LiftCom();

    void setSerialPort(const char *s);

    void startSerial();

    void goTo(int p);
    void goToTop();
    void goToMiddle();
    void goToBottom();

    void setCurrentPos(int p);
    uint8_t getPosition();
    bool test();

private:
    void writeToSerial(uint8_t a);
    void writeToSerial(int a);
    uint8_t readFromSerial();

    Serial *port;
    char serial_port[10];
    int current_pos;
};

#endif /* LIFTCOM_H */
