#include "goalposition.h"

GoalPosition::GoalPosition() {
	reset();
}

GoalPosition::GoalPosition(int n_id, int n_x, int n_y, int r) {
	id = n_id;
	x = n_x; 
	y = n_y;
	rotation = r;
}


GoalPosition::~GoalPosition() {
}


void GoalPosition::reset() {
	id = 0;
	x = 0;
	y = 0;
	rotation = 0.0;
	changed = time(0);
}


void GoalPosition::setAngle(float angle) {
	if(angle > 0.0 && angle < 360.0) {
		rotation = angle;
	} else if(angle == 360) {
		rotation = 0.0;
	}
}

//should perhaps check for reasonable values
void GoalPosition::setPosition(int n_x, int n_y){
	x = n_x;
	y = n_y;
}

//should not be used!
void GoalPosition::set(int n_x, int n_y, float n_rotation) {	
	PRINTLINE("[GOALPOS] Warning; using deprecated function set(x,y,r)");
	setPosition(n_x, n_y);
	setAngle(n_rotation);
}


double GoalPosition::timeSinceUpdate() {
	return (time(0) - changed);
}


float GoalPosition::getRotation() {
	return static_cast<float>(rotation);
}


int GoalPosition::getId() {
	return id;
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