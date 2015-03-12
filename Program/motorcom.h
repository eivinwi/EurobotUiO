/*
 *  File: motorcom.h
 *  Author: Eivind Wikheim
 *
 *  MotorCom implements an interface for working with the MD49 motor controller, 
 *  either via serial or serialsim (decided in main.cpp based on program arguments). 
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

// TODO: some of the methods should be private

#ifndef MOTORCOM_H
#define	MOTORCOM_H

#include "printing.h"
#include "serial.h"
#include "serialsim.h"
#include <cstring>
#include <stdint.h>

#define LEFT 1
#define RIGHT 2
#define ENC_BUFF_SIZE 8


class MotorCom {
public:
    MotorCom(std::string serial, bool sim_enabled);
    ~MotorCom();

    // Starts the serial trough libSerial library
    void startSerial();

    // Test the serial communication on supplied serial-port
    // TODO: should timeout the check
    // Returns true if correct reply is recieved trough serial.
    bool test();

    // Sends new acceleration constant to MD49
    void setAcceleration(int acc);

    // Sets speed of left motor
    void setSpeedL(uint8_t speed);
    
    // Sets speed of right motor
    void setSpeedR(uint8_t speed);

    // Sets same speed (opposite) on both motors.
    void setSpeedBoth(uint8_t speed);
    
    // Sets the speed-control mode of MD49. Should probably always be 0
    void setMode(uint8_t mode);
    
    // Enable/Disable power-regulation feature on MD49 (see MD49 doc). Disabled by default (?)
    void enableReg(bool enable);

    // Enable/disable timeout on MD49. Is enabled by default. (See MD49 doc)
    void enableTimeout(bool enable);

    // Sends reset-command to MD49 encoders
    void resetEncoders();

    // Returns current speed of left motor
    // PROBLEM: will return 0 if it gets nothing on serial
    uint8_t getSpeedL();

    // Returns current speed of left motor
    // PROBLEM: will return 0 if it gets nothing on serial    
    uint8_t getSpeedR();

    // Returns current acceleration setting
    uint8_t getAcceleration();

    // Returns current voltage on MD49 (should be about 24V)
    uint8_t getVoltage();

    // Returns current speed control mode (see MD49 doc). Should probably be 0
    uint8_t getMode();

    // Returns MD49 board version. Should be 2 (?)
    uint8_t getVersion();

    // Returns current error (see MD49 doc). Should be 0 if everything okay.
    uint8_t getError();

    // Returns [voltage,current of left motor, current of right motor, 0] 
    long getVi();
    
    // Retrieves left encoder value
    long getEncL();

    // Retrieves right encoder value
    long getEncR();

    // Retrieves both encoder values
    void getEncoders();
    
    // Clears and prints anything left on the serial bus.
    void flush();

    // Returns true if serial is being simulated
    bool isSimulating();

private:
    // Writes to Serial. Overloaded int calls uint8_t
    void writeToSerial(uint8_t a);
    void writeToSerial(int a);


    // Reads a byte from serial. Will time out if no input
    uint8_t readFromSerial();

    // Reads a byte form serial without waiting.
    // Usefull for burst-reading (like when reading a long).
    uint8_t readFromSerialNoWait();

    // Read 4 bytes from serial and returns then spliced together to a long.
    long readLongFromSerial();

    // Sends 0x00 to MD49, to signal start of new command.
    void sync();

    SerialSim *simport;
    Serial *port;

    char enc_buffer[ENC_BUFF_SIZE];

    //contains previous encoder values (for checking)
    long prev_encL, prev_encR;

    bool simulating;
    std::string serial_port;
};

#endif /* MOTORCOM_H */
