#include "Serial.h"


Serial::Serial() {
	serial.Open("/dev/ttyACM2");
//	serial.Open("/dev/ttyS0");	
//	serial.Open("/dev/ttyUSB0");
//	serial.SetBaudRate(SerialStreamBuf::BAUD_115200);
	serial.SetBaudRate(SerialStreamBuf::BAUD_38400);
	//serial.SetBaudRate(SerialStreamBuf::BAUD_9600);
	serial.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 );
	serial.SetNumOfStopBits( SerialStreamBuf::DEFAULT_NO_OF_STOP_BITS);
	serial.SetParity( SerialStreamBuf::PARITY_NONE );
	serial.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_HARD );

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

uint8_t Serial::readNoWait(){
	std::cout << "reading" << '\n';
	uint8_t r = '\0';
	//serial >> r;
	
	if(serial.rdbuf()->in_avail()) {
		r =  serial.get();
	} else {
		usleep(5000);
		if(serial.rdbuf()->in_avail()) {
			r =  serial.get();
		}
	}

//	serial.get(r);
	//serial.read(r);
	std::cout << "done" << '\n';
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

void Serial::printAll() {
	std::cout << "All serial: [";
	while(serial.rdbuf()->in_avail()) {
		std::cout << serial.get();
	}
	std::cout << "]\n";
} 	
