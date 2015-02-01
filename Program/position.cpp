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
	rotation = new Rotation;
//	changed = time(0);
}


//should perhaps check x/y, but checking is also performed in PosControl
void Position::set(double n_x, double n_y, double n_rotation) {	
	pos_mutex.lock();
	x = n_x;
	y = n_y;
	setAngle(n_rotation);
	pos_mutex.unlock();
}


void Position::setAngle(double angle) {
	if(angle > 0 && angle <= 360) {
		rotation->set(angle);
	}	
}


double Position::getAngle() {
	return rotation->get();
}


double Position::getX() {
	return x;
}


double Position::getY() {
	return y;
}


double Position::distanceX(double to) {
	return (x - to);
}


double Position::distanceY(double to) {
	return (y - to);
}


double Position::distanceRot(double to) {
	return rotation->distanceTo(to);
}


//Deprecated
//double Position::timeSinceUpdate() {
//	return (time(0) - changed);
//}


void Position::print() {
	PRINTLINE("x:" << x << " y:" << y << " r:" << getAngle());
}


void Position::updateAngle(double leftDiff, double rightDiff) {
	rotation->updateAngle(leftDiff, rightDiff);
}


void Position::updateX(double dist) {
	x += dist;//*POS_DIR;
}


void Position::updateY(double dist) {
	y += dist;//*POS_DIR;
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
		ss << (int) floor(getAngle());
		pos_string = ss.str();
		pos_mutex.unlock(); 
	}
}