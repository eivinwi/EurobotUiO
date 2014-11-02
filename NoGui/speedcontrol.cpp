#include "speedcontrol.h"

SpeedControl::SpeedControl() {
  	simulating = false;
  	serial_port = "ACM0";

    prev_encL = 0;
    prev_encR = 0;
}

SpeedControl::~SpeedControl() {
}

void SpeedControl::startSerial() {
	if(simulating) {
		simport = new SerialSim;
	} else {
		port = new Serial(serial_port);
	}
}

void SpeedControl::serialSimEnable() {
	simulating = true;
}

void SpeedControl::serialSimDisable() {
	simulating = false;
}

void SpeedControl::setSerialPort(char *s) {
	serial_port = s;
}

void SpeedControl::setAcceleration(int acc) {
    sync();
    writeToSerial(SET_ACCELERATION);
    writeToSerial(acc);
}

void SpeedControl::setSpeedL(uint8_t speed) {
    sync();
    writeToSerial(SET_SPEEDL);
    writeToSerial(speed);
}

void SpeedControl::setSpeedR(uint8_t speed) {
    sync();
    writeToSerial(SET_SPEEDR);
    writeToSerial(speed);
}

void SpeedControl::setSpeedBoth(uint8_t speed) {
    setSpeedL(speed);
    setSpeedR(speed);
}

uint8_t SpeedControl::getSpeedL() {
    sync();
    writeToSerial(GET_SPEEDL);
    uint8_t speed = readFromSerial();
    return speed;
}

uint8_t SpeedControl::getSpeedR() {
    sync();
    writeToSerial(GET_SPEEDR);
    uint8_t speed = readFromSerial();
    return speed;
}

uint8_t SpeedControl::getAcceleration() {
    sync();
    writeToSerial(GET_ACCELERATION);
    uint8_t acc = readFromSerial();
    return acc;
}

//input 0 [default], 1, 2 or 3
void SpeedControl::setMode(uint8_t mode) {
    sync();
    writeToSerial(SET_MODE);
    writeToSerial(mode);
}


void SpeedControl::resetEncoders() {
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
void SpeedControl::getEncoders() {
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
    DBP("EncL: "); 
    DBP(result1);
    DBP("\nEncR: "); 
    DBP(result2);
    DBP("\n");
}




long SpeedControl::getEncL() {
    sync();
    writeToSerial(GET_ENCODERL);
    long result = 0;
    result = readFromSerial() << 24ul;
    std::bitset<32> r1(result);
    result += readFromSerialNoWait() << 16ul;
    std::bitset<32> r2(result);
    result += readFromSerialNoWait() << 8ul;
    std::bitset<32> r3(result);
    result += readFromSerialNoWait();
    long diff = result - prev_encL;

    std::stringstream ss;
    ss << "BITS: \n" << r1 << '\n' << r2 << '\n' << r3 << '\n' << result << "\n--------------\n"
        << "EncL: " << result << " (diff: " << diff << ")\nWheel rotations: " <<  (diff/980.0) << 
        "\nDistance: " << diff*0.385 << '\n';

    DBP(ss);
    prev_encL = result;
    return result;
}

long SpeedControl::getEncR() {
    sync();
    writeToSerial(GET_ENCODERR);
    long result = 0;
    result = readFromSerial() << 24ul;
    std::bitset<32> r1(result);
    result += readFromSerialNoWait() << 16ul;
    std::bitset<32> r2(result);
    result += readFromSerialNoWait() << 8ul;
    std::bitset<32> r3(result);
    result += readFromSerialNoWait();
    long diff = result - prev_encL;

    std::stringstream ss;
    ss << "BITS: \n" << r1 << '\n' << r2 << '\n' << r3 << '\n' << result << "\n--------------\n"
        << "EncL: " << result << " (diff: " << diff << ")\nWheel rotations: " <<  (diff/980.0) << 
        "\nDistance: " << diff*0.385 << '\n';

    DBP(ss);
    prev_encL = result;
    return result;
}

uint8_t SpeedControl::getVoltage() {
    sync();
    writeToSerial(GET_VOLT);
    int volt = 0;
    volt = readFromSerial(); //readNoWait();
    return volt;
}

long SpeedControl::getVi() {
    long vi = 0;
    sync();
    writeToSerial(GET_VI);
    vi = readFromSerial() << 16ul;
    vi += readFromSerialNoWait() << 8ul;
    vi += readFromSerialNoWait();
    return vi;
}

uint8_t SpeedControl::getMode() {
    sync();
    writeToSerial(GET_MODE);
    int mode = 0;
    mode = readFromSerial();
    return mode;
}

uint8_t SpeedControl::getVersion() {
    sync();
    writeToSerial(GET_VERSION);
    int version = 0;
    version = readFromSerial();
    return version;
}

uint8_t SpeedControl::getError() {
    sync();
    writeToSerial(GET_ERROR);
    uint8_t error = 0;
    error = readFromSerial();
    return error;
}

void SpeedControl::enableReg(bool b) {
    sync();
    if(b) {
        writeToSerial(ENABLE_REGULATOR);
    } else {
        writeToSerial(DISABLE_REGULATOR);
    }
}

void SpeedControl::enableTimeout(bool b) {
    sync();
    if(b) {
        writeToSerial(ENABLE_TIMEOUT);
    } else {
        writeToSerial(DISABLE_TIMEOUT);
    }
}

void SpeedControl::flush() {
    port->printAll();
}

void SpeedControl::sync() {
    writeToSerial(CLEAR);
}

void SpeedControl::writeToSerial(uint8_t a) {
	if(simulating) {
	    simport->write(a);
	} else {
		port->write(a);
	}
}

void SpeedControl::writeToSerial(int a) {
    if(simulating) {
	    simport->write(a);
	} else {
		port->write(a);
	}
}

/*int SpeedControl::readFromSerial() {
    return port->read();
}*/

uint8_t SpeedControl::readFromSerial() {
    if(simulating) {
	    return simport->read();
	} else {
		return port->read();
	}
}

uint8_t SpeedControl::readFromSerialNoWait() {
    if(simulating) {
	    return simport->readNoWait();
	} else {
		return port->readNoWait();
	}
}