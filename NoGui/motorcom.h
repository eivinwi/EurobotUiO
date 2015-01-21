/* 
 * File: motorcom.h
 * Author: Eivind Wikheim
 *
 * Implements a interface for working with the MD49 motor controller, either via serial
 * or serialsim (decided in main.cpp based on program arguments). 
 *
 * TODO: some of the methods should be private
 */

#ifndef MOTORCOM_H
#define	MOTORCOM_H

#include "serialsim.h"
#include "serial.h"
#include "printing.h"
#include <stdint.h>
#include <cstring>

#define LEFT 1
#define RIGHT 2
#define ENC_BUFF_SIZE 8


class MotorCom {
public:
    MotorCom(std::string serial, bool sim_enabled);
    ~MotorCom();

    bool test();
    void startSerial();
    void setAcceleration(int acc);
    void setSpeedL(uint8_t speed);
    void setSpeedR(uint8_t speed);
    void setSpeedBoth(uint8_t speed);
    void resetEncoders();

    uint8_t getSpeedL();
    uint8_t getSpeedR();
    uint8_t getAcceleration();
    uint8_t getVoltage();
    uint8_t getMode();
    uint8_t getVersion();
    uint8_t getError();
    long getVi();
    
    long getEncL();
    long getEncR();
    void getEncoders();
    
    void setMode(uint8_t mode);
    void enableReg(bool enable);
    void enableTimeout(bool enable);
    void flush();
    void writeToSerial(uint8_t a);
    void writeToSerial(int a);

    bool isSimulating();

private:
    uint8_t readFromSerial();
    uint8_t readFromSerialNoWait();
    long readLongFromSerial();
    void sync();

    SerialSim *simport;
    Serial *port;

    char enc_buffer[ENC_BUFF_SIZE];

    long prev_encL;
    long prev_encR;

    bool simulating;
//	char serial_port[10];
    std::string serial_port;
};

#endif /* MOTORCOM_H */
