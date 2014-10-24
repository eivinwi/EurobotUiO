#include "speedcontrol.h"
#include <bitset>

SpeedControl::SpeedControl() {
    port = new Serial;
//	left = new Motor(LEFT, 0, 0);
    left = new Motor;
    left->id = LEFT;
    left->speed = 0;
    right = new Motor;
    right->id = RIGHT;
    right->speed = 0;

    prev_encL = 0;
    prev_encR = 0;
}

SpeedControl::~SpeedControl() {
    //free(&left);
    //free(&right);
}

void SpeedControl::setAcceleration(int acc) {
    sync();
    action(SET_ACCEL);
    port->write(acc);
}

void SpeedControl::setSpeedL(uint8_t speed) {
    left->speed = speed;
    sync();
    action(SET_SPEEDL);
    port->write(speed);
}

void SpeedControl::setSpeedR(uint8_t speed) {
    right->speed = speed;
    sync();
    action(SET_SPEEDR);
    port->write(speed);
}

void SpeedControl::setSpeedBoth(uint8_t speed) {
    setSpeedL(speed);
    setSpeedR(speed);
}

uint8_t SpeedControl::getSpeedL() {
    sync();
    action(GET_SPEEDL);
    uint8_t speed = port->read();
    left->speed = speed;
    return speed;
}

uint8_t SpeedControl::getSpeedR() {
    sync();
    action(GET_SPEEDR);
    uint8_t speed = port->read();
    right->speed = speed;
    return speed;
}

uint8_t SpeedControl::getAcceleration() {
    sync();
    action(GET_ACCEL);
    uint8_t acc = port->read();
    return acc;
}

//input 0 [default], 1, 2 or 3
void SpeedControl::setMode(uint8_t mode) {
    sync();
    action(SET_MODE);
    port->write(mode);
}


void SpeedControl::resetEncoders() {
    sync();
    action(RESET_ENC);
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
    action(GET_ENCODERS);
    flush();
    long result1;
    long result2;

    result1 = port->read() << 24ul;
    result1 += port->readNoWait() << 16ul;
    result1 += port->readNoWait() << 8ul;
    result1 += port->readNoWait();

    result2 = port->readNoWait() << 24ul;
    result2 += port->readNoWait() << 16ul;
    result2 += port->readNoWait() << 8ul;
    result2 += port->readNoWait();
    std::cout << "EncL: " << result1 << '\n';
    std::cout << "EncR: " << result2 << '\n';
}




long SpeedControl::getEncL() {
    sync();
    action(GET_ENCL);
    long result = 0;
    result = port->read() << 24ul;
    std::bitset<32> r1(result);
    result += port->readNoWait() << 16ul;
    std::bitset<32> r2(result);
    result += port->readNoWait() << 8ul;
    std::bitset<32> r3(result);
    result += port->readNoWait();

    long diff = result - prev_encL;

    std::cout << "BITS: \n" << r1 << '\n' << r2 << '\n'
              << r3 << '\n' << result << "\n--------------\n";

    std::cout << "EncL: " << result;
    std::cout << " (diff: " << diff << ")\n";
    std::cout << "Wheel rotations: " <<  (diff/980.0) << '\n';
    std::cout << "Distance: " << diff*0.385 << '\n';

    prev_encL = result;
    return result;
}

long SpeedControl::getEncR(long diff) {
    sync();
    action(GET_ENCR);
    long result = 0;
    result = port->read() << 24ul;
    result += port->readNoWait() << 16ul;
    result += port->readNoWait() << 8ul;
    result += port->readNoWait();
    diff = result - prev_encR;
    return result;
}

uint8_t SpeedControl::getVoltage() {
    sync();
    action(GET_VOLT);
    int volt = 0;
    volt = port->read(); //readNoWait();
    return volt;
}

long SpeedControl::getVi() {
    long vi = 0;
    sync();
    action(GET_VI);
    vi = port->read() << 16ul;
    vi += port->readNoWait() << 8ul;
    vi += port->readNoWait();
    return vi;
}

uint8_t SpeedControl::getMode() {
    sync();
    action(GET_MODE);
    int mode = 0;
    mode = port->read();
    return mode;
}

uint8_t SpeedControl::getVersion() {
    sync();
    action(GET_VERSION);
    int version = 0;
    version = port->read();
    return version;
}

uint8_t SpeedControl::getError() {
    sync();
    action(GET_ERROR);
    uint8_t error = 0;
    error = port->read();
    return error;
}

void SpeedControl::enableReg(bool b) {
    sync();
    if(b) {
        action(REG_ENABLE);
    } else {
        action(REG_DISABLE);
    }
}

void SpeedControl::enableTimeout(bool b) {
    sync();
    if(b) {
        action(TIMEOUT_ENABLE);
    } else {
        action(TIMEOUT_DISABLE);
    }
}

void SpeedControl::flush() {
    port->printAll();
}

void SpeedControl::sync() {
    action(CLEAR);
}

void SpeedControl::action(uint8_t a) {
    port->write(a);
}
