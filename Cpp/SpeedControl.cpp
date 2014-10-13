#include "SpeedControl.h"

SpeedControl::SpeedControl() {
	port = new Serial;
//	left = new Motor(LEFT, 0, 0);
	left = new Motor;
	left->id = LEFT;
	left->speed = 0;
	right = new Motor;
	right->id = RIGHT;
	right->speed = 0;
	
	setSpeed(LEFT, 111);
	setSpeed(RIGHT, 111);
	updateSpeeds();
}

SpeedControl::~SpeedControl() {
	//free(&left);
	//free(&right);
	port->close();
}


void SpeedControl::setSpeed(uint8_t side, uint16_t speed) {
	if(side == LEFT) {
		left->speed = speed;
	} else {
		right->speed = speed;
	}
}

uint16_t SpeedControl::getSpeed(uint8_t side) {
	if(side == LEFT) {
		return left->speed;
	} else {
		return right->speed;
	}
}

void SpeedControl::updateSpeeds() {
	port->writeSpeeds(left->speed, right->speed);

}