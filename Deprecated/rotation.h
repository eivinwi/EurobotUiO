/*
 *  File: rotation.h
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

#ifndef ROTATION_H
#define	ROTATION_H

#include "md49.h"
#include "printing.h"
#include <atomic>

class Rotation {
public:
    Rotation();
    ~Rotation();
    void reset();
    void set(float a);
    float get();
    
    //calculate new angle based on encoders
    void updateAngle(long diffL, long diffR);
    

    // Returns shortest distance to goal rotation
    float distanceTo(float goal);

private:
    // Returns distance to goal in positive direction
    float distanceLeft(float goal);

    // Returns distance to goal in negative direction
    float distanceRight(float goal);

	float angle;
    
};

#endif /* ROTATION_H */