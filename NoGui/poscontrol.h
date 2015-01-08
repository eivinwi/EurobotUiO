/* 
 * File: poscontrol.h
 * Author: Eivind Wikheim
 *
 * Controls the motors by sending commands to MotorCom. commandLoop() runs 
 * continuosly in an own thread, and moves the robot while curPos is not close
 * enough to goalPos. 
 *
 *
 * TODO:
 * - tune algorithms for accuracy
 * - encoders in own class?
 *
 * The resolution of the positioning system is 3000x2000, where 1 = 1cm
 */

#ifndef POSCONTROL_H
#define POSCONTROL_H

#define _USE_MATH_DEFINES

#include "motorcom.h"
#include "printing.h"
#include "position.h"
#include "goalposition.h"
#include "rotation.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <thread>


#define PI 3.14159265

// Game area resolution in mm:
#define XRES 30000
#define YRES 20000

#define REASONABLE_ENC_DIFF 980
#define TOO_LONG 20

class PosControl {
public:
    PosControl(MotorCom *s);
    ~PosControl();
    void resetPosition();
    void controlLoop();
	void setGoalPos(int x, int y, int r);
	std::string getCurrentPos();
	
private:
	bool inGoal();
    void rotate(float distR);
    void drive(float dist);
    void fullStop();
//    float currentRotation();
	float distanceX();
	float distanceY();
	float distanceAngle();

	float updateDist(float angle, float distX, float distY);

	void updatePosition();
	void updateRotation();

	//bool closeEnoughPos(float a, float b);
	bool closeEnoughEnc(long a, long b);

	bool closeEnoughX();
	bool closeEnoughY();
	bool closeEnoughAngle();


	long encoderDifference();
	void updateEncoder(long e, struct encoder *side);
	void updateLeftEncoder();
	void updateRightEncoder();
	void resetEncoders();

	float average(long a, long b);
	void printGoal();
	void printCurrent();
	void printDist();
	void setSpeed(int l, int r);

	float sin_d(float angle);
	float cos_d(float angle);

	std::string in;
	MotorCom *com;
	bool turning;
	bool working;
	GoalPosition *goalPos;
	Position *curPos;
	Position *exactPos;

	int itr;

	int curSpeedLeft;
	int curSpeedRight;
};

#endif /* POSCONTROL_H */