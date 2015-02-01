/*  
 * File: goalPosition.h
 * Author: Eivind Wikheim
 *
 * Contains the current (exact) goal position, and functions for calculating/updating based on it.
 * Each task has an unique task-ID (int id).   
 *
 * IMPORTANT:
 * - Is EITHER a rotational or a positional change. 
 * 		- Can only be both rotational and positional when manipulated internally by PosControl
 *
 * TODO:
 * 	  - atomic?
 *    - remove set functions
 */

#ifndef GOALPOSITION_H
#define GOALPOSITION_H

#include "rotation.h"
#include "md49.h"
#include "printing.h"

#include <atomic>
#include <math.h>
#include <string>
#include <sstream>
#include <unistd.h>

class GoalPosition {
public:
	//Only the constructor with arguments should probably be used.
	GoalPosition(int id, double x, double y, double r);
	GoalPosition();
	~GoalPosition();
	void reset();

	//set functions should probably not be used, as goal should be created using constructors
	void setId(int id);
	void setAngle(double angle);
	void setPosition(double n_x, double n_y);
	void set(double n_x, double n_y, double n_rotation);

	int getId();
	double getX();
	double getY();
	double getAngle();

	// Returns goal position as a comma-delimited string
	std::string getPosString();

	// Prints goal position to terminal
	void print();

private:
	//no idea why this is atomic
	std::atomic<int> id;
	
	double x;
	double y;
	double rotation;
};

#endif /* GOALPOSITION_H */