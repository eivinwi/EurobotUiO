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
void Position::set(int n_x, int n_y, int n_rotation) {	
	x = static_cast<float>( n_x );
	y = static_cast<float>( n_y );
	if(n_rotation > 0 || n_rotation <= 360) {
		rotation->set(n_rotation);
	}
}

void Position::updateEncoder() {


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

std::string Position::getPosString() {
	std::stringstream ss;
	ss << (int) floor(x) << ",";
	ss << (int) floor(y) << ",";
	ss << (int) floor(getRotation());
	return ss.str();
}

void Position::print() {
	PRINTLINE("x:" << x << " y:" << y << " r:" << getRotation());
}

void Position::updateAngle(float leftDiff, float rightDiff) {
	rotation->updateAngle(leftDiff, rightDiff);
}

void Position::incrX(float dist) {
	x += dist*POS_DIR;
}

void Position::decrX(float dist) {
	x -= dist*POS_DIR;
}

void Position::incrY(float dist) {
	y += dist*POS_DIR;
}

void Position::decrY(float dist) {
	y -= dist*POS_DIR;
}