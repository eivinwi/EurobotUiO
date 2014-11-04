#include "poscontrol.h"
#include "motorcom.h"

Poscontrol::Poscontrol(MotorCom *s) {
	com = s;
	currentX = 0;
	currentY = 0;
	currentR = 0;
	prev_encL = 0;
	prev_encR = 0;
}

Poscontrol::~Poscontrol() {
}


void Poscontrol::testDrive(int x, int y) {
	setGoalPos(x, y, 0);
	
	while(true) {
		drive();
		updatePosition();
		std::cout << "X: " << currentX << " goalX: " << goalX << std::endl;
		if(inGoal()) {
			break;
		}
	}
}

bool Poscontrol::inGoal() {
	return (distanceFromX() < 5);
}


void Poscontrol::drive() {
	if(!inGoal()) {
		com->setSpeedBoth(100);
	//} else if(distanceFromY > 5) {
	//	setSpeedBoth(100);
	} else {
		com->setSpeedBoth(SPEED_STOP);
	}
}

void Poscontrol::setGoalPos(int x, int y, int rot) {
	goalX = x*10;
	goalY = y*10;
	goalR = rot;
}

int Poscontrol::distanceFromX() {
	return (goalX - currentX);
}

int Poscontrol::distanceFromY() {
	return (goalY - currentY);	
}

/*
 * Encoder counts: 980 per output shaft turn
 * Wheel diameter: 120mm
 * Wheel circumference: 377mm
 * Distance per count: 0.385mm
 */
void Poscontrol::updatePosition() {
	long encL = com->getEncL();
	long encR = com->getEncR(); 


	long diffL = prev_encL - encL;
	long diffR = prev_encR - encR;

	long distanceL = diffL*0.385;
	long distanceR = diffR*0.385;


	currentX += distanceL;
	currentY += distanceR;
	prev_encL = encL;
	prev_encR = encR;
}














