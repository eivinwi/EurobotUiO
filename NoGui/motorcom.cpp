#include "motorcom.h"

MotorCom::MotorCom() {
  	simulating = false;
  	//serial_port = "ACM0";
  	strcpy(serial_port, "ACM0");

    prev_encL = 0;
    prev_encR = 0;
}

MotorCom::~MotorCom() {
}

void MotorCom::startSerial() {
	if(simulating) {
		simport = new SerialSim;
	} else {
		port = new Serial(serial_port);
	}
}

void MotorCom::serialSimEnable() {
	simulating = true;
}

void MotorCom::serialSimDisable() {
	simulating = false;
}

void MotorCom::setSerialPort(const char *s) {
	//serial_port = s;
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
    DBP("EncL: "); 
    DBP(result1);
    DBP("\nEncR: "); 
    DBP(result2);
    DBP("\n");
}




long MotorCom::getEncL() {
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

long MotorCom::getEncR() {
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

uint8_t MotorCom::getVoltage() {
    sync();
    writeToSerial(GET_VOLT);
    int volt = 0;
    volt = readFromSerial(); //readNoWait();
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
    port->printAll();
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