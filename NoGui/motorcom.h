/* 
 * File: motorcom.h
 * Author: Eivind Wikheim
 *
 * Implements a interface for working with the MD49 motor controller, either via serial
 * or serialsim (decided in main.cpp based on program arguments). 
 * 
 */

#ifndef MOTORCOM_H
#define	MOTORCOM_H

#include "serialsim.h"
#include "serial.h"
#include "printing.h"
#include <stdint.h>
#include <bitset>
#include <sstream>
#include <cstring>

#define LEFT 1
#define RIGHT 2
#define ENC_BUFF_SIZE 8


class MotorCom {
public:
    MotorCom();
    ~MotorCom();

    void startSerial();
	void serialSimEnable();
	void serialSimDisable();
	void setSerialPort(const char *s);
    void setAcceleration(int acc);
    void setSpeedL(uint8_t speed);
    void setSpeedR(uint8_t speed);
    void setSpeedBoth(uint8_t speed);
    uint8_t getSpeedL();
    uint8_t getSpeedR();
    uint8_t getAcceleration();
    void resetEncoders();
    long getEncL();
    long getEncR();
    void getEncoders();
    uint8_t getVoltage();
    long getVi();
    uint8_t getMode();
    uint8_t getVersion();
    uint8_t getError();
    void setMode(uint8_t mode);
    void enableReg(bool b);
    void enableTimeout(bool b);
    void flush();
    void sync();
    void writeToSerial(uint8_t a);
    void writeToSerial(int a);
    //int readFromSerial();
    uint8_t readFromSerial();
    uint8_t readFromSerialNoWait();

private:
    SerialSim *simport;
    Serial *port;

    char enc_buffer[ENC_BUFF_SIZE];

    long prev_encL;
    long prev_encR;

    bool simulating;
	char serial_port[10];
};

#endif /* MOTORCOM_H */
