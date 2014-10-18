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
	uint8_t sdir = 0x31;
	port->write(sdir);
	port->write(speed);
}

void SpeedControl::setSpeed2(uint8_t speed) {
	right->speed = speed;
	sync();
	port->write(0x32);
	port->write(speed);
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

void SpeedControl::resetEncoders() {
	sync();
	port->write(0x35);
}

void SpeedControl::getEncoders() {
	sync();
	uint8_t x = 0x25;
	port->write(x);
	long result;
	result = port->read() << 24ul;
	result += port->read() << 16ul;
	result += port->read() << 8ul;
	result += port->read(); 
	//port->readBlock(ENC_BUFF_SIZE, enc_buffer);

	//TODO: Do stuff with enc_buffer
	std::cout << result << '\n';
}

uint8_t SpeedControl::getVoltage() {
	sync();
	port->write(0x26);
	usleep(5000);
	uint8_t volt = port->read();
	std::cout << "Voltage: " << volt << '\n';
	return volt;
}

void SpeedControl::sync() {
	uint8_t clear_b = 0x00;
	port->write(clear_b);
}