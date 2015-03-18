/*
 *	File: liftcom.cpp
 *	Author: Eivind Wikheim
 *
 *  DynaCom implements a interface controlling lift/gripper parts via an arduino, 
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

#include "dynacom.h"

DynaCom::DynaCom(std::string serial, bool sim_enabled) {
    serial_port = serial;
    simulating = sim_enabled;
}


DynaCom::~DynaCom() {
}


void DynaCom::startSerial() {
	port = new Serial("/dev/ttyUSB0");
	usleep(500000);
	return; 

	PRINTLINE("StartSerial");
	if(simulating) {
	    LOG(INFO) << "[DYNA] 	 Simulating serial";		
	}
	else {        
	    if(serial_port == "") { //should never happen, but check just in case
	        LOG(WARNING) << "[DYNA] 	Error, empty serial_port. Setting to /dev/ttyUSB1";
	        serial_port = "/dev/ttyUSB1";
	    }     
	    LOG(INFO) << "[DYNA] 	 Starting serial at: " << serial_port;
		port = new Serial(serial_port);
	}
}

void DynaCom::performAction(int arg) {
	switch(arg) {
		case 0:
			liftDown();
			break;
		case 1:
			liftMiddle();
			break;
		case 2:
			liftUp();
			break;
		case 3:
			openGrip();
			break;
		case 4:
			closeGrip();
			break;
		default:
			break;
	}
}


void DynaCom::toggleLed(int id) {
	led = !led;
	setReg1(id, 25, led);
	usleep(ACTION_DELAY);
}


int DynaCom::liftPosition() {
	int pos = readPosition(LIFT_ID);
	if((pos - TOP_POS) < 10) {
		return TOP_STATE;
	} 
	else if((pos - MIDDLE_POS) < 10) {
		return MIDDLE_STATE;
	} 
	else if((pos - BOTTOM_STATE) < 10) {
		return BOTTOM_STATE;
	}
	return INVALID_STATE;
}


int DynaCom::gripperPosition() {
	int pos = readPosition(GRIP_ID);
	if((pos - OPEN_POS) < 10 ) {
		return OPEN_STATE;
	} 
	else if((pos - CLOSED_POS) < 10) {
		return CLOSED_STATE;
	} 
	return INVALID_STATE;
}

void DynaCom::openGrip() {
	LOG(INFO) << "[DYNA] openGrip";
	setPosition(GRIP_ID, OPEN_POS);
} 

void DynaCom::closeGrip() {
	LOG(INFO) << "[DYNA] closeGrip";
	setPosition(GRIP_ID, CLOSED_POS);
}

void DynaCom::liftUp() {
	LOG(INFO) << "[DYNA] liftUp";
	setPosition(LIFT_ID, TOP_POS);
}


void DynaCom::liftMiddle() {
	LOG(INFO) << "[DYNA] liftMiddle";
	setPosition(LIFT_ID, MIDDLE_POS);
}


//TODO: find correct position
void DynaCom::liftDown() {
	LOG(INFO) << "[DYNA] liftDown";
	setPosition(LIFT_ID, BOTTOM_POS);
}


bool DynaCom::testLift() {
	LOG(DEBUG) << "[DYNA] Testing lift";
	return test(LIFT_ID);
}

bool DynaCom::testGripper() {
	LOG(DEBUG) << "[DYNA] Testing gripper";
	return test(GRIP_ID);
}

bool DynaCom::test(int id) {
	uint8_t testval = 0;
	if(simulating) {
		return true;
	}
	else {
		for(int i = 0; i < 5; i++) {
			regRead(LIFT_ID, 0, 2);
			usleep(1000);
			testval = (uint8_t) returnValue(readByte());
			if(testval == 0X0C) {
				return true;
			}
		}
	} 
	return false;
}




/*** PRIVATE FUNCTIONS ***/


void DynaCom::setSpeed(int id, int speed) {
	if(speed >= 0 && speed < 1024) {
		setReg2(id, 32, speed);
	}
	usleep(ACTION_DELAY);
}


