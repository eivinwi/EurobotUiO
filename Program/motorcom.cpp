/*
 *  File: motorcom
 *  Author: Eivind Wikheim
 *
 *  Implements an interface for working with the MD49 motor controller, 
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
            serial_port = "/dev/ttyUSB0";
        }     
        LOG(INFO) << "[MOTOR]    Starting serial at: " << serial_port;
        port = new Serial(serial_port);
        LOG(INFO) << "[MOTOR]    Serial opened";
    }
}


bool MotorCom::test() {
    return ((simulating)? true : (getVersion() == 2));
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
    LOG(DEBUG) << "[MCOM] left_speed has been set to: " << int(speed);
}


void MotorCom::setSpeedR(uint8_t speed) {
    sync();
    writeToSerial(SET_SPEEDR);
    writeToSerial(speed);
    LOG(DEBUG) << "[MCOM] right_speed has been set to: " << int(speed);
}


void MotorCom::setSpeedBoth(uint8_t speed) {
    setSpeedL(speed);
    setSpeedR(speed);
}


uint8_t MotorCom::getSpeedL() {
    sync();
    writeToSerial(GET_SPEEDL);
    return (uint8_t) readFromSerial();
}


uint8_t MotorCom::getSpeedR() {
    sync();
    writeToSerial(GET_SPEEDR);
    return (uint8_t) readFromSerial();
}


uint8_t MotorCom::getAcceleration() {
    sync();
    writeToSerial(GET_ACCELERATION);
    return (uint8_t) readFromSerial();
}


uint8_t MotorCom::getVoltage() {
    sync();
    writeToSerial(GET_VOLT);
    return (uint8_t) readFromSerial(); 
}


long MotorCom::getVi() {
    sync();
    writeToSerial(GET_VI);
    return readLongFromSerial();
}


uint8_t MotorCom::getMode() {
    sync();
    writeToSerial(GET_MODE);
    return (uint8_t) readFromSerial();
}


uint8_t MotorCom::getVersion() {
    sync();
    writeToSerial(GET_VERSION);
    return (uint8_t) readFromSerial(); 
}


uint8_t MotorCom::getError() {
    sync();
    writeToSerial(GET_ERROR);
    return (uint8_t) readFromSerial(); 
}


//input 0 [default], 1, 2 or 3
void MotorCom::setMode(uint8_t mode) {
    sync();
    writeToSerial(SET_MODE);
    writeToSerial(mode);
}


void MotorCom::resetEncoders() {
    sync();
    writeToSerial(RESET_ENCODERS);
}


std::tuple<long, long> MotorCom::getEncoders() {
    sync();
    writeToSerial(GET_ENCODERS);
    long l = readLongFromSerial();
    long r = readLongFromSerial();
    return std::make_tuple(l, r); 
}


long MotorCom::getEncL() {
    sync();
    writeToSerial(GET_ENCODERL);
    return readLongFromSerial();
}


long MotorCom::getEncR() {
    sync();
    writeToSerial(GET_ENCODERR);
    return readLongFromSerial();
}


void MotorCom::enableReg(bool enable) {
    sync();
    (enable)? writeToSerial(ENABLE_REGULATOR) : writeToSerial(DISABLE_REGULATOR);
}


void MotorCom::enableTimeout(bool enable) {
    sync();
    (enable)? writeToSerial(ENABLE_TIMEOUT) : writeToSerial(DISABLE_TIMEOUT);
}


void MotorCom::flush() {
   (simulating) ? simport->flush() : port->flush();
}


/***  PRIVATE FUNCTIONS ***/


void MotorCom::sync() {
    writeToSerial(CLEAR);
}


void MotorCom::writeToSerial(uint8_t a) {
    (simulating)? simport->write(a) : port->write(a);
}


void MotorCom::writeToSerial(int a) {
    (simulating)? simport->write(a) : port->write(a);
}


uint8_t MotorCom::readFromSerial() {
    return ((simulating) ? simport->read() : port->read());
}


uint8_t MotorCom::readFromSerialNoWait() {
    return ((simulating)? simport->readNoWait() : port->readNoWait());
}


long MotorCom::readLongFromSerial() {
    return ((simulating)? simport->readLong() : port->readLong());
}


bool MotorCom::isSimulating() {
    return simulating;
}