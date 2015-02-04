/*
 *	File: liftcom.cpp
 *	Author: Eivind Wikheim
 *
 *  LiftCom implements a interface controlling lift/gripper parts via an arduino, 
 *  either via serial or serialsim (decided in main.cpp based on program arguments).
 *
 *	Copyright (c) 2015 Eivind Wikheim <eivinwi@ifi.uio.no>. All Rights Reserved.
 *
 *	This file is part of EurobotUiO.
 *
 *	EurobotUiO is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 * 	(at your option) any later version.
 *
 *	EurobotUiO is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with EurobotUiO.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "liftcom.h"

LiftCom::LiftCom(std::string serial) {
    serial_port = serial;
    lift_pos = BOTTOM;
}


LiftCom::~LiftCom() {
}


void LiftCom::startSerial() {        
    if(serial_port == "") { //should never happen
        LOG(WARNING) << "[LIFT] 	Error, empty serial_port. Setting to /dev/ttyUSB1";
        serial_port = "ttyUSB1";
    }     
    LOG(INFO) << "[LIFT] 	 Starting serial at: " << serial_port;
	port = new Serial(serial_port);
}


void LiftCom::goTo(int p) {
	switch(p) {
		case TOP: 
			goToTop();
			break;
		case MIDDLE:
			goToMiddle();
			break;
		case BOTTOM:
			goToBottom();
			break;
		case OPEN:
			openGrabber();
			break;
		case CLOSE:
			closeGrabber();
			break;
		case GET:
			getPosition();
			break;
		default:
			LOG(INFO) << "[LIFT] invalid goto-command.";
			break;

	}
}


void LiftCom::goToTop() {
	LOG(INFO) << "[LIFT] goToTop, previous is: " << lift_pos;
	writeToSerial(ARD_TOP);
	lift_pos = TOP;
}


void LiftCom::goToMiddle() {
	LOG(INFO) << "[LIFT] goToMiddle, previous is: " << lift_pos;
	writeToSerial(ARD_MIDDLE);
	lift_pos = MIDDLE;
}


void LiftCom::goToBottom() {
	LOG(INFO) << "[LIFT] goToBottom, previous is: " << lift_pos;
	writeToSerial(ARD_BOTTOM);
	lift_pos = BOTTOM;
}


void LiftCom::openGrabber() {
	LOG(INFO) << "[LIFT] openGrabber, previous is: " << grabber_pos;
	PRINTLINE("[LIFT] openGrabber, previous is: " << grabber_pos);
	writeToSerial(OPEN);
	grabber_pos = OPEN;
}


void LiftCom::closeGrabber() {
	LOG(INFO) << "[LIFT] closeGrabber, previous is: " << grabber_pos;	
	PRINTLINE("[LIFT] closeGrabber, previous is: " << grabber_pos);

	writeToSerial(CLOSE);
	grabber_pos = CLOSE;
}


uint8_t LiftCom::getPosition() {
	/*writeToSerial(GET);
	usleep(50);
	uint8_t pos = readFromSerial();
	LOG(INFO) << "Lift is at: " << pos;
	return pos;*/
	return lift_pos;
}


bool LiftCom::test() {
	if(port->available()) {
		return true;
	} else {
		usleep(1000);
		if(port->available()) {
			return true;
		}
	}
	return false;
}


//used to simulate lift while testing
void LiftCom::setCurrentPos(int p) {
	lift_pos = p;
}


bool LiftCom::waitForResponse() {
	auto t_start = std::chrono::high_resolution_clock::now();
    auto t_end = std::chrono::high_resolution_clock::now();    

	do {
		if(port->available()) {
			uint8_t resp = readFromSerial();
			LOG(INFO) << "[LIFT resp after: "  << (std::chrono::duration<double, std::milli>(t_end-t_start).count());

			if(resp == SUCCESS) {
				return true;
			} 	
			else {
				return false;
			}
		}
		t_end = std::chrono::high_resolution_clock::now();
    } while ((std::chrono::duration<double, std::milli>(t_end-t_start).count()) < 100); //time needs testing
    return false;
}



/*** Private Functions ***/


void LiftCom::writeToSerial(uint8_t a) {
	port->write(a);
}


void LiftCom::writeToSerial(int a) {
	port->write(a);
}


uint8_t LiftCom::readFromSerial() {
	return port->read();
}
