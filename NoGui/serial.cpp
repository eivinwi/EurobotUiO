#include "serial.h"
#include <cstring>


Serial::Serial(char *serial_port) {
	char s[20]; 
//	strcpy(s, "/dev/");
//	strcat(s, serial_port);
//    serial.Open(s);

    serial.Open("/dev/ttyACM0");
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
	DBP("WRITEu: " << arg << std::endl);
    serial << arg;// << std::endl;
}

void Serial::write(char arg) {
	DBP("WRITEchar: " << arg << std::endl);
    serial << arg;// << std::endl;
}

void Serial::write(int arg) {
	DBP("WRITEint: " << arg << std::endl);
    //serial << arg;// << std::endl;
    write((uint8_t) arg);
}

uint8_t Serial::readNoWait(){
    DBP("Reading" << '\n');
    uint8_t r = 0x00;

    if(available()) {
        r =  serial.get();
    } else {
        DBP("In ELSE, sleeping for 50ms\n");
        usleep(5000);
        if(available()) {
            r =  serial.get();
        }
    }
    DBP("Read returning " << r << '\n');
    return r;
}

uint8_t Serial::read() {
    usleep(5000);
    return readNoWait();
}

bool Serial::available() {
	return serial.rdbuf()->in_avail();
}

void Serial::printAll() {
    std::cout << "Flushed serial: [";
    while(serial.rdbuf()->in_avail()) {
        std::cout << serial.get();
    }
    std::cout << "]" << std::endl;
}
