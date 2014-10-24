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

    prev_enc1 = 0;
    prev_enc2 = 0;
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
    return port->read();
}

//input 0 [default], 1, 2 or 3
void SpeedControl::setMode(int mode) {
    sync();
    action(SET_MODE);
    port->write(mode);
}


void SpeedControl::resetEncoders() {
    sync();
    action(RESET_ENC);
    prev_enc1 = 0;
    prev_enc2 = 0;
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

    long result1;
    long result2;
    result1 = port->read() << 24ul;
    result1 += port->readNoWait() << 16ul;
    result1 += port->readNoWait() << 8ul;
    result1 += port->readNoWait();

    result2 = port->read() << 24ul;
    result2 += port->readNoWait() << 16ul;
    result2 += port->readNoWait() << 8ul;
    result2 += port->readNoWait();
    std::cout << "Enc1: " << result1 << '\n';
    std::cout << "Enc2: " << result2 << '\n';
}




long SpeedControl::getEnc1() {
    sync();
    action(GET_ENC1);
    long result = 0;
    long diff = 0;
    result = port->read() << 24ul;
    result += port->readNoWait() << 16ul;
    result += port->readNoWait() << 8ul;
    result += port->readNoWait();

    std::cout << "Enc1: " << result;
    diff = result - prev_enc1;
    std::cout << " (diff: " << diff << ")\n";

    std::cout << "Wheel rotations: " <<  (diff/980.0) << '\n';
    std::cout << "Distance: " << diff*0.385 << '\n';

    prev_enc1 = result;
    return result;
}

long SpeedControl::getEnc2() {
    sync();
    action(GET_ENC2);
    long result = 0;
    result = port->read() << 24ul;
    result += port->readNoWait() << 16ul;
    result += port->readNoWait() << 8ul;
    result += port->readNoWait();
    return result;
}

uint8_t SpeedControl::getVoltage() {
    sync();
    action(GET_VOLT);
    int volt = 0;
    return port->read();
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

uint8_t SpeedControl::getVersion() {
    sync();
    action(GET_VERSION);
    return port->read();
}

uint8_t SpeedControl::getError() {
    sync();
    action(GET_VERSION);
    return port->read();
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
