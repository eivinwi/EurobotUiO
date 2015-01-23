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
	void set(float n_x, float n_y, float n_rotation);
	void setAngle(float angle);
	float distanceX(float to);
	float distanceY(float to);
	float distanceRot(float to);
	double timeSinceUpdate();
	float getAngle();
	float getX();
	float getY();
	void updateAngle(float leftDiff, float rightDiff);
	std::string getPosString();
	
	//update x/y based on encoder readings
	void updateX(float dist);
	void updateY(float dist);

	void print();
	void updatePosString();

private:
	float x;
	float y;
	Rotation *rotation;
	time_t changed;

	std::string pos_string;
	std::mutex pos_mutex;
};

#endif /* POSITION_H */