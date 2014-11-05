#include "poscontrol.h"
#include <iostream>
#include <string>
#include <sstream>
//using namespace std;

struct position {
	float x;
	float y;
	int rot;
} goalPos, curPos, exactPos;

struct encoder {
	long prev;
	long diff;
	long total;
} encL, encR;




Poscontrol::Poscontrol(MotorCom *s) {
	com = s;

	curPos.x = 0;
	curPos.y = 0;
	curPos.rot = 0;
	
	encL.prev = 0;
	encL.diff = 0;
	encL.total = 0;

	encR.prev = 0;
	encR.diff = 0;
	encR.total = 0;
}

Poscontrol::~Poscontrol() {
}


void Poscontrol::testDrive(int x, int y) {
	setGoalPos(x, y, 0);
	
	while(true) {
		drive();
		updatePosition();
		PRINTLINE("X: " << curPos.x << " goalPos.x: " << goalPos.x);
		usleep(50000);

		if(inGoal()) {
			break;
		}

	}
	com->setSpeedBoth(SPEED_STOP);
	usleep(100000);

	PRINTLINE("Stopped");
	updatePosition();
}


bool Poscontrol::inGoal() {
	return (distanceFromX() < 5);
}


void Poscontrol::drive() {

	if(!inGoal()) {
		com->setSpeedBoth(200);
	//} else if(distanceFromY > 5) {
	//	setSpeedBoth(100);
	} 
}

void Poscontrol::setGoalPos(int x, int y, int rot) {
	goalPos.x = x*10;
	goalPos.y = y*10;
	goalPos.rot = rot;
}

int Poscontrol::distanceFromX() {
	return (goalPos.x - curPos.x);
}

int Poscontrol::distanceFromY() {
	return (goalPos.y - curPos.y);	
}

/*
 * Encoder counts: 980 per output shaft turn
 * Wheel diameter: 120mm
 * Wheel circumference: 377mm
 * Distance per count: 0.385mm
 */
void Poscontrol::updatePosition() {
	com->flush();
	long encoderL = com->getEncL();
	//long encR = com->getEncR(); 


	long diffL = encoderL - encL.prev;
	//long diffR = encR - encR.prev;

	float distanceL = diffL*0.385;
	//long distanceR = diffR*0.385;

	PRINTLINE("\nUPDATEPOSITION");
	PRINTLINE("encL.prev: " << encL.prev); 
	PRINTLINE("encL: " << encoderL); 
	PRINTLINE("diffL: " << diffL); 
	PRINTLINE("distanceL: " << distanceL); 
	PRINTLINE("totalDist: " << encoderL*0.385);
	PRINTLINE("--------------");







	curPos.x += distanceL;
	//curPos.y += distanceR;
	encL.prev = encoderL;
	//encR.prev = encR;
}


long totalDist = 0;
/* CurrentPos - current estimated position based on dead reckoning with encoders. Is updated regularly to be Exactpos
 * ExactPos   - input position from SENS
 * GoalPos    - input from main or AI 
*/









