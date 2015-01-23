/*  
 * File: serial.h
 * Author: Eivind Wikheim
 *
 * Implements functions for communicating to the MD49 motor controller via serial. 
 * Serial can be run at either 9600 or 38400 baud (IMPORTANT: changed with hardware pins on the controller.) 
 *
 */

#ifndef SERIAL_H
#define	SERIAL_H

#include "md49.h"
#include "printing.h"
#include <bitset>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <SerialStream.h>
#include <sstream>
#include <stdint.h>
#include <string>
#include <unistd.h>
 
using namespace LibSerial ;


class Serial {
public:
    Serial(std::string serial_port);
    ~Serial();
    void write(uint8_t arg);
    void write(char arg);
    void write(int arg);
    uint8_t readNoWait();
    uint8_t read();
    long readLong();
    void printAll();
    bool available();

private:
    SerialStream serial;
//    SerialPort serial;

};

#endif	/* SERIAL_H */

