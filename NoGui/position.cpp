#include "position.h"

Position::Position() {
	reset();
}


Position::~Position() {
}


void Position::reset() {
	x = 0.0;
	y = 0.0;
	rotation = new Rotation;
	changed = time(0);
}


//should perhaps check x/y, but checking is also performed in PosControl
void Position::set(float n_x, float n_y, float n_rotation) {	
	pos_mutex.lock();
	x = n_x;
	y = n_y;
	setAngle(n_rotation);
	pos_mutex.unlock();
}


void Position::setAngle(float angle) {
	if(angle > 0 && angle <= 360) {
		rotation->set(angle);
	}	
}


float Position::distanceX(float to) {
	return (x - to);
}


float Position::distanceY(float to) {
	return (y - to);
}


float Position::distanceRot(float to) {
	return rotation->distanceTo(to);
}


double Position::timeSinceUpdate() {
	return (time(0) - changed);
}


float Position::getRotation() {
	return rotation->get();
}


float Position::getX() {
	return x;
}


float Position::getY() {
	return y;
}


//used externally
std::string Position::getPosString() {
	pos_mutex.lock();
	std::string pos = pos_string;
	pos_mutex.unlock();
	return pos;
}


void Position::print() {
	PRINTLINE("x:" << x << " y:" << y << " r:" << getRotation());
}


void Position::updateAngle(float leftDiff, float rightDiff) {
	rotation->updateAngle(leftDiff, rightDiff);
}


void Position::updateX(float dist) {
	x += dist;//*POS_DIR;
}


void Position::updateY(float dist) {
	y += dist;//*POS_DIR;
}


void Position::updatePosString() {
	if(pos_mutex.try_lock()) {
		//update string
		//PRINTLINE("UPDATING POS");
		std::stringstream ss;
		ss << (int) floor(x) << ",";
		ss << (int) floor(y) << ",";
		ss << (int) floor(getRotation());
		pos_string = ss.str();
		pos_mutex.unlock(); 
	}
}