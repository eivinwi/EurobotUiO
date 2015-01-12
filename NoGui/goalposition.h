/*  
 * File: goalPosition.h
 * Author: Eivind Wikheim
 *
 * Contains the current (exact) goal position, and functions for using/updating it.
 * 
 * IMPORTANT:
 * - Is EITHER a rotational or a positional change. 
 * - Can only be both rotational and positional when used internally by PosControl
 *
 */

#ifndef GOALPOSITION_H
#define GOALPOSITION_H

#include "rotation.h"
#include "md49.h"
#include "printing.h"
#include <unistd.h>
#include <time.h>
#include <string>
#include <sstream>
#include <math.h>
#include <atomic>


/* curPos - current estimated position based on dead reckoning with encoders. Is updated regularly to be Exactpos
 * ExactPos   - input position from SENS
 * GoalPos    - input from main or AI 
*/
class GoalPosition {
public:
	GoalPosition();
	~GoalPosition();

	void reset();
	void setAngle(float angle);
	void setPosition(int n_x, int n_y);
	void set(int n_x, int n_y, float n_rotation);
	double timeSinceUpdate();
	float getRotation();
	float getX();
	float getY();
	std::string getPosString();

	void print();

private:
	int x;
	int y;
	float rotation;
	time_t changed;

};

#endif /* GOALPOSITION_H */