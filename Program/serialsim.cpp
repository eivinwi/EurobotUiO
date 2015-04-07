/*
 *  File: serialsim.cpp
 *  Author: Eivind Wikheim
 *
 *	SerialSim simulates a serial-connection to the MD49 motor controller.
 *	Relatively realistic encoder-values are calculated based on set motorspeed and time 
 * 	since last encoder-request (or since program start).
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

#include "serialsim.h"

SerialSim::SerialSim() {
	speedL = 255;
	speedR = 255;
	acceleration = 5;
	encoderL = 0;
	encoderR = 0;
	volt = 24;
	currentL = 2;
	currentR = 3;
	version = 2;
	mode = 0;
	error = 0;
	regulator = 1;
	timeout = 0;
	timeL = 0;
	timeR = 0;
	readCounter = 0;
	synced = false;
	prev_cmd = 0x00;
	encoderL = 0;
	encoderR = 0;
}


SerialSim::~SerialSim() {
}


// overload hack
void SerialSim::write(char arg) {
	write((uint8_t) arg);
}


// overload hack
void SerialSim::write(int arg) {
	write((uint8_t) arg);
}


void SerialSim::write(uint8_t arg) {
	LOG(DEBUG) << "[SIM] write " << arg;
	if(arg == 0x00) {
		synced = true;
		prev_cmd = 0x00;
	} 
	else if(synced && (prev_cmd == 0x00)) {
		// Already synced
		// reading second byte, defines command
		prev_cmd = arg;
		switch(arg){
	        //2byte in  -- 0 out
			case RESET_ENCODERS:
				encoderL = 0;
				encoderR = 0;
				synced = false;
	        	break;
			case DISABLE_REGULATOR:
				regulator = 0;
	        	synced = false;
	        	break;
			case ENABLE_REGULATOR:
				regulator = 1;
	        	synced = false;
	        	break;
			case DISABLE_TIMEOUT:
				timeout = 0;
	        	synced = false;
	        	break;
			case ENABLE_TIMEOUT:
				timeout = 1;
	        	synced = false;
	        	break;

	        //3byte in -- 0 out 
	        //set prev_cmd to use on next input
			case SET_SPEEDL:
				//std::cout << "setSpeedL\n";
	        	break;
			case SET_SPEEDR:
	        	break;
			case SET_ACCELERATION:
	        	break;
			case SET_MODE:
	        	break;

		    //2byte in -- 1 out
	        //store prev_cmd to use on next read
	        case GET_SPEEDL:
	        	break;
			case GET_SPEEDR:
	        	break;
			case GET_VOLT:
	        	break;
			case GET_CURRENTL:
	        	break;
			case GET_CURRENTR:
	        	break;
			case GET_VERSION:
	        	break;
			case GET_ACCELERATION:
	        	break;
			case GET_MODE:
	        	break;
			case GET_ERROR:
	        	break;

	        //2byte in -- multiple return, special cases
			case GET_ENCODERL:
				calculateEncL();
				readCounter = 0;
				break;
			case GET_ENCODERR:
				calculateEncR();
				readCounter = 0;
	        	break;
			case GET_ENCODERS:
				calculateEncL();
				calculateEncR();
	        	readCounter = 0;
	        	break;
			case GET_VI:
				readCounter = 0;
	        	break;
	        default:
	        	//PRINTLINE("Invalid serial-cmd in [SIM] " << arg);
		    	break;

		}
	} 
	else {
		//recieved third argument; speed/acc/mode etc.
		switch(prev_cmd) {
			case SET_SPEEDL:
				time(&timeL);		//store time speed was set, for use in encoder calculations
	        	speedL = arg;
	        	break;
			case SET_SPEEDR:
				time(&timeR);		//store time speed was set, for use in encoder calculations
	        	speedR = arg;
	        	break;
			case SET_ACCELERATION:
	        	acceleration = arg;
	        	break;
			case SET_MODE:
				mode = arg;
	        	break;
		}
	}
}


uint8_t SerialSim::readNoWait() {
	LOG(DEBUG) << "[SIM] readNoWait " << prev_cmd;
	switch(prev_cmd) {
        case GET_SPEEDL:
        	return speedL;
        	break;
		case GET_SPEEDR:
			return speedR;
        	break;
		case GET_VOLT:
        	return volt;
        	break;
		case GET_CURRENTL:
			return currentL;
        	break;
		case GET_CURRENTR:
        	return currentR;
        	break;
		case GET_VERSION:
			return version;
        	break;
		case GET_ACCELERATION:
			return acceleration;
        	break;
		case GET_MODE:
			return mode;
        	break;
		case GET_ERROR:
			return error;
        	break;

        case GET_VI:
        	return readVi();
        	break;
        case GET_ENCODERL:
        	return readEncL();
        	break;
        case GET_ENCODERR:
        	return readEncR();
        	break;
        case GET_ENCODERS:
        	return readEncs();
        	break;

        default:
        	break;
    }
    return 0x00;
}


uint8_t SerialSim::read() {
	LOG(DEBUG) << "[SIM] read";
	return readNoWait();
}


//TODO: check what should be returned
long SerialSim::readLong() {
	return encoderL;
}


uint8_t SerialSim::readVi() {
	readCounter++;
	if(readCounter == 1) {
		return volt;
	} else if(readCounter == 2) {
		return currentL;
	} else {
		readCounter = 0;
		return currentL;
	}
}


uint8_t SerialSim::readEncL() {
	uint8_t b = 0x00;
	if(readCounter < 4) {
		b = encL[readCounter];
		readCounter++;	
	}
	return b;
}


uint8_t SerialSim::readEncR() {
	uint8_t b = 0x00;
	if(readCounter < 4) {
		b = encR[readCounter];	
		readCounter++;	
	}
	return b;
}


uint8_t SerialSim::readEncs() {
	uint8_t b = 0x00;
	if(readCounter < 4) {
		b = encL[readCounter];
		readCounter++;	
	} else if(readCounter < 8) {
		b = encR[readCounter - 4];
		readCounter++;	
	}
	return b;
}


void SerialSim::calculateEncL() {
	time_t now;
	time(&now);
	double timePassed = difftime(now, timeL);
	double rotations = (timePassed)/RPS;
	double encoderDiff = rotations*980;
	encoderL = (long) encoderDiff;
	
	encL[3] = ((encoderL >> 24) & 0xff);
	encL[2] = ((encoderL >> 16) & 0xff);
	encL[1] = ((encoderL >> 8) & 0xff);
	encL[0] = (encoderL & 0xff);
}


void SerialSim::calculateEncR() {
	time_t now;
	time(&now);
	double timePassed = difftime(now, timeR);
	double rot = (timePassed)/RPS;
	double encoderDiff = rot*980;
	encoderR = (long) encoderDiff;

	encR[3] = ((encoderR >> 24) & 0xff);
	encR[2] = ((encoderR >> 16) & 0xff);
	encR[1] = ((encoderR >> 8) & 0xff);
	encR[0] = (encoderR & 0xff);
}


void SerialSim::printAll() {
	DBPL("Sim \"flushed\"");
}