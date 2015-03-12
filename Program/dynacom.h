/*
 *  File: dynacom.h
 *  Author: Eivind Wikheim
 *
 *  DynaCom implements a interface for controlling Dynamixel servos trough USB2AX serial.
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
#ifndef DYNACOM_H
#define DYNACOM_H

#include "dynacom.h"
#include "protocol.h"
#include "printing.h"
#include "serial.h"
#include <atomic>
#include <array>

#define ACTION_DELAY 10000
#define SERIAL_DELAY 500

#define OPEN_STATE 0
#define CLOSED_STATE 1


class DynaCom {
public:
    DynaCom(std::string serial, bool sim_enabled);
    ~DynaCom();
    
    void startSerial();
    void writeToSerial(uint8_t bytes[], int len);
    
    template<std::size_t SIZE>
    void writeToSerial( std::array<uint8_t, SIZE> array );
    
    template<std::size_t SIZE>
    uint8_t calcCheckSum(std::array<uint8_t, SIZE> b);

    void sendCmd(int id, int cmd);
    void setReg1(int id, int regNo, int val);
    void setReg2(int id, int regNoLSB, int val);
    void regRead(int id, int firstRegAdress, int noOfBytesToRead);
    std::array <uint8_t, 8>  readByte();
    void setSpeed(int id, int speed);
    void setMaxTorque(int id, int torque);

    void toggleLed(int id);
    void setPosition(int id, int angle);
    bool openGrip();
    bool closeGrip(); 
    int getPosition();
    bool test();

private:
    int returnValue( std::array <uint8_t, 8> arr );

    Serial *port;
    std::string serial_port;

    // Current positions. Atomic because client may request the current position.
    std::atomic<int> state;
    int id = 1;
//    uint8_t return_byte[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int led = 0;
    bool simulating;
};

#endif /* DYNACOM_H */
