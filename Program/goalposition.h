/*
 *	File: goalposition.h
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


// IMPORTANT:
// - Is EITHER a rotational or a positional change. 
// 		- Can only be both rotational and positional when manipulated internally by PosControl

// TODO:
// 	  - atomic?
//    - remove set functions
 

#ifndef GOALPOSITION_H
#define GOALPOSITION_H

#include "rotation.h"
#include "md49.h"
#include "printing.h"

#include <atomic>
#include <math.h>
#include <string>
#include <sstream>
#include <unistd.h>

class GoalPosition {
public:
	//Only the constructor with arguments should probably be used.
	GoalPosition(int id, float x, float y, float r);
	GoalPosition();
	~GoalPosition();
	void reset();

	//set functions should probably not be used, as goal should be created using constructors
	void setId(int id);
	void setAngle(float angle);
	void setPosition(float n_x, float n_y);
	void set(float n_x, float n_y, float n_rotation);

	int getId();
	float getX();
	float getY();
	float getAngle();

	// Returns goal position as a comma-delimited string
	std::string getPosString();

	// Prints goal position to terminal
	void print();

private:
	//no idea why this is atomic
	std::atomic<int> id;
	
	float x;
	float y;
	float rotation;
};

#endif /* GOALPOSITION_H */