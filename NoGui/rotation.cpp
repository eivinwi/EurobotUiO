#include "rotation.h"

//TODO: something is not right with direction-select
// sometimes choses the furthest path to rotate

Rotation::Rotation() {
	reset();
}

Rotation::~Rotation(){
}

void Rotation::reset() {
	angle = 0.0;
}


void Rotation::rotateTowards(float goalRot) {
	if(goalRot > 360.0) {
		DBPL("ROTATION: invalid goalRot: " << goalRot);
	} else if (goalRot == angle) {
		DBPL("ROTATION: already at goalRot: " << goalRot);
	} else {
		float rot = distanceTo(goalRot);

		if(rot > 0) {
			DBPL("ROTATION: rotating positive direction (" << angle << "->" << goalRot << ")");
		}
		else {
			DBPL("ROTATION: rotating positive direction (" << angle << "->" << goalRot << ")");    			
		}
	}

}


//BUG: turned is always positive
//calculate new angle based on encoders
void Rotation::updateAngle(long diffL, long diffR) {
	int ediff = (abs(diffL) + abs(diffR))/2;
	if(ediff > 50) {
		DBPL("POS: Warning, large encoder difference (turning): " << ediff);
	}


  	//TODO: needs to know that encoders read the same
	long encAvg = (abs(diffL) + abs(diffR))/2;
	float turned = 0.0;

	if(diffL > 0) {
		turned = (encAvg/ENC_PER_DEGREE) * POS_DIR;//*ROTATION_DIR; 
	} else {
		turned = -(encAvg/ENC_PER_DEGREE) * POS_DIR;//*ROTATION_DIR;
	}

	DBP("POS: changing rotation: " << angle << " + " << turned);

	angle += turned;
	if(angle > 360.0) {
		DBP("\nPOS: overflow " << angle);
		angle -= 360.0;
	} else if(angle < 0.0) {
		DBP("\nPOS: overflow " << angle);
		angle = 360.0 + angle;
	}

	DBPL(" = " << angle);
}


void Rotation::set(float a) {
	if(a < 0.0 || a > 360.0) {
		DBPL("INVALID input to setAngle(): " << a);
	} else {
    	angle = a;
	}
}


float Rotation::distanceTo(float goalRot) {
	float l = distanceLeft(goalRot);
	float r = distanceRight(goalRot);
	DBPL("ROTATION distanceTo: l=" << l << " r=" << r);
	if(abs(l) <= abs(r)) {
		return l;
	} else {
		return r;
	}
}

float Rotation::distanceLeft(float goal) {
	if(goal >= angle) {
		return (goal - angle);
	} else {
		return ((360 - angle) + goal);
	}
}

//result will be negative, to show rotation towards the right
float Rotation::distanceRight(float goal) {
	if(goal >= angle) {
		return ( (-angle) - (360-goal) );  //(goal - 360 - angle);
	} else {
		return (goal - angle);
	}
}


float Rotation::get() {
	return angle;
}