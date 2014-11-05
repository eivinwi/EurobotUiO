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

// Game area resolution in mm:
#define XRES 30000
#define YRES 20000



void *worker_routine(void *arg);

class Poscontrol {
public:
    Poscontrol(MotorCom *s);
    ~Poscontrol();

    bool inGoal();
	void testDrive(int x, int y);    
	void drive();
	void setGoalPos(int x, int y, int r);
	int distanceFromX();
	int distanceFromY();
	void updatePosition();


private:
	std::string in;

	MotorCom *com;

};

#endif /* POSCONTROL_H */


