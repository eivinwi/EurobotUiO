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
// TODO: comments/documentation
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
    
    // Initializes Dynamixel communication trough the Serial class
    void startSerial();

    // Toggle LED of servo "id"
    void toggleLed(int id);


    // Puts the grippers in open position
    bool openGrip();

    // Puts the grippers in closed position
    bool closeGrip(); 

    // Returns gripper position. See Dynamixel AX-12A documentation for details
    int getPosition();
    
    // Sends a request to the Arduino connected to serial, returns true if response is correct
    bool test();

private:
    // Sets gripper position. See Dynamixel AX-12A documentation for details.
    // Servos have been hardware-limited to only accept legal positions to avoid breaking parts.
    void setPosition(int id, int angle);

    // DEPRECATED
    // Writes bytes in array to serial. 
    void writeToSerial(uint8_t bytes[], int len);
    
    // Writes an array of size SIZE to serial
    template<std::size_t SIZE>
    void writeToSerial( std::array<uint8_t, SIZE> array );
    
    // Returns a 8-byte response from serial, as an array
    std::array <uint8_t, 8>  readByte();

    // Returns a checksum for the input ASCII byte packet. Input must be of length > 4
    template<std::size_t SIZE>
    uint8_t calcCheckSum(std::array<uint8_t, SIZE> b);

    // Sends short command
    void sendCmd(int id, int cmd);
    
    // Writes 0<val<255 to register "regNo" in servo "id" 
    void setReg1(int id, int regNo, int val);
    
   // Writes 0<val<1023 to register "regNoLSB/regNoLSB+1" in servo "id"
    void setReg2(int id, int regNoLSB, int val);
    
    // Request response-byte servo "id", with specified registers
    void regRead(int id, int firstRegAdress, int noOfBytesToRead);
    
    // Sets the speed register of servo "id" to speed [0-1023]
    void setSpeed(int id, int speed);
    
    // Sets the torque register of servo "id" to speed [0-1023] 
    void setMaxTorque(int id, int torque);
    
    // extracts registry-value from response byte arr
    int returnValue( std::array <uint8_t, 8> arr );

    // Gripper servo ID
    int id = 1;

    Serial *port;
    std::string serial_port;

    // Current positions. Atomic because client may request the current position.
    std::atomic<int> state;

    int led = 0;

    // Set from main. If true, no actions are performed, but responses are always SUCCESS
    bool simulating;
};

#endif /* DYNACOM_H */
