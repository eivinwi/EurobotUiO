/*
 *  File: liftcom.h
 *  Author: Eivind Wikheim
 *
 *  LiftCom implements a interface controlling lift/gripper parts via an arduino, 
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
#ifndef LIFTCOM_H
#define	LIFTCOM_H

#include "protocol.h"
#include "printing.h"
#include "serial.h"
#include <atomic>

// Lift positions
#define BOTTOM 0
#define MIDDLE 1
#define TOP 2
// Grabber positions
#define OPEN 3
#define CLOSE 4
// Get position
#define GET 5

// Arduino has different protocol
#define ARD_BOTTOM 1
#define ARD_TOP 0
#define ARD_MIDDLE 2

#define SUCCESS 53

class LiftCom {
public:
    LiftCom(std::string serial, bool sim_enabled);
    ~LiftCom();

    // Setting serial port. Not really used as port should be sent as param to constructor
    void setSerialPort(const char *s);

    // Creates a Serial object and starts serial trough libSerial
    void startSerial();

    // Sends lift to position p. (see defines at top of file)
    void goTo(int p);

    // Used instead of goTo for testing, just sets position instead of communicating with arduino.
    void setCurrentPos(int p);

    // Returns current stored position. Cannot actually check the physical position
    uint8_t getPosition();

    // Checks if serial is open.
    // TODO: should perhaps check if arduino is connected
    bool test();

    // Waits for a response on serial, times out after a set time.
    bool waitForResponse();

private:
    // Functions for sending commands to arduino
    void goToTop();
    void goToMiddle();
    void goToBottom();
    void openGrabber();
    void closeGrabber();

    // Functions to use Serial object.
    void writeToSerial(uint8_t a);
    void writeToSerial(int a);
    uint8_t readFromSerial();

    Serial *port;
    std::string serial_port;

    // Current positions. Atomic because client may request the current position.
    std::atomic<int> lift_pos;
    std::atomic<int> grabber_pos;

    bool simulating;
};

#endif /* LIFTCOM_H */
