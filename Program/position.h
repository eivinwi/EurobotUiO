/*  
 * File: position.h
 * Author: Eivind Wikheim
 *
 * Contains the current estimated robot position, and functions for using/updating.
 * Is used by PosControl for calculating path, and is updated continuosly.
 */

#ifndef POSITION_H
#define POSITION_H

#include "md49.h"
#include "printing.h"
#include "rotation.h"
#include <mutex>
#include <string>
#include <sstream>
#include <time.h>


class Position {
public:
	Position();
	~Position();
	void reset();
	void set(double n_x, double n_y, double n_rotation);
	void setAngle(double angle);
	double getAngle();
	double getX();
	double getY();

	// Distance in x-dir to input goal
	double distanceX(double to);
	// Distance in y-dir to input goal
	double distanceY(double to);
	// Distance in rotation to input goal
	double distanceRot(double to);

	// Deprecated: returns time since position was last updated
	//double timeSinceUpdate();

	// Calls on updateAngle in Rotation to calculate curren angle based on encoder readings
	void updateAngle(double leftDiff, double rightDiff);

	
	// Adds positional updates to x
	void updateX(double dist);
	// Adds positional updates to y
	void updateY(double dist);

	void print();

	// Returns position as comma-delimited string. Thread-safe
	std::string getPosString();
	
	void updatePosString();

private:
	double x;
	double y;
	Rotation *rotation;
	time_t changed;

	std::string pos_string;
	std::mutex pos_mutex;
};

#endif /* POSITION_H */