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
#include <iostream>

#define ENC_PER_DEGREE 6.2 //7.5

class Rotation {
public:
    Rotation();
    ~Rotation();
    void reset();
    
    //calculate new angle based on encoders
    void updateAngle(long diffL, long diffR);

    void set(float a);
    float distanceTo(float goal);
    float distanceLeft(float goal);
	float distanceRight(float goal);
    float get();

private:
	float angle;
    
};

#endif /* ROTATION_H */