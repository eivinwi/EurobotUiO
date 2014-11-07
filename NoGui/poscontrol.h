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
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include <cassert>
#include <time.h>

// Game area resolution in mm:
#define XRES 30000
#define YRES 20000

#define REASONABLE_ENC_DIFF 980

#define TURNING 0
#define DRIVE_X 1
#define DRIVE_Y 2

void *worker_routine(void *arg);

class PosControl {
public:
    PosControl(MotorCom *s);
    ~PosControl();

    bool controlLoop();
    void stop();
    void turn(int distR);
    void drive(float distX, float distY);
	void setGoalPos(int x, int y, int r);
	float distanceFromX();
	float distanceFromY();
	int rotationOffset();
	long encoderDifference();
	void updatePosition(int action);
	int getRotation();
	bool closeEnoughAngle(int a, int b); 
	bool closeEnoughEnc(long a, long b);
	void updateEncoder(long e, struct encoder *side);
	void updateLeftEncoder();
	void updateRightEncoder();
	float average(long a, long b);
	float currentRotation();
	//seconds
	double timeSinceGoal(); 

private:
	std::string in;

	MotorCom *com;

	bool turning;

};

#endif /* POSCONTROL_H */


