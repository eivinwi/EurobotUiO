/*
 *	File: dynacom.cpp
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

struct shutter {
    int ID;
    int open;
    int closed;
} left_shutter, right_shutter;

struct gripper {
    int ID;
    int speed;
    int start;
    int open;
    int wide;
    int straight;
    int closed;
    int goal;
} left_gripper, right_gripper;


DynaCom::DynaCom(std::string serial, bool sim_enabled) {
    serial_port = serial;
    simulating = sim_enabled;
    left_gripper = {2, 145, 750, -100, -200, -300, -400, 750-100};
	right_gripper = {1, 145, 280, 100, 200, 300, 400, 280+100};
	left_shutter = {4, 500, 800};
	right_shutter = {3, 523, 223};
}


DynaCom::~DynaCom() {
}


void DynaCom::startSerial() {
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
	setSpeed(left_gripper.ID, left_gripper.speed);
    setSpeed(right_gripper.ID, left_gripper.speed);
}



void DynaCom::performAction(std::vector<int> cmd) {
	if(cmd.size() < 3) {
		LOG(WARNING) << "[DYNA] invalid cmd, size=" << cmd.size();
		return;
	}
	int type = cmd[0];
	//int id = cmd[1];
	int action = cmd[2];

	//LOG(INFO) << "[DYNA] performAction: " << cmd[0] << "," << cmd[1] << "," << cmd[2] << "," << cmd[3] << "," << cmd[4]; 

	if( type == LIFT ) {
		//deprecated
		//shutter now
		switch(action) {
			case 0:
				LOG(WARNING) << "[DYNA] lift_bottom, deprecated.";
				break;
			case 1:
				LOG(WARNING) << "[DYNA] lift_mid, deprecated.";
				break;
			case 2:
				LOG(WARNING) << "[DYNA] lift_top, deprecated.";
				break;
			case 3:
				//gripper_open
				LOG(WARNING) << "[DYNA] gripper_open, deprecated.";
				break;
			case 4:
				LOG(WARNING) << "[DYNA] gripper_close, deprecated.";
				break;
			case 5:
				shutterOpenLeft();
				break;
			case 6:
				shutterCloseLeft();
				break;
			case 7:
				shutterOpenRight();
				break;
			case 8:
				shutterCloseRight();
				break;
			default:
				break;
		} 
	} 
	else if( type == GRIPPER ) {
		switch(action) {
			case 0:
				if(cmd.size() > 3) {
					setGrippers(cmd[3], cmd[4]);
				}
				break;
			case 1:
				setLeftGripper(cmd[3]);
				break;
			case 2:
				setRightGripper(cmd[3]);
				break;
			case 3:
				packGrippers();
				break;
			case 4: 
				unpackGrippers();
			default:
				break;
		}
		usleep(1000000);
	}
}


void DynaCom::toggleLed(int id) {
	led = !led;
	setReg1(id, 25, led);
	usleep(ACTION_DELAY);
}


void DynaCom::setLeftGripper(int pos) {
	if( (pos <= 400) && (pos >= 0) ) {
		left_gripper.goal = pos;
		setPosition(left_gripper.ID, left_gripper.start - pos);
	}
}


void DynaCom::setRightGripper(int pos) {
	if( (pos <= 400) && (pos >= 0) ) {
		right_gripper.goal = pos;
		setPosition(right_gripper.ID, right_gripper.start + pos);	
	}
}


void DynaCom::setGrippers(int left_pos, int right_pos) {
	setLeftGripper(left_pos);
	setRightGripper(right_pos);
} 


std::string DynaCom::getGripperPosition() {
	int left = readPosition(left_gripper.ID);
	int right = readPosition(right_gripper.ID);
	int normalized_left = left_gripper.start - left;
	if( (abs(normalized_left) - left_gripper.goal) < 30 ) {
		normalized_left = left_gripper.goal;
	}

	int normalized_right = right - right_gripper.start;
	if( (abs(normalized_right) - right_gripper.goal) < 30 ) {
		normalized_right = right_gripper.goal;
	}
	
	std::stringstream ss;
	ss << normalized_left << "," << left_gripper.goal << "," << normalized_right << "," << right_gripper.goal;
	return ss.str();
}


bool DynaCom::testGripper() {
	LOG(DEBUG) << "[DYNA] Testing gripper";
	return test(left_gripper.ID);
}


bool DynaCom::testShutter() {
	LOG(DEBUG) << "[DYNA] Testing shutters";
	return test(left_shutter.ID) && test(right_shutter.ID);
}


/*** PRIVATE FUNCTIONS ***/
void DynaCom::packGrippers() {
	setPosition(left_gripper.ID, left_gripper.start - 580);	
	usleep(450000);
	setPosition(right_gripper.ID, right_gripper.start + 500);	
	usleep(100000);
} 

void DynaCom::unpackGrippers() {
	setPosition(right_gripper.ID, right_gripper.start + 100);	
	usleep(100000);
	setPosition(left_gripper.ID, left_gripper.start - 100);	
	usleep(100000);
} 



void DynaCom::shutterOpenLeft() {
	setPosition(left_shutter.ID, left_shutter.open);
}


void DynaCom::shutterCloseLeft() {
	setPosition(left_shutter.ID, left_shutter.closed);
}


void DynaCom::shutterOpenRight() {
	setPosition(right_shutter.ID, right_shutter.open);
}


void DynaCom::shutterCloseRight() {
	setPosition(right_shutter.ID, right_shutter.closed);
}

bool DynaCom::test(int id) {
	uint8_t testval = 0;
	if(simulating) {
		return true;
	}
	else {
		for(int i = 0; i < 5; i++) {
			regRead(left_gripper.ID, 0, 2);
			usleep(1000);
			testval = (uint8_t) returnValue(readByte());
			if(testval == 0X0C) {
				return true;
			}
		}
	} 
	return false;
}


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


void DynaCom::setPosition(int id, int pos) {
	LOG(INFO) << "[DYNA] setPosition(" << id << "," << pos << ")";
	setReg2(id, 30, pos);
  	usleep(ACTION_DELAY);
}


int DynaCom::readPosition(int id) {
	if(simulating) {
		return true;
	}
	regRead(id, 36, 2); //(int id, int firstRegAdress, int noOfBytesToRead)
  	usleep(ACTION_DELAY*10);

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
	return (uint8_t) ~counter; // inverting bits and adding checkSum
}


void DynaCom::sendCmd(int id, int cmd) {
	std::array <uint8_t, 6> arr {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
	arr[2] = (uint8_t) id;
	arr[3] = (uint8_t) (2); //adding length
	arr[4] = (uint8_t) cmd;
	arr[5] = calcCheckSum(arr);
	writeToSerial(arr);
}


void DynaCom::setReg1(int id, int regNo, int val) {
	std::array <uint8_t, 8> arr {0xFF, 0xFF, 0, 0, 3, 0, 0, 0}; 
	arr[2] = (uint8_t) id;
	arr[3] = (uint8_t) (4); // adding length
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