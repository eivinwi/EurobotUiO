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

#include "goalposition.h"
#include "liftcom.h"
#include "motorcom.h"
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

// Game area resolution in mm:
#define XRES 30000
#define YRES 20000

#define REASONABLE_ENC_DIFF 980 // == one rotation. Sould possibly be smaller
#define TOO_LONG 20             //timeout in ms. Currently not in use (?)

//type definitions
#define NONE 0
#define ROTATION 1
#define POSITION 2

#define ACTION_STORAGE 200

class PosControl {
public:
    PosControl(MotorCom *m, LiftCom *l, bool testing);
    ~PosControl();
    void reset();
    bool test();

    void enableTesting();
    void controlLoop();
	void enqueue(int id, int x, int y, float rot, int arg, int type);
	struct qPos dequeue();

	void setGoalRotation(int r);
	void setGoalPosition(int x, int y);

	int getCurrentId();
	std::string getCurrentPos();
	int getLiftPos();
	bool running();
	
private:
	void logTrace();

	void completeCurrent();
	void goToRotation();
    void goToPosition();
    void goToLift(int arg);

    void rotate(float distR);
    void drive(float dist);
    void fullStop();
	float distanceX();
	float distanceY();
	float distanceAngle();

	float updateDist(float angle, float distX, float distY);
	void updatePosition();
	void updateRotation();

	bool closeEnoughEnc(long a, long b);
	bool closeEnoughX();
	bool closeEnoughY();
	bool closeEnoughAngle();
	bool inGoalPosition();
	bool inGoal();

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

	std::queue <qPos>q;
	std::mutex qMutex;
	std::condition_variable notifier;

	std::string in;
	MotorCom *mcom;
	LiftCom *lcom;
	bool turning;
	bool working;
	GoalPosition *goalPos;
	Position *curPos;
	Position *exactPos;

	int itr;

	int curSpeedLeft;
	int curSpeedRight;

	bool completed_actions[ACTION_STORAGE];
	bool testing;
	bool pos_running;
};

#endif /* POSCONTROL_H */