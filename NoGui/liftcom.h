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

#include "protocol.h"
#include "printing.h"
#include "serial.h"
#include <atomic>

#define BOTTOM 0
#define MIDDLE 1
#define TOP 2
#define OPEN 3
#define CLOSE 4
#define GET 5

#define ARD_BOTTOM 1
#define ARD_TOP 0
#define ARD_MIDDLE 2

#define SUCCESS 53

class LiftCom {
public:
    LiftCom(std::string serial);
    ~LiftCom();

    void setSerialPort(const char *s);

    void startSerial();

    void goTo(int p);

    void setCurrentPos(int p);
    uint8_t getPosition();
    bool test();
    bool waitForResponse();

private:
    void goToTop();
    void goToMiddle();
    void goToBottom();
    void openGrabber();
    void closeGrabber();

    void writeToSerial(uint8_t a);
    void writeToSerial(int a);
    uint8_t readFromSerial();

    Serial *port;
    std::string serial_port;
    std::atomic<int> lift_pos;
    std::atomic<int> grabber_pos;
};

#endif /* LIFTCOM_H */
