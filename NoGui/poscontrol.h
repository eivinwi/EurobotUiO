/* 
 * File: poscontrol.h
 * Author: Eivind Wikheim
 *
 * TODO:
 * - driver robot from its current position to goal position in a controlled manner.
 * - tune algorithms for accuracy
 *
 * INPUTS via zeromq in communication.h:
 * - from AI: goal pos, goal orientation
 * - from SENS: true position
 *
 *
 * The resolution of the positioning system is 3000x2000, where 1res = 1cm
 */

#ifndef POSCONTROL_H
#define POSCONTROL_H

#include "motorcom.h"
#include "printing.h"
#include <string>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include <cassert>
#include <time.h>
#include <math.h>

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


void *worker_routine(void *arg);

class PosControl {
public:
    PosControl(MotorCom *s);
    ~PosControl();
    void resetPosition();
    bool controlLoop();
	void setGoalPos(int x, int y, int r);
	float currentRotation();
	std::string getCurrentPos();
	bool inGoal();

private:
	long encoderDifference();
    void changeRotation(float distR);
    void driveX(float distX);
    void driveY(float distY);
    void fullStop();
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

	double timeSinceGoal(); 
	void printGoal();
	void printCurrent();
	void printDist();

	std::string in;
	MotorCom *com;
	bool turning;

};

#endif /* POSCONTROL_H */