#include "motorcom.h"


MotorCom::MotorCom() {
    prev_encL = 0;
    prev_encR = 0;
    simulating = false;
}


MotorCom::~MotorCom() {
}


bool MotorCom::test() {
    if(simulating) {
        return true; 
    } 
    else {
        int ver = getVersion();
        usleep(1000);
        ver = getVersion();
        usleep(500);
        if(ver == 2) {
            return true;    
        } else {
            return false;
        }
    }
}


void MotorCom::startSerial() {
	if(simulating) {
        DBP("   [MOTOR] Starting serialsim\n")
		simport = new SerialSim;
	} else {
        if(strcmp(serial_port, "") == 0) {
            PRINTLINE("    Error, empty serial_port. Setting to /dev/ttyACM0");
            strcpy(serial_port, "ttyACM0");
        }     
        PRINTLINE("    [MOTOR] Starting serial at: " << serial_port)
		port = new Serial(serial_port);
	}
}


void MotorCom::serialSimEnable() {
    PRINTLINE("    [MOTOR] Simulating=true\n");
	simulating = true;
}


void MotorCom::serialSimDisable() {
    DBP("   [MOTOR] Simulating=false\n");
    simulating = false;
}


void MotorCom::setSerialPort(const char *s) {
    DBPL("  [MOTOR] serial_port=" << s)
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
    return readFromSerial();
}


uint8_t MotorCom::getVoltage() {
    DBPL("[MOTOR] getVoltage");
    sync();
    writeToSerial(GET_VOLT);
    int volt = 0;
    volt = readFromSerial(); //readNoWait();
    DBPL("[MOTOR] volt: " << volt << " (int val is: " << (int) volt << ")");
    return volt;
}


long MotorCom::getVi() {
    long vi = 0;
    sync();
    writeToSerial(GET_VI);
    vi = readLongFromSerial();
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
    uint8_t version = 0;
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


//input 0 [default], 1, 2 or 3
void MotorCom::setMode(uint8_t mode) {
    sync();
    writeToSerial(SET_MODE);
    writeToSerial(mode);
}


void MotorCom::resetEncoders() {
    DBPL("  [MOTOR] Resetting encoders");
    sync();
    writeToSerial(RESET_ENCODERS);
    prev_encL = 0;
    prev_encR = 0;
}


/* unused?
 *
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
    DBPL("[MOTOR] EncL: " << result1 << "\nEncR" << result2); 
}


long MotorCom::getEncL() {
    sync();
    writeToSerial(GET_ENCODERL);
    long result = readLongFromSerial();
    DBPL("[MOTOR] EncL: " << result << " (diff: " << (result - prev_encL) << ")\nWheel rotations: " <<  ((result-prev_encL)/980.0) << 
                "\nDistance: " << (result - prev_encL)*0.385);
    prev_encL = result;
    return result;
}


long MotorCom::getEncR() {
    sync();
    writeToSerial(GET_ENCODERR);
    long result = readLongFromSerial();
    DBPL("[MOTOR] EncR: " << result << " (diff: " << (result - prev_encR) << ")\nWheel rotations: " 
        << ((result - prev_encR)/980.0) << "\nDistance: " << ((result - prev_encR)*0.385));
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