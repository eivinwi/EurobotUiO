/*
 *  File: position.h
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

#ifndef POSITION_H
#define POSITION_H

#include "md49.h"
#include "printing.h"
#include "rotation.h"
#include <mutex>
#include <string>
#include <sstream>
#include <time.h>


class Position {
public:
	Position();
	~Position();
	void reset();

	// Functions to set/get position variables
	void set(float n_x, float n_y, float n_rotation);
	void setAngle(float angle);
	float getAngle();
	float getX();
	float getY();

	// Functions calculating distances from "this" position to input goal-position "to"
	float distanceX(float to);
	float distanceY(float to);
	float distanceRot(float to);

	// Calls on updateAngle in Rotation to calculate curren angle based on encoder readings
	void updateAngle(float leftDiff, float rightDiff);
	
	// Adds positional updates to x
	void updateX(float dist);
	// Adds positional updates to y
	void updateY(float dist);

	void print();

	// Returns position as comma-delimited string. Thread-safe
	std::string getPosString();
	
	// Updates the comma-delimited position string. Thread-safe
	void updatePosString();

private:
	float x;
	float y;
	Rotation *rotation;
	time_t changed;

	std::string pos_string;
	std::mutex pos_mutex;
};

#endif /* POSITION_H */