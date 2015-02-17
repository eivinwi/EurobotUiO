/*
 *	File: goalposition.cpp
 *	Author: Eivind Wikheim
 *
 *	GoalPosition contains the current (exact) goal position, and functions for 
 *	calculating/updating based on it. Each task has an unique task-ID (int id).   
 *
 *	Copyright (c) 2015 Eivind Wikheim <eivinwi@ifi.uio.no>. All Rights Reserved.
 *
 *	This file is part of EurobotUiO.
 *
 *	EurobotUiO is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 * 	(at your option) any later version.
 *
 *	EurobotUiO is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with EurobotUiO.  If not, see <http://www.gnu.org/licenses/>.
 */

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