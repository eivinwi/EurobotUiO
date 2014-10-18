#include "SpeedControl.h"

SpeedControl::SpeedControl() {
	port = new Serial;
//	left = new Motor(LEFT, 0, 0);
	left = new Motor;
	left->id = LEFT;
	left->speed = 0;
	right = new Motor;
	right->id = RIGHT;
	right->speed = 0;
	

}

SpeedControl::~SpeedControl() {
	//free(&left);
	//free(&right);

}


void SpeedControl::setSpeed1(uint8_t speed) {
	left->speed = speed;
	
	sync();
	port->write(0x31);
	port->write(speed);
}

void SpeedControl::setSpeed2(uint8_t speed) {
	right->speed = speed;

	sync();
	port->write(0x31);
	port->write(0x200);
}

uint8_t SpeedControl::getSpeed1() {
	sync();
	port->write(0x21);
	uint8_t speed = port->read();
	left->speed = speed;
	return speed;
}

uint8_t SpeedControl::getSpeed2() {
	sync();
	port->write(0x21);
	uint8_t speed = port->read();
	right->speed = speed;
	return speed;
}

void SpeedControl::getEncoders() {
	sync();
	port->write(0x25);

}

void SpeedControl::sync() {
	port->write(0x00);
}