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
	GoalPosition();
	GoalPosition(int id, float x, float y, float r);
	~GoalPosition();

	void reset();
	void setId(int id);
	void setAngle(float angle);
	void setPosition(float n_x, float n_y);
	void set(float n_x, float n_y, float n_rotation);
	double timeSinceUpdate();

	int getId();
	float getX();
	float getY();
	float getAngle();
	std::string getPosString();

	void print();

private:
	std::atomic<int> id;
	float x;
	float y;
	float rotation;
};

#endif /* GOALPOSITION_H */