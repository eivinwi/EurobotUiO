/*
 *  File: serial.cpp
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

#include "serial.h"

//TODO: try catch
Serial::Serial(std::string serial_port) {
    LOG(INFO) << "[SERIAL]     Opening serial port: " << serial_port.c_str();
    serial.Open(serial_port.c_str());

    serial.SetBaudRate( SerialStreamBuf::BAUD_38400 );
    serial.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 );
    serial.SetNumOfStopBits( SerialStreamBuf::DEFAULT_NO_OF_STOP_BITS );
    serial.SetParity( SerialStreamBuf::PARITY_NONE );
    serial.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_NONE ); //FLOW_CONTROL_HARD
    LOG(INFO) << "[SERIAL]     Serial configuration complete";
}


Serial::~Serial() {
    serial.Close();
}


void Serial::write(uint8_t arg) {
    serial << arg;
}


void Serial::write(char arg) {
    serial << arg;
}


void Serial::write(int arg) {
    write((uint8_t) arg); //hack!!!!
}


uint8_t Serial::readNoWait(){
    DBPL("[SERIAL] readNW");
    int r = 0;
    if(available()) {
        r =  serial.get();
    } else {
        DBPL("In ELSE, sleeping for 50ms");
        usleep(SERIAL_DELAY);
        if(available()) {
            r =  serial.get();
        }
    }
    DBPL("[SERIAL] readNW returning " << r);
    return r;
}


uint8_t Serial::read() {
    auto t_start = std::chrono::high_resolution_clock::now();
    auto t_end = std::chrono::high_resolution_clock::now();    
    uint8_t byte = 0;
    
    double timepassed = std::chrono::duration<double, std::milli>(t_end-t_start).count();
    while(timepassed < 1000) {
        if(available()) {
            byte = readNoWait();
            break;
        }
        t_end = std::chrono::high_resolution_clock::now();
        timepassed = std::chrono::duration<double, std::milli>(t_end-t_start).count();
    }   
    return byte;
}


long Serial::readLong() {
    auto t_start = std::chrono::high_resolution_clock::now();
    auto t_end = std::chrono::high_resolution_clock::now();    
    uint8_t bytes[4];
    std::fill(std::begin(bytes), std::end(bytes), 0);

    for(int i = 0; i < 4; i++) {
        double timepassed = std::chrono::duration<double, std::milli>(t_end-t_start).count();
        while(timepassed < 100) {
            if(available()) {
                bytes[i] = readNoWait();
                break;
            }
            t_end = std::chrono::high_resolution_clock::now();
            timepassed = std::chrono::duration<double, std::milli>(t_end-t_start).count();
        }   
    }

    long result = 0;
    result += bytes[0] << 24;
    result += bytes[1] << 16;
    result += bytes[2] << 8;
    result += bytes[3];

    std::bitset<8> r1(bytes[0]);
    std::bitset<8> r2(bytes[1]);
    std::bitset<8> r3(bytes[2]);
    std::bitset<8> r4(bytes[3]);
    
    LOG(DEBUG) << "[SERIAL] read long, bits:[" 
        << r1 << "][" << r2 << "][" << r3 << "][" << r4 << "] result: " << result;
    return result;
}


bool Serial::available() {
	return serial.rdbuf()->in_avail();
}


void Serial::printAll() {
    std::stringstream ss;
    while(serial.rdbuf()->in_avail()) {
        uint8_t b = serial.get();
        ss << (int) b;
        (void) b;
    }
    LOG(DEBUG) << "Flushed serial: [" << ss << "]";
}