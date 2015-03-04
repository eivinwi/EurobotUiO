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

DynaCom::DynaCom(std::string serial) {
    serial_port = serial;
    state = OPEN_STATE;
 //   return_byte[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
}


DynaCom::~DynaCom() {
}


void DynaCom::startSerial() {        
    if(serial_port == "") { //should never happen
        LOG(WARNING) << "[LIFT] 	Error, empty serial_port. Setting to /dev/ttyACM1";
        serial_port = "/dev/ttyACM1";
    }     
    LOG(INFO) << "[LIFT] 	 Starting serial at: " << serial_port;
	port = new Serial(serial_port);
}


void DynaCom::writeToSerial(uint8_t b[], int len) {
	for(int i = 0; i < len; i++) {
		port->write(b[i]);
	}
}

template<std::size_t SIZE> 
void DynaCom::writeToSerial(std::array<uint8_t, SIZE> array) {
	for(uint8_t b : array) {
		port->write(b);
	}
}

// adds checksum and length bytes to the ASCII byte packet
// input needs to be of length 4, with the last 2 places empty
template<std::size_t SIZE> 
void DynaCom::addChecksumAndLength(std::array<uint8_t, SIZE> b) {
	b[3] = (uint8_t) (SIZE - 4);  // adding length

	// finding sum
	int counter = 0;
	for (int i=2; i<(SIZE-1); i++) {
		int tmp = (int)b[i];
		if (tmp < 0) {
	  		tmp = tmp + 256 ;
		}
		counter = counter + tmp;
	}
	counter = ~counter; // inverting bits
	counter = (uint8_t) counter;  // int2byte 
	
	b[SIZE-1] = (uint8_t) counter; // adding checkSum
}


void DynaCom::sendCmd(int id, int cmd) {
	std::array <uint8_t, 6> arr {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
	arr[2] = (uint8_t) id;
	arr[4] = (uint8_t) cmd;
	addChecksumAndLength(arr);
	writeToSerial(arr);
}


// Writes 0<val<255 to register "regNo" in servo "id" 
void DynaCom::setReg1(int id, int regNo, int val) {
	std::array <uint8_t, 8> arr {0xFF, 0xFF, 0, 0, 3, 0, 0, 0}; 

	arr[2] = (uint8_t) id;
	arr[5] = (uint8_t) regNo;
	arr[6] = (uint8_t) val;

	addChecksumAndLength(arr); 
	writeToSerial(arr);
}

// Writes 0<val<1023 to register "regNoLSB/regNoLSB+1" in servo "id"
void DynaCom::setReg2(int id, int regNoLSB, int val) {
	std::array <uint8_t, 9> arr {0xFF, 0xFF, 0, 0, 3, 0, 0, 0, 0}; 
	arr[2] = (uint8_t) id;
	arr[5] = (uint8_t) regNoLSB;
	arr[6] = (uint8_t) ( val & 255 );
	arr[7] = (uint8_t) ( (val >> 8) & 255 );
	addChecksumAndLength(arr); 

	//std::cout << "Byte sent: {"
	//	<< (int) b[0] << ", " << (int) b[1] << ", " << (int) b[2]  << ", " << (int) b[3] << ", " 	
	//	<< (int) b[4] << ", " << (int) b[5] << ", " << (int) b[6] << ", " << (int) b[7] << ", " << (int) b[8] << "]" << std::endl;
	writeToSerial(arr);
}


// Read from register, status packet printout is handled by readByte()
void DynaCom::regRead(int id, int firstRegAdress, int noOfBytesToRead) {
	// println(" "); // console newline before serialEvent() printout
	std::array <uint8_t, 8> arr {0xFF, 0xFF, 0, 0, 2, 0X2B, 0X01, 0}; 
	arr[2] = (uint8_t) id;
	arr[5] = (uint8_t) firstRegAdress;
	arr[6] = (uint8_t) noOfBytesToRead;
	addChecksumAndLength(arr); 
	writeToSerial(arr);
}


// called after sending a read command, to read a 8-byte return-packet
std::array <uint8_t, 8>  DynaCom::readByte() {
	std::array <uint8_t, 8> arr;
	for(int i = 0; i < 8; i++) {
		arr[i] = port->read();
	}
	return arr;
}


//set top speed
void DynaCom::setSpeed(int id, int speed) {
	if(speed >= 0 && speed < 1024) {
		setReg2(id, 32, speed);
	}
	usleep(ACTION_DELAY);
}


// set max torque
void DynaCom::setMaxTorque(int id, int torque) {
	if(torque >= 100 && torque < 1024) {
		setReg2(id, 14, torque);
	}
	usleep(ACTION_DELAY);
}


void DynaCom::toggleLed(int id) {
	if(led == 1) led = 0;
	else led = 1;

	setReg1(id, 25, led);
	usleep(ACTION_DELAY);
}


// sets a new goal position. Servo is configured to not go outside of safe ranges,
// but values are restricted, just in case
void DynaCom::setPosition(int id, int angle) {
	std::cout << "SetPosition(" << id << "): " << angle << std::endl;
	if(angle >= 215 && angle < 580) {
		setReg2(id, 30, angle);
	}
  	usleep(ACTION_DELAY*100);
}


// sets the gripper to OPEN position
bool DynaCom::openGrip() {
	PRINTLINE("DYNA: openGrip");
	if(state == CLOSED_STATE) {
		setPosition(id, 230);
		state = OPEN_STATE;
		return true;
	}
	return false;
} 


// sets the gripper to CLOSED position
bool DynaCom::closeGrip() {
	PRINTLINE("DYNA: closeGrip");
	if(state == OPEN_STATE) {
		setPosition(id, 530);
		state = CLOSED_STATE;
		return true;
	}
	return false;
}


//extracts value from _ packet
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


// sends request for current position, reads return packet and returns position as int
int DynaCom::getPosition() {
	regRead(id, 36, 2); //(int id, int firstRegAdress, int noOfBytesToRead)
	usleep(10000);

	int val = returnValue( readByte() );
//	std::cout << "[";
//	for(int i = 0; i < 7; i++) {
//		std::cout << (int) return_byte[i] << ", ";
//	}
//	std::cout << (int) return_byte[7] << "]" << std::endl;
//	std::cout << "Position is: " << val << std::endl;

	return val;
}


bool DynaCom::test() {
	regRead(id, 0, 2);
	int model_number = returnValue(readByte());

	return (model_number == 0x0C);
}