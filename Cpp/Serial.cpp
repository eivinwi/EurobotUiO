#include "Serial.h"


Serial::Serial() {
	//serial.Open("/dev/ttyS0");
	serial.Open("/dev/ttyACM0");
	serial.SetBaudRate(SerialStreamBuf::BAUD_9600);
	serial.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 );
	//serial.SetNumOfStopBits(1);
	//serial.SetParity( SerialStreamBuf::PARITY_ODD );
	//serial.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_HARD );
}

Serial::~Serial() {
	close();
}

void Serial::testProg() {
	// Read
	for(int i = 0; i < 100; i++) {
		//readBlock(buffer);
		//std::cout << buffer;// << '\n';
		std::cout << readChar();
		usleep(100000); //us
		//std::this_thread::sleep_for( std::chrono::milliseconds(100));
	}

	//Write
/*	for(int i = 0; i < 100; i++) {
		//readBlock(buffer);
		//std::cout << buffer;// << '\n';
		write('U');
		usleep(100000); //us
		//std::this_thread::sleep_for( std::chrono::milliseconds(100));
	}*/
}

char Serial::readChar() {
	char next_char;
	serial.get(next_char);
	return next_char;
}

int Serial::readInt() {
	int next_int;
	serial >> next_int; 
	return next_int;
}

void Serial::readBlock(char* buf) {
	std::cout << "readBlock" << '\n';

	serial.read(buf, BUFFER_SIZE);
}

void Serial::write(char c) {
	serial << c;
}

void Serial::write(int i) {
	serial << i;
}

void Serial::writeBlock(char* buf) {
	for(int i = 0; i < BUFFER_SIZE; i++) {
		serial << buf[i];
	}
}



/* change one:
 * |motor|dir|pwm2|pwm1|pwm0| = 5byte. Motor is 0=l 1=r
 * 
 * change both:
 * |Ldir|Rdir|Lpwm2|Lpwm1|Lpwm0|Rpwm2|Rpwm1|Rpwm0| = 8byte
*/
void Serial::writeSpeeds(uint8_t dir_l, uint8_t pwm_l, uint8_t dir_r, uint8_t pwm_r) {
	/*uint8_t speeds[4];
	speeds[0] = (uint8_t) ((l & 0xFF00) >> 8);
	speeds[1] = (uint8_t) (l & 0x00FF);
	speeds[2] = (uint8_t) ((r & 0xFF00) >> 8);
	speeds[3] = (uint8_t) (r & 0x00FF);
	std::cout << speeds[0] << speeds[1] << speeds[2] << speeds[3] << '\n';
	std::cout << speeds << '\n';

	serial << "set";
	for(int i = 0; i < 4; i++) {
		serial << speeds[i];
	}*/
	serial << "set"
	serial << l;
	serial << r;
	serial << '\n';
}


void Serial::close() {
	serial.Close();
}