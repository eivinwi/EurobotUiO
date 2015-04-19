/*
 *  File: position.cpp
 *  Author: Eivind Wikheim
 *
 *	Position contains the current estimated robot position, and functions for using/updating it.
 *	Is used by PosControl for calculating path, and is updated continuosly.
 *
 *  Copyright (c) 2015 Eivind Wikheim <eivinwi@ifi.uio.no>. All Rights Reserved.
 *
 *  This file is part of EurobotUiO.
 *
 *  EurobotUiO is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EurobotUiO is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EurobotUiO.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "position.h"


Position::Position() {
	reset();
}


Position::~Position() {
}


void Position::reset() {
	x = 0.0;
	y = 0.0;
	angle = 0.0;
}


//mutex locked because it can be set from outside the poscontrol-thread
void Position::set(float n_x, float n_y, float a) {	
	pos_mutex.lock();
	x = n_x;
	y = n_y;
	angle = a;
	setAngle(a);
	pos_mutex.unlock();
}


void Position::setAngle(float a) {
	if(a < 360) {
		angle = a;
	} else if(a == 360) {
		angle = 0;
	} else {
			PRINTLINE("[POS] warning, invalid angle: " << a);
	}
}


float Position::getX() {
	return x;
}


float Position::getY() {
	return y;
}


float Position::getAngle() {
	return angle;
}


float Position::distanceX(float to) {
	return (x - to);
}


float Position::distanceY(float to) {
	return (y - to);
}


float Position::distanceRot(float to) {
	return shortestRotation(to);
}


void Position::print() {
	PRINTLINE("x:" << x << " y:" << y << " r:" << angle);
}


void Position::updateX(float dist) {
	x += dist;
}


void Position::updateY(float dist) {
	y += dist;
}


std::string Position::getPosString() {
	pos_mutex.lock();
	std::string pos = pos_string;
	pos_mutex.unlock();
	return pos;
}


void Position::updatePosString() {
	if(pos_mutex.try_lock()) {
		std::stringstream ss;
		ss << (int) floor(x) << ",";
		ss << (int) floor(y) << ",";
		ss << (int) floor(angle);
		pos_string = ss.str();
		pos_mutex.unlock(); 
	}
}


// ROTATION stuff

float Position::shortestRotation(float goal) {
	float dist_left = (goal >= angle)? (goal - angle) : ((360 - angle) + goal);
	float dist_right = (goal >= angle)? -(angle + (360-goal)) : (goal - angle); 
	return (abs(dist_left) < abs(dist_right))? dist_left : dist_right;
}


void Position::updateAngle(long diffL, long diffR) {
  	// Average of encoders for reduced maximum-error
	long encAvg = (abs(diffL) + abs(diffR))/2;
	float turned = 0.0;

	turned = (diffL > 0)? -(encAvg/ENC_PER_DEGREE) : (encAvg/ENC_PER_DEGREE);

	angle += turned;
	if(angle > 360.0) {
		angle -= 360.0;
	} else if(angle < 0.0) {
		angle += 360.0;
	}
}


