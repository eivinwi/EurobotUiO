#include "Serial.h"


Serial::Serial() {
	//serial.Open("/dev/ttyS0");
	serial.Open("/dev/ttyACM0");
	serial.SetBaudRate(SerialStreamBuf::BAUD_57600);
	serial.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 );
	//serial.SetNumOfStopBits(1);
	//serial.SetParity( SerialStreamBuf::PARITY_ODD );
	//serial.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_HARD );
}

Serial::~Serial() {
	serial.Close();
}


void Serial::write(char arg) {
	serial << arg;
}

void Serial::write(int arg) {
	serial << arg;	
}

char Serial::read(){
	char r;
	serial >> r;
	return r;		
}

int Serial::readInt() {
	int i;
	serial >> i;
	return i;
}


void Serial::readBlocks(int byte, char *buf) {
	serial.read(buf, byte);
} 	