#include "goalposition.h"

GoalPosition::GoalPosition() {
	reset();
}

GoalPosition::GoalPosition(int n_id, float n_x, float n_y, float r) {
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
}

void GoalPosition::setId(int i) {
	id = i;
}


void GoalPosition::setAngle(float angle) {
	if(angle > 0.0 && angle < 360.0) {
		rotation = angle;
	} else if(angle == 360) {
		rotation = 0.0;
	}
}

//should perhaps check for reasonable values
void GoalPosition::setPosition(float n_x, float n_y){
	x = n_x;
	y = n_y;
}

//should not be used!
void GoalPosition::set(float n_x, float n_y, float n_rotation) {	
	PRINTLINE("[GOALPOS] Warning; using deprecated function set(x,y,r)");
	setPosition(n_x, n_y);
	setAngle(n_rotation);
}


int GoalPosition::getId() {
	return id;
}


float GoalPosition::getX() {
	return x;
}


float GoalPosition::getAngle() {
	return rotation;
}


float GoalPosition::getY() {
	return y;
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