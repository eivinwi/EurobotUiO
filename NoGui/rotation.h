#ifndef ROTATION_H
#define	ROTATION_H

#include "printing.h"
#include <iostream>

#define ENC_PER_DEGREE 15.0 //7.5

class Rotation {
public:
    Rotation();
    ~Rotation();
    void resetAngle();
    void rotateTowards(float goalRot);

    //calculate new angle based on encoders
    void updateAngle(long diffL, long diffR);

    void setAngle(float a);
    float distanceTo(float goal);
    float distanceLeft(float goal);
	float distanceRight(float goal);
    float getAngle();

private:
	float angle;
};

#endif /* ROTATION_H */
