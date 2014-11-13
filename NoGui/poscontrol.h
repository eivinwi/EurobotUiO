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

// Game area resolution in mm:
#define XRES 30000
#define YRES 20000

#define REASONABLE_ENC_DIFF 980

#define TURNING 0
#define DRIVE_X 1
#define DRIVE_Y 2

#define ROTATION_CLOSE_ENOUGH 1.0
#define POSITION_CLOSE_ENOUGH 1.0
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
	long encoderDifference();
    void changeRotation(float distR);
    void driveX(float distX);
    void driveY(float distY);
    void fullStop();
//    float currentRotation();
	float distanceX();
	float distanceY();
	float rotationOffset();
	void updatePosition(int action);
	bool closeEnoughAngle(int a, int b); 
	bool closeEnoughPos(float a, float b);
	bool closeEnoughEnc(long a, long b);
	void updateEncoder(long e, struct encoder *side);
	void updateLeftEncoder();
	void updateRightEncoder();
	float average(long a, long b);
	void printGoal();
	void printCurrent();
	void printDist();

	std::string in;
	MotorCom *com;
	bool turning;
	bool working;
	GoalPosition *goalPos;
	Position *curPos;
	Position *exactPos;

};

#endif /* POSCONTROL_H */