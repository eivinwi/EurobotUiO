#include "SpeedControl.h
"
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
	port->write(0x22);
	uint8_t speed = port->read();
	right->speed = speed;
	return speed;
}


//input 0 [default], 1, 2 or 3
void SpeedControl::setMode(int mode) {
	sync();
	port->write(0x34);
	port->write(mode);
}

void SpeedControl::resetEncoders() {
	sync();
	port->write(0x35);
}


/*
 * Encoder counts: 980 per output shaft turn
 * Wheel diameter: 120mm
 * Wheel circumference: 377mm
 * Distance per count: 0.385mm
 */
void SpeedControl::getEncoders() {
	sync();
	uint8_t x = 0x25;
	port->write(x);
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

	//port->readBlock(ENC_BUFF_SIZE, enc_buffer);

	//TODO: Do stuff with enc_buffer
	std::cout << "Enc1: " << result1 << '\n';
	std::cout << "Enc2: " << result2 << '\n';
}  

void SpeedControl::getEnc1() {
	sync();
//	port->write
}

void SpeedControl::getEnc2() {

}

uint8_t SpeedControl::getVoltage() {
	sync();
	uint8_t x = 0x26;
	port->write(B);
	int volt = 0;
	volt = port->readNoWait();
	std::cout << "Voltage: " << volt << '\n';
	return volt;
}

uint8_t SpeedControl::getVersion() {
	sync();
	port->write(0x2C);	
	int version = 0;
	version = port->read();
	std::cout << "Version: " << version << '\n';
	return version;
}

void SpeedControl::printSerial() {
	port->printAll();
}

void SpeedControl::sync() {
	uint8_t clear_b = 0x00;
	port->write(clear_b);
}