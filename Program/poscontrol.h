/*
 *  File: poscontrol.h
 *  Author: Eivind Wikheim
 *
 *	PosControl controls navigation and tool movement via MotorCom and LiftCom objects respectively. 
 *  commandLoop() runs continuosly in its own thread, initialized in main.cpp.
 *
 *  Copyright (c) 2015 Eivind Wikheim <eivinwi@ifi.uio.no>. All Rights Reserved.
 *
 *  This file is part of EurobotUiO.
 *
 *  EurobotUiO is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EurobotUiO is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EurobotUiO.  If not, see <http://www.gnu.org/licenses/>.
 */

// TODO:
// - tune algorithms for accuracy (wait for new wheels)
// - encoders in own class?
//
//  The resolution of the positioning system is 3000x2000, where 1 = 1cm

#ifndef POSCONTROL_H
#define POSCONTROL_H

#define _USE_MATH_DEFINES

#include "goalposition.h"
#include "liftcom.h"
#include "motorcom.h"
#include "dynacom.h"
#include "position.h"
#include "printing.h"
#include "rotation.h"
#include <cassert>
#include <condition_variable>
#include <math.h>
#include <mutex>
#include <queue>
#include <string>
#include <sstream>
#include <thread>
#include <time.h>
#include <vector>

//state definitions for robot movement
#define NONE 0
#define ROTATION 1
#define FORWARD 2
#define REVERSE 3
#define STRAIGHT 7

//number of action IDs to store
#define ACTION_STORAGE 1024

class PosControl {
public:
	// m = instance of MotorCom used to communicate with motors trough MD49 via serial
	// l = instance of LiftCom used to communicate with lift/gripper-arduino via serial
	// testing: see enableTesting()
    PosControl(MotorCom *m, LiftCom *l, DynaCom *d, bool testing);
    ~PosControl();

    //Reset all counters, encoders etc. Sets initial position (x, y, rot)
    void reset(int x, int y, int rot);

    // Tests if poscontrol is working by enqeueueing and dequeueing an object
    bool test();

    // Testing enabled means actions will be simulated, and always seemingly finish
	// successfully and instanteneously.
    void enableTesting();

    // Main execution-loop, runs in its own thread initialized in main.cpp
    // Waits for a notifier from the action-queue saying its not empty, then 
    // dequeues an action and starts executing it.
    void controlLoop();

	void enqueue(int id, int x, int y, float rot, int arg, int type);

	// Returns id of the action currently being executed. 0 if none
	int getCurrentId();

	// Returns current position as comma-delimited string.
	// First char is 'w' if working, 's' if stopped (completed).
	std::string getCurrentPos();
	
	// Returns lift-position as int (defined in liftcom.h)
	int getLiftPos();

	// Returns true if a command is currently being executed.
	bool running();
	
private:
	// Waits untill queue-size is >0 (using a Notifier), then returns the first action in the queue.
	struct Cmd dequeue();


	// Logs current position with LOG(TRACE)
	void logTrace();

	// Sets current action as completed (by ID). 
	// Pauses for a brief moment to ensure no action overlap.
	void completeCurrent();
	
	// Loops untill goal rotation is reached.
	void goToRotation();
 	
	// Calculates angle to turn to in order to drive from curPos to goalPos,
	// rotates there, and then loops untill goal position is reached. 
	void goToPosition();

	// Similar to goToPosition(), but reverses the robot instead.
	void goToReverse();

	// Drive a (input) distance in the current rotation. 
	// Calculates goalX,goalY by trigonometry based on the input and the current rotation
	void goStraight();

    //Sends command to lift-arduino, and waits for ACK before returning
    void goToLift(int arg);

 	// Sets motor speeds to advance towards goal rotation
    void rotate(float distR);

 	// Sets motor speeds to advance towards goal position    
    void drive(float dist);

    // Similar to drive(), but sends opposite directions to MotorCom
    void drive_reverse(float dist);

    // Sends stop commands to the motors.
    void fullStop();

   	// Returns distance from cur x to goal x
	float distanceX();

   	// Returns distance from cur y to goal y
	float distanceY();

	// Returns distance, in degrees, from current orientaton to goal orientation
	float distanceAngle();

	// Calculates straight distance from current position to goal position by trigonometry.
	float updateDist(float angle, float distX, float distY);

	// Updates current position based on encoder-readings
	// returns distance traveled (straight)
	float updatePosition();
	float updatePositionReverse();

	// Updates current rotation based on encoder-readings
	void updateRotation();

	// Returns true if current x and goal x is the same within margin of error
	bool closeEnoughX();

	// Returns true if current y and goal y is the same within margin of error
	bool closeEnoughY();

	// Returns true if current orientation and goal orientation is the same within margin of error
	bool closeEnoughAngle();

	// Returns true if in goal-position (x,y)
	bool inGoalPosition();

	// Return true if in goal (x, y, r)
	bool inGoal();

	// Returns difference in the absolute value of left and right encoders
	long encoderDifference();

	// Updates values in enc based on reading of the encoder in question
	void updateEncoder(long e, struct encoder *enc);

	// Calls update on left encoder
	void updateLeftEncoder();
	// Calls update on right encoder
	void updateRightEncoder();

	// Zeros out the encoder structs and sends reset-command to motor controller
	// Should only be called at program startup and if something is wrong.
	// Fun facts:
	// 32bit: Robot can drive 859km before encoder values overflow ~= 0.13*(earth radius)
	// 64bit: Robot can drive 3.7*10^12 km before overflow ~= 0.39 light years
	void resetEncoders();

	// DEBUGING: Prints goal position to terminal
	void printGoal();

	// DEBUGING: Prints current position to terminal
	void printCurrent();

	// DEBUGING: Prints distance from goal to terminal
	void printDist();

	// Changes motors speeds via MotorCom
	void setSpeed(int l, int r);

	// returns sin/cos of angle in radians, in the correct quadrant
	float sin_d(float angle);
	float cos_d(float angle);

	// DEBUGING: prints current values in encoder struct
	void printEncoder(struct encoder *e);

	// Serial ports
	MotorCom *mcom;
	LiftCom *lcom;
	DynaCom *dcom;

	// Position objects
	GoalPosition *goalPos;
	Position *curPos;
	Position *exactPos;

	//Command queue
	std::queue <Cmd>q;
	std::mutex qMutex;
	std::condition_variable notifier;
	
	std::string in;

	// Robot-state
	bool turning;
	bool working;

	// While testing, always set action to success, instantly, without performing it. 
	bool testing;
	
	// True if main controlloop is running (initizalied in own thread in main.cpp).  
	bool pos_running;

	// Used to prevent unneccessary motor communication. TODO: reinstate
	int curSpeedLeft, curSpeedRight;

	// completed_actions[n] is true if action with ID=n has been completed.
	bool completed_actions[ACTION_STORAGE];
};

#endif /* POSCONTROL_H */