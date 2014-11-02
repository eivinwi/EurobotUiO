#ifndef SPEEDCONTROL_H
#define	SPEEDCONTROL_H

#include "serialsim.h"
#include "serial.h"

#define LEFT 1
#define RIGHT 2
#define ENC_BUFF_SIZE 8


#include <stdint.h>
#include <bitset>
#include <sstream>

/* Structs */
typedef struct Motor {
    uint8_t id;
    uint8_t dir;
    uint8_t speed;
} Motor;


class SpeedControl {
public:
    SpeedControl();
    ~SpeedControl();

    void startSerial();
	void serialSimEnable();
	void serialSimDisable();
	void setSerialPort(char *s);
    void setAcceleration(int acc);
    void setSpeedL(uint8_t speed);
    void setSpeedR(uint8_t speed);
    void setSpeedBoth(uint8_t speed);
    uint8_t getSpeedL();
    uint8_t getSpeedR();
    uint8_t getAcceleration();
    void resetEncoders();
    long getEncL();
    long getEncR();
    void getEncoders();
    uint8_t getVoltage();
    long getVi();
    uint8_t getMode();
    uint8_t getVersion();
    uint8_t getError();
    void setMode(uint8_t mode);
    void enableReg(bool b);
    void enableTimeout(bool b);
    void flush();
    void sync();
    void writeToSerial(uint8_t a);
    void writeToSerial(int a);
    //int readFromSerial();
    uint8_t readFromSerial();
    uint8_t readFromSerialNoWait();

private:
    SerialSim *simport;
    Serial *port;

    Motor *right;
    Motor *left;

    char enc_buffer[ENC_BUFF_SIZE];

    long prev_encL;
    long prev_encR;

    bool simulating;
	char *serial_port;
};

#endif /* SPEEDCONTROL_H */
