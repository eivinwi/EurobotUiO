/*
 *	File: action.h
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

#ifndef ACTION_H
#define ACTION_H

#include "md49.h"
#include "printing.h"

#include <atomic>
#include <math.h>
#include <string>
#include <sstream>
#include <unistd.h>


class Action {
public:
	//Only the constructor with arguments should probably be used.
	template<std::size_t SIZE>
	Action(int id, float x, float y, float r);
	
	Action();
	~Action();


private:


};

#endif /* ACTION_H */