/*
 *  File: rotation.cpp
 *  Author: Eivind Wikheim
 *
 *  Rotation contains the current estimated robot rotation (angle), 
 *  and functions for calculating distances (in degrees).
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


#include "rotation.h"


Rotation::Rotation() {
	reset();
}


Rotation::~Rotation(){
}


void Rotation::reset() {
	angle = 0.0;
}


void Rotation::set(float a) {
	if(a > 0.0 && a < 360.0) {
    	angle = a;
	}
}


float Rotation::get() {
	return angle;
}


void Rotation::updateAngle(long diffL, long diffR) {
/*	
	int ediff = (abs(diffL) + abs(diffR))/2;
	if(ediff > 50) {
		DBPL("[ROT] Warning, large encoder difference (turning): " << ediff);
		return;
	}
*/
  	// Average of encoders for reduced maximum-error
	long encAvg = (abs(diffL) + abs(diffR))/2;
	float turned = 0.0;

	if(diffL > 0) {
		turned = -(encAvg/ENC_PER_DEGREE); 
	} else {
		turned = (encAvg/ENC_PER_DEGREE);
	}

	LOG(DEBUG) << "[ROT] changing rotation: " << angle << " + " << turned;
	angle += turned;
	if(angle > 360.0) {
		angle -= 360.0;
	} else if(angle < 0.0) {
		angle += 360.0;
	}
	LOG(DEBUG) << " = " << angle;
}




// Returns distance in the shortest rotation-direction. 
// returns <0 if clockwise, >0 if counter-clockwise
float Rotation::distanceTo(float goalRot) {
	float l = distanceLeft(goalRot);
	float r = distanceRight(goalRot);
	DBPL("[ROT] distanceTo: l=" << l << " r=" << r);
	if(abs(l) < abs(r)) {
		return l;
	} else {
		return r;
	}
}


/*** PRIVATE FUNCTIONS ***/

// Distance to goal counter-clockwise
float Rotation::distanceLeft(float goal) {
	if(goal >= angle) {
		return (goal - angle);
	} else {
		return ((360 - angle) + goal);
	}
}


// Distance to goal clockwise
//result will be negative, to show rotation towards the right
float Rotation::distanceRight(float goal) {
	if(goal >= angle) {
		return ( (-angle) - (360-goal) );  //(goal - 360 - angle);
	} else {
		return (goal - angle);
	}
}