void DynaCom::setMaxTorque(int id, int torque) {
	if(torque >= 100 && torque < 1024) {
		setReg2(id, 14, torque);
	}
	usleep(ACTION_DELAY);
}


void DynaCom::setPosition(int id, int angle) {
	std::cout << "SetPosition(" << id << "): " << angle << std::endl;
	setReg2(id, 30, angle);
  	usleep(ACTION_DELAY*100);
}


int DynaCom::readPosition(int id) {
	if(simulating) {
		return true;
	}
	regRead(id, 36, 2); //(int id, int firstRegAdress, int noOfBytesToRead)
  	usleep(ACTION_DELAY*100);

	return (int) returnValue( readByte() );
}


void DynaCom::writeToSerial(uint8_t b[], int len) {
	if(!simulating) {
		for(int i = 0; i < len; i++) {
			port->write(b[i]);
		}
	}
}


template<std::size_t SIZE> 
void DynaCom::writeToSerial(std::array<uint8_t, SIZE> array) {
	if(!simulating) {
		for(uint8_t b : array) {
			port->write(b);
		}
	}
}


std::array <uint8_t, 8>  DynaCom::readByte() {
	LOG(DEBUG) << "[DYNA] starting read";
	std::array <uint8_t, 8> arr;
	for(int i = 0; i < 8; i++) {
		if(!simulating) {
			arr[i] = port->read();
		}
	}
	LOG(DEBUG) << "[DYNA] done reading";
	return arr;
}


int DynaCom::returnValue(std::array <uint8_t, 8> arr) {
	int val = 0;
	if(arr[3] == 3) {
		val =  (int) arr[5];
	} else if(arr[3] == 4) {
	    val += arr[6] << 8;
    	val += arr[5];
	}
	return val;
}


template<std::size_t SIZE> 
uint8_t DynaCom::calcCheckSum(std::array<uint8_t, SIZE> b) {
	int counter = 0;
	int tmp = 0;
	for (int i=2; i<(SIZE-1); i++) {
		tmp = (int) b.at(i);
		if (tmp < 0) {
	  		tmp = tmp + 256 ;
		}
		counter = counter + tmp;
	}
	//counter = ~counter; // inverting bits
	return (uint8_t) ~counter; // inverting bits and adding checkSum
}


void DynaCom::sendCmd(int id, int cmd) {
	std::array <uint8_t, 6> arr {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
	arr[2] = (uint8_t) id;
	arr[3] = (uint8_t) (2);
	arr[4] = (uint8_t) cmd;
	arr[5] = calcCheckSum(arr);
	writeToSerial(arr);
}


void DynaCom::setReg1(int id, int regNo, int val) {
	std::array <uint8_t, 8> arr {0xFF, 0xFF, 0, 0, 3, 0, 0, 0}; 
	arr[2] = (uint8_t) id;
	arr[3] = (uint8_t) (4);// adding length
	arr[5] = (uint8_t) regNo;
	arr[6] = (uint8_t) val;
	arr[7] = calcCheckSum(arr);
	writeToSerial(arr);
}


void DynaCom::setReg2(int id, int regNoLSB, int val) {
	std::array <uint8_t, 9> arr {0xFF, 0xFF, 0, 0, 3, 0, 0, 0, 0}; 
	arr[2] = (uint8_t) id;
	arr[3] = (uint8_t) (5);// adding length
	arr[5] = (uint8_t) regNoLSB;
	arr[6] = (uint8_t) ( val & 255 );
	arr[7] = (uint8_t) ( (val >> 8) & 255 );
	arr[8] = calcCheckSum(arr);
	writeToSerial(arr);
}


void DynaCom::regRead(int id, int firstRegAdress, int noOfBytesToRead) {
	std::array <uint8_t, 8> arr {0xFF, 0xFF, 0, 0, 2, 0X2B, 0X01, 0}; 
	arr[2] = (uint8_t) id;
	arr[3] = (uint8_t) (4);// adding length
	arr[5] = (uint8_t) firstRegAdress;
	arr[6] = (uint8_t) noOfBytesToRead;
	arr[7] = calcCheckSum(arr);
	writeToSerial(arr);
}