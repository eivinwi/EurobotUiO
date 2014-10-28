#include "Serial.h"


Serial::Serial() {
	serial.Open("/dev/ttyACM0");
//	serial.Open("/dev/ttyS0");	
//	serial.Open("/dev/ttyUSB0");

	serial.SetBaudRate( SerialStreamBuf::BAUD_38400 );
	serial.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 );
	serial.SetNumOfStopBits( SerialStreamBuf::DEFAULT_NO_OF_STOP_BITS);
	serial.SetParity( SerialStreamBuf::PARITY_NONE );
	serial.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_HARD );
}

Serial::~Serial() {
	serial.Close();
}

void Serial::write(uint8_t arg) {
	serial << arg;// << std::endl;
}

void Serial::write(char arg) {
	serial << arg;// << std::endl;
}

void Serial::write(int arg) {
	serial << arg << std::endl;
}

uint8_t Serial::readNoWait(){
	std::cout << "Reading" << '\n';
	uint8_t r = '\0';
	
	if(checkAvailable()) {
		r =  serial.get();
	} else {
		std::cout << "In ELSE, sleeping for 50ms\n";
		usleep(5000);
		if(checkAvailable()) {
			r =  serial.get();
		}
	}
	
	return r;		
}

uint8_t Serial::read() {
	usleep(5000);
	return readNoWait();
}

int Serial::readInt() {
	int i;
	serial >> i;
	return i;
}

void Serial::readBlock(int byte, char *buf) {
	serial.read(buf, byte);
}

bool Serial::checkAvailable() {
	return serial.rdbuf()->in_avail();
}

void Serial::printAll() {
	std::cout << "Flushed serial: [";
	while(serial.rdbuf()->in_avail()) {
		std::cout << serial.get();
	}
	std::cout << "]\n";
} 	
