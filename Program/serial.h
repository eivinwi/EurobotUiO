 /*
 *  File: serial.h
 *  Author: Eivind Wikheim
 *
 *  Serial implements functions for communicating to the MD49 motor controller via serial. 
 *  Can be run at either 9600 or 38400 baud (IMPORTANT: changed with hardware pins on the controller.) 
 *
 *  Copyright (c) 2015 Eivind Wikheim <eivinwi@ifi.uio.no>. All Rights Reserved.
 *
 *  This file is part of EurobotUiO.
 *
 *  EurobotUiO is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EurobotUiO is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EurobotUiO.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SERIAL_H
#define	SERIAL_H

#include <bitset>
#include <chrono>
#include <string> 
#include <SerialStream.h>
using namespace LibSerial ;

#include "md49.h"
#include "printing.h"

#define SERIAL_DELAY 500

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

    // Reads 4 bytes from serial and combines them to a long
    long readLong();
    
    // Reads and prints everything currently on the serial.
    void printAll();

    // true: a byte is ready to be read form the serial
    bool available();

private:
    SerialStream serial;
    
};

#endif	/* SERIAL_H */

