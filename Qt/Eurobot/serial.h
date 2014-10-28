#ifndef SERIAL_H
#define	SERIAL_H

#ifdef DEBUG
#define DBP(x) DBP( x;
#else
#define DBP(x);
#endif

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
    void write(uint8_t arg);
    void write(char arg);
    void write(int arg);
    uint8_t readNoWait();
    uint8_t read();
    int readInt();
    void readBlock(int byte, char *buf);
    void printAll();


private:
    SerialStream serial;

//    SerialPort serial;

};

#endif	/* SERIAL_H */

