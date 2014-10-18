#include "Serial.h"


Serial::Serial() {
	//serial.Open("/dev/ttyS0");
	serial.Open("/dev/ttyUSB0");
	serial.SetBaudRate(SerialStreamBuf::BAUD_38400);
	serial.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 );
	//serial.SetNumOfStopBits(1);
	//serial.SetParity( SerialStreamBuf::PARITY_ODD );
	//serial.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_HARD );
}

Serial::~Serial() {
	serial.Close();
}

void Serial::write(uint8_t arg) {
	std::cout << "Writing uint8_t" << '\n';
	serial << arg;// << std::endl;
}

void Serial::write(char arg) {
	std::cout << "Writing char" << '\n';
	serial << arg;// << std::endl;
}

void Serial::write(int arg) {
	std::cout << "Writing int" << '\n';
	serial << arg << std::endl;
}

uint8_t Serial::read(){
	std::cout << "reading" << '\n';
	uint8_t r;
	serial >> r;
	std::cout << "done" << '\n';
	return r;		
}

int Serial::readInt() {
	int i;
	serial >> i;
	return i;
}


void Serial::readBlock(int byte, char *buf) {
	serial.read(buf, byte);
} 	