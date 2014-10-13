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
	void updateSpeeds();
	void setSpeed(uint8_t, uint8_t);
	uint8_t getSpeed(uint8_t);

private:
	Serial *port;
	Motor *right; 
	Motor *left;
};

#endif /* SPEEDCONTROL_H */