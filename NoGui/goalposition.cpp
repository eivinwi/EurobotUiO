#include "goalposition.h"

GoalPosition::GoalPosition() {
	reset();
}


GoalPosition::~GoalPosition() {
}


void GoalPosition::reset() {
	x = 0;
	y = 0;
	rotation = 0;
	changed = time(0);
}


//should perhaps check x/y, but checking is also performed in PosControl
void GoalPosition::set(int n_x, int n_y, float n_rotation) {	
	x = n_x;
	y = n_y;
	setAngle(n_rotation);
}


void GoalPosition::setAngle(float angle) {
	if(angle > 0.0 || angle < 360.0) {
		rotation = angle;
	} else if(angle == 360) {
		rotation = 0.0;
	}
}


double GoalPosition::timeSinceUpdate() {
	return (time(0) - changed);
}


float GoalPosition::getRotation() {
	return static_cast<float>(rotation);
}


float GoalPosition::getX() {
	return static_cast<float>(x);
}


float GoalPosition::getY() {
	return static_cast<float>(y);
}


std::string GoalPosition::getPosString() {
	std::stringstream ss;
	ss << x << ",";
	ss << y << ",";
	ss << rotation;
	return ss.str();
}


void GoalPosition::print() {
	PRINTLINE("x:" << x << " y:" << y << " r:" << rotation);
}