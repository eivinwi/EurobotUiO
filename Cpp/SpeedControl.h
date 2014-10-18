#ifndef SPEEDCONTROL_H
#define	SPEEDCONTROL_H

#define LEFT 1
#define RIGHT 2

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
	void getEncoders();
	void sync();
private:
	Serial *port;
	Motor *right; 
	Motor *left;
};

#endif /* SPEEDCONTROL_H */