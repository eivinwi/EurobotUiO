#include "serial.h"
#include <cstring>


Serial::Serial(char *serial_port) {
	char s[20]; 
	strcpy(s, "/dev/");
	strcat(s, serial_port);
    serial.Open(s);

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
	DBPL("SERIAL: writeu: " << arg);
    serial << arg;
}

void Serial::write(char arg) {
	DBPL("SERIAL: writec: " << arg);
    serial << arg;
}

void Serial::write(int arg) {
	DBPL("SERIAL: writeint: " << arg << " (converting)");
    write((uint8_t) arg); //hack!!!!
}

uint8_t Serial::readNoWait(){
    DBPL("SERIAL: readNW");
    uint8_t r = 0x00;

    if(available()) {
        r =  serial.get();
    } else {
        DBPL("In ELSE, sleeping for 50ms");
        usleep(5000);
        if(available()) {
            r =  serial.get();
        }
    }
    DBPL("SERIAL: readNW returning " << r);
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
    PRINT("Flushed serial: [");
    while(serial.rdbuf()->in_avail()) {
        PRINT(serial.get());
    }
    PRINTLINE("]");
}
