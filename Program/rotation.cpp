#include "rotation.h"


Rotation::Rotation() {
	reset();
}


Rotation::~Rotation(){
}


void Rotation::reset() {
	angle = 0.0;
}


void Rotation::set(double a) {
	if(a > 0.0 && a < 360.0) {
    	angle = a;
	}
}


double Rotation::get() {
	return angle;
}


void Rotation::updateAngle(long diffL, long diffR) {
	int ediff = (abs(diffL) + abs(diffR))/2;
	if(ediff > 50) {
		DBPL("[ROT] Warning, large encoder difference (turning): " << ediff);
	}

  	//TODO: needs to know that encoders read the same
	long encAvg = (abs(diffL) + abs(diffR))/2;
	double turned = 0.0;

	if(diffL > 0) {
		turned = -(encAvg/ENC_PER_DEGREE);// * POS_DIR;//*ROTATION_DIR; 
	} else {
		turned = (encAvg/ENC_PER_DEGREE);// * POS_DIR;//*ROTATION_DIR;
	}

	DBP("[ROT] changing rotation: " << angle << " + " << turned);
	angle += turned;
	if(angle > 360.0) {
		angle -= 360.0;
	} else if(angle < 0.0) {
		angle += 360.0;
	}
	DBPL(" = " << angle);
}


double Rotation::distanceLeft(double goal) {
	if(goal >= angle) {
		return (goal - angle);
	} else {
		return ((360 - angle) + goal);
	}
}


//result will be negative, to show rotation towards the right
double Rotation::distanceRight(double goal) {
	if(goal >= angle) {
		return ( (-angle) - (360-goal) );  //(goal - 360 - angle);
	} else {
		return (goal - angle);
	}
}


double Rotation::distanceTo(double goalRot) {
	double l = distanceLeft(goalRot);
	double r = distanceRight(goalRot);
	DBPL("[ROT] distanceTo: l=" << l << " r=" << r);
	if(abs(l) < abs(r)) {
		return l;
	} else {
		return r;
	}
}