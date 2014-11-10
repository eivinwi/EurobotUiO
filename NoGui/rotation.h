#ifndef ROTATION_H
#define	ROTATION_H

#include "printing.h"
#include "md49.h"
#include <iostream>

#define ENC_PER_DEGREE 6.2 //7.5

class Rotation {
public:
    Rotation();
    ~Rotation();
    void reset();
    void rotateTowards(float goalRot);

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
