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
#include <SerialStream.h>
#include <string>

 
using namespace LibSerial ;


class Serial {
public:
    Serial(std::string serial_port);
    ~Serial();

    // Overloaded write, shifts arg onto serial
    void write(uint8_t arg);
    void write(char arg);
    void write(int arg);
    
    // Waits for serial to be available, then writes. Timeout after set time.
    uint8_t read();

    // Reads without the wait (useful for burst-read of multi-byte variables).
    uint8_t readNoWait();

    //
    long readLong();
    
    // Reads and prints everything currently on the serial.
    void printAll();

    // True if serial has byte ready to be red.
    bool available();

private:
    SerialStream serial;
    
};

#endif	/* SERIAL_H */

