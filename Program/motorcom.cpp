/*
 *  File: motorcom
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

#include "motorcom.h"

MotorCom::MotorCom(std::string serial, bool sim_enabled) {
    simulating = sim_enabled;
    prev_encL = 0;
    prev_encR = 0;
    serial_port = serial;
}


MotorCom::~MotorCom() {
}


void MotorCom::startSerial() {
    if(simulating) {
        LOG(INFO) << "[MOTOR]     Starting serial simulator";
        simport = new SerialSim;
    } 
    else {
        if(serial_port == "") { //should never happen
            LOG(WARNING) << "[MOTOR]    Error, empty serial_port. Setting to /dev/ttyUSB0";
            serial_port = "/ttyUSB0";
        }     
        LOG(INFO) << "[MOTOR]    Starting serial at: " << serial_port;
        port = new Serial(serial_port);
    }
}


bool MotorCom::test() {
    if(simulating) {
        return true; 
    } 
    else {
        int ver = getVersion();
        if(ver == 2) {
            return true;    
        } else {
            return false;
        }
    }
}


void MotorCom::setAcceleration(int acc) {
    sync();
    writeToSerial(SET_ACCELERATION);
    writeToSerial(acc);
}


void MotorCom::setSpeedL(uint8_t speed) {
    sync();
    writeToSerial(SET_SPEEDL);
    writeToSerial(speed);
}


void MotorCom::setSpeedR(uint8_t speed) {
    sync();
    writeToSerial(SET_SPEEDR);
    writeToSerial(speed);
}


void MotorCom::setSpeedBoth(uint8_t speed) {
    setSpeedL(speed);
    setSpeedR(speed);
}


uint8_t MotorCom::getSpeedL() {
    sync();
    writeToSerial(GET_SPEEDL);
    uint8_t speed = readFromSerial();
    return speed;
}


uint8_t MotorCom::getSpeedR() {
    sync();
    writeToSerial(GET_SPEEDR);
    uint8_t speed = readFromSerial();
    return speed;
}


uint8_t MotorCom::getAcceleration() {
    sync();
    writeToSerial(GET_ACCELERATION);
    uint8_t acc = readFromSerial(); 
    return acc;
}


uint8_t MotorCom::getVoltage() {
    sync();
    writeToSerial(GET_VOLT);
    int volt = readFromSerial();
    return volt;
}


long MotorCom::getVi() {
    sync();
    writeToSerial(GET_VI);
    long vi = readLongFromSerial();
    return vi;
}


uint8_t MotorCom::getMode() {
    sync();
    writeToSerial(GET_MODE);
    int mode = readFromSerial();
    return mode;
}


uint8_t MotorCom::getVersion() {
    sync();
    writeToSerial(GET_VERSION);
    uint8_t version = readFromSerial();
    return version;
}


uint8_t MotorCom::getError() {
    sync();
    writeToSerial(GET_ERROR);
    uint8_t error = readFromSerial();
    return error;
}


//input 0 [default], 1, 2 or 3
void MotorCom::setMode(uint8_t mode) {
    sync();
    writeToSerial(SET_MODE);
    writeToSerial(mode);
}


void MotorCom::resetEncoders() {
    LOG(DEBUG) << "[MOTOR] Resetting encoders";
    sync();
    writeToSerial(RESET_ENCODERS);
    prev_encL = 0;
    prev_encR = 0;
}


/* 
 * Encoder counts: 980 per output shaft turn
 * Wheel diameter: 120mm
 * Wheel circumference: 377mm
 * Distance per count: 0.385mm
 */
void MotorCom::getEncoders() {
    sync();
    writeToSerial(GET_ENCODERS);
    flush();
    long result1;// = readLongFromSerial();
    long result2;// = readLongFromSerial();
    result1 = readFromSerial() << 24ul;
    result1 += readFromSerialNoWait() << 16ul;
    result1 += readFromSerialNoWait() << 8ul;
    result1 += readFromSerialNoWait();


    result2 = readFromSerialNoWait() << 24ul;
    result2 += readFromSerialNoWait() << 16ul;
    result2 += readFromSerialNoWait() << 8ul;
    result2 += readFromSerialNoWait();
    LOG(DEBUG) << "[MOTOR] EncL: " << result1 << "\nEncR" << result2;
}


long MotorCom::getEncL() {
    sync();
    writeToSerial(GET_ENCODERL);
    long result = readLongFromSerial();
   // LOG(INFO) << "[MOTOR] EncL: " << result << " (diff: " << (result - prev_encL) << ")\nWheel rotations: " <<  ((result-prev_encL)/980.0) << 
     //           "\nDistance: " << (result - prev_encL)*0.385;
    prev_encL = result;
    return result;
}


long MotorCom::getEncR() {
    sync();
    writeToSerial(GET_ENCODERR);
    long result = readLongFromSerial();
    //LOG(INFO) << "[MOTOR] EncR: " << result << " (diff: " << (result - prev_encR) << ")\nWheel rotations: " 
    //    << ((result - prev_encR)/980.0) << "\nDistance: " << ((result - prev_encR)*0.385);
    prev_encR = result;
    return result;
}


void MotorCom::enableReg(bool enable) {
    sync();
    if(enable) {
        writeToSerial(ENABLE_REGULATOR);
    } else {
        writeToSerial(DISABLE_REGULATOR);
    }
}


void MotorCom::enableTimeout(bool enable) {
    sync();
    if(enable) {
        writeToSerial(ENABLE_TIMEOUT);
    } else {
        writeToSerial(DISABLE_TIMEOUT);
    }
}


void MotorCom::flush() {
    LOG(DEBUG) << "[MOTOR] Flushing serial";
    if(simulating) {
        simport->printAll();
    } else {
        port->printAll();
    }
}


/***  PRIVATE FUNCTIONS ***/


void MotorCom::sync() {
    writeToSerial(CLEAR);
}


void MotorCom::writeToSerial(uint8_t a) {
	if(simulating) {
	    simport->write(a);
	} else {
		port->write(a);
	}
}


void MotorCom::writeToSerial(int a) {
    if(simulating) {
	    simport->write(a);
	} else {
		port->write(a);
	}
}


uint8_t MotorCom::readFromSerial() {
    if(simulating) {
	    return simport->read();
	} else {
		return port->read();
	}
}


uint8_t MotorCom::readFromSerialNoWait() {
    if(simulating) {
	    return simport->readNoWait();
	} else {
		return port->readNoWait();
	}
}


long MotorCom::readLongFromSerial() {
    if(simulating) {
        return simport->readLong();
    } else {
        return port->readLong();
    }
}


bool MotorCom::isSimulating() {
    return simulating;
}