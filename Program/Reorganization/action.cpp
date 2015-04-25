/*
 *	File: action.cpp
 *	Author: Eivind Wikheim
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

#include "action.h"

/* Should be defined somewhere centralized */
#define NONE 0
#define ROTATION 1
#define FORWARD 2
#define REVERSE 3
#define STRAIGHT 7


int type;

int id;
float x;
float y; 
float angle;

template<std::size_t SIZE> 
Action::Action(std::array<int, SIZE> array) 
{
	// size: rotation is padded with 0
	if(SIZE < 4) {
		PRINTLINE("[ACT] warning: array size too small: " << SIZE);
		reset();
	}
	else {
		reset();
		type = array[0];
		id = array[1];
		
		switch(type) {
			case ROTATION: 
				angle = array[2];
				break;
			case FORWARD: 
				x = array[2];
				y = array[3];
				break;
			case REVERSE: 
				x = array[2];
				y = array[3];				
				break;
			case STRAIGHT: 
				x = array[2];
				y = array[3];
				break;
			default:
				LOG(WARNING) << "[ACT] invalid action type: " << type;
		}



	}
}

Action::~Action() {
}

void Action::reset() {
	type = NONE;
	id = 0;
	x = 0;
	y = 0;
	angle = 0;
}