#ifndef SERIAL_H
#define	SERIAL_H

#define BUFFER_SIZE 4

#include <SerialStream.h>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <stdint.h>

using namespace LibSerial ;

class Serial {
public:
    Serial();
    ~Serial();
    void testProg();
    void close();
    void write(int);
    void write(char);
    void writeBlock(char[]); 
    void writeSpeeds(uint16_t l, uint16_t r); 
    char readChar();
   	int readInt();
   	void readBlock(char[]);
private:
    SerialStream serial;

	char buffer[BUFFER_SIZE];    
    
    
};

#endif	/* SERIAL_H */

