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
    void write(char arg);
    void write(int arg);
    char read();
    int readInt();
    void readBlocks(int byte, char *buf);
    
private:
    SerialStream serial;

	char buffer[BUFFER_SIZE];    
    
    
};

#endif	/* SERIAL_H */

