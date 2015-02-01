/*  
 * File: rotation.h
 * Author: Eivind Wikheim
 *
 * Contains the current estimated robot rotation (angle), 
 * and functions for calculating distances (in degrees).
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
    void set(double a);
    double get();
    
    //calculate new angle based on encoders
    void updateAngle(long diffL, long diffR);
    
    // Returns distance to goal in positive direction
    double distanceLeft(double goal);

    // Returns distance to goal in negative direction
    double distanceRight(double goal);

    // Returns shortest distance to goal rotation
    double distanceTo(double goal);

private:
	double angle;
    
};

#endif /* ROTATION_H */