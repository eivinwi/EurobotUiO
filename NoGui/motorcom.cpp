#include "motorcom.h"

MotorCom::MotorCom() {
  	//serial_port = "ACM0";

    prev_encL = 0;
    prev_encR = 0;
    simulating = false;
}

MotorCom::~MotorCom() {
}

void MotorCom::startSerial() {
	if(simulating) {
        DBP("MOTORCOM: Starting serialsim\n")
		simport = new SerialSim;
	} else {
        if(strcmp(serial_port, "") == 0) {
            PRINTLINE("Error, empty serial_port. Setting to /dev/ttyACM0");
            strcpy(serial_port, "ttyACM0");
        }     
        DBP("MOTORCOM:  serial\n")
		port = new Serial(serial_port);
	}
}

void MotorCom::serialSimEnable() {
    PRINTLINE("MOTORCOM: Simulating=true\n");
	simulating = true;
}

void MotorCom::serialSimDisable() {
    DBP("MOTORCOM: Simulating=false\n");
    simulating = false;
}

void MotorCom::setSerialPort(const char *s) {
    DBPL("MOTORCOM: serial_port=" << s)
	strcpy(serial_port, s);
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

//input 0 [default], 1, 2 or 3
void MotorCom::setMode(uint8_t mode) {
    sync();
    writeToSerial(SET_MODE);
    writeToSerial(mode);
}


void MotorCom::resetEncoders() {
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
    long result1;
    long result2;

    result1 = readFromSerial() << 24ul;
    result1 += readFromSerialNoWait() << 16ul;
    result1 += readFromSerialNoWait() << 8ul;
    result1 += readFromSerialNoWait();

    result2 = readFromSerialNoWait() << 24ul;
    result2 += readFromSerialNoWait() << 16ul;
    result2 += readFromSerialNoWait() << 8ul;
    result2 += readFromSerialNoWait();
    DBPL("MOTORCOM: EncL: " << result1 << "\nEncR" << result2); 
}




long MotorCom::getEncL() {
    sync();
    writeToSerial(GET_ENCODERL);
    long result = readLongFromSerial();
    //DBPL("MOTORCOM: encoder update\n" << ss);
    DBPL("EncL: " << result << " (diff: " << (result - prev_encL) << ")\nWheel rotations: " <<  ((result-prev_encL)/980.0) << 
                "\nDistance: " << diff*0.385);
    prev_encL = result;
    return result;
}

long MotorCom::getEncR() {
    sync();
    writeToSerial(GET_ENCODERR);
    long result = readLongFromSerial();
    DBPL("EncR: " << result << " (diff: " << (result - prev_encR) << ")\nWheel rotations: " 
        << (diff/980.0) << "\nDistance: " << ((result - prev_encR)*0.385));
    prev_encR = result;
    return result;
}

uint8_t MotorCom::getVoltage() {
    DBPL("MOTORCOM: getVoltage");
    sync();
    writeToSerial(GET_VOLT);
    int volt = 0;
    volt = readFromSerial(); //readNoWait();
    DBPL("MOTORCOM: volt: " << volt << " (int val is: " << (int) volt << ")");
    return volt;
}

long MotorCom::getVi() {
    long vi = 0;
    sync();
    writeToSerial(GET_VI);
    vi = readFromSerial() << 16ul;
    vi += readFromSerialNoWait() << 8ul;
    vi += readFromSerialNoWait();
    return vi;
}

uint8_t MotorCom::getMode() {
    sync();
    writeToSerial(GET_MODE);
    int mode = 0;
    mode = readFromSerial();
    return mode;
}

uint8_t MotorCom::getVersion() {
    sync();
    writeToSerial(GET_VERSION);
    int version = 0;
    version = readFromSerial();
    return version;
}

uint8_t MotorCom::getError() {
    sync();
    writeToSerial(GET_ERROR);
    uint8_t error = 0;
    error = readFromSerial();
    return error;
}

void MotorCom::enableReg(bool b) {
    sync();
    if(b) {
        writeToSerial(ENABLE_REGULATOR);
    } else {
        writeToSerial(DISABLE_REGULATOR);
    }
}

void MotorCom::enableTimeout(bool b) {
    sync();
    if(b) {
        writeToSerial(ENABLE_TIMEOUT);
    } else {
        writeToSerial(DISABLE_TIMEOUT);
    }
}

void MotorCom::flush() {
    if(simulating) {
        simport->printAll();
    } else {
        port->printAll();
    }
}

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

/*int MotorCom::readFromSerial() {
    return port->read();
}*/

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