#ifndef SPEEDCONTROL_H
#define	SPEEDCONTROL_H

#define LEFT 1
#define RIGHT 2
#define ENC_BUFF_SIZE 8


#include "Serial.h"
#include <stdint.h>

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
	void setSpeed1(uint8_t speed);
	void setSpeed2(uint8_t speed);
	uint8_t getSpeed1();
	uint8_t getSpeed2();
	void resetEncoders();
	void getEnc1();
	void getEnc2();
	void getEncoders();
	uint8_t getVoltage();
	uint8_t getVersion();
	void setMode(int mode);
	void printSerial();
	void sync();
private:
	Serial *port;
	Motor *right; 
	Motor *left;

	char enc_buffer[ENC_BUFF_SIZE];
};

#endif /* SPEEDCONTROL_H */