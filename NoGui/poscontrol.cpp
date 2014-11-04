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
		usleep(50000);

		if(inGoal()) {
			break;
		}

	}
	com->setSpeedBoth(SPEED_STOP);
	usleep(100000);
	std::cout << "Stopped" << std::endl;
	updatePosition();
}


bool Poscontrol::inGoal() {
	return (distanceFromX() < 5);
}


void Poscontrol::drive() {

	if(!inGoal()) {
		com->setSpeedBoth(150);
	//} else if(distanceFromY > 5) {
	//	setSpeedBoth(100);
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
	com->flush();
	long encL = com->getEncL();
	//long encR = com->getEncR(); 


	long diffL = encL - prev_encL;
	//long diffR = encR - prev_encR;

	float distanceL = diffL*0.385;
	//long distanceR = diffR*0.385;

	std::cout << "\nUPDATEPOSITION" << std::endl;
	std::cout << "prev_encL: " << prev_encL << std::endl; 
	std::cout << "encL: " << encL << std::endl; 
	std::cout << "diffL: " << diffL << std::endl; 
	std::cout << "distanceL: " << distanceL << std::endl; 
	std::cout << "totalDist: " << encL*0.385 << std::endl;
	std::cout << "--------------" << std::endl;


	currentX += distanceL;
	//currentY += distanceR;
	prev_encL = encL;
	//prev_encR = encR;
}


long totalDist = 0;











