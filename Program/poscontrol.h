/*
 *  File: poscontrol.cpp
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
#ifndef POSCONTROL_H
#define	POSCONTROL_H

#include "liftcom.h"
#include "motorcom.h"
#include "dynacom.h"
#include "printing.h"
#include "config.h"
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
#include <fstream>
#include "yaml-cpp/yaml.h"

//state definitions for robot movement
#define NONE 0
#define ROTATION 1
#define FORWARD 2
#define REVERSE 3
#define STRAIGHT 7

//number of action IDs to store
#define ACTION_STORAGE 1024

//int curSpeedLeft, curSpeedRight;



class PosControl {
public:
	PosControl(MotorCom* m, DynaCom* d, bool test, std::string config_file);
	~PosControl();


	void reset(int x, int y, int rot);
	bool test();
	void enqueue(int id, int x, int y, float rot, int arg, int type);
	struct Cmd dequeue();
	void clearQueue();
	void controlLoop();
	void rotationLoop();
	float updateAngle();
	void setRotationSpeed(float angle_err);
	void straightLoop();
	void positionLoop();
	float updatePosition();
	float updatePositionReverse();
	void setDriveSpeed(float straight_dist);
	void setSpeeds(int l, int r);
	float shortestRotation(float angle, float goal);
	void readEncoders();
	float distStraight(float angle, float distX, float distY);
	bool angleCloseEnough();
	bool xCloseEnough();
	bool yCloseEnough();
	bool positionCloseEnough();
	bool inGoal();
	int getCurrentId();
	std::string getCurrentPos();
	std::string getState();

	int getLiftPos();
	bool running();
	void halt();
	void completeCurrent();
	void setCurrent(float x, float y, float angle);
	float sin_d(float angle);
	float cos_d(float angle);
	float atan2Adjusted(float x, float y);
	float atan2AdjustedReverse(float x, float y);
	float getSpeed();
	void readConfig(std::string filename);

private:	
	MotorCom *mcom;
	DynaCom *dcom;

	std::queue <Cmd>q;
	std::mutex qMutex;
	std::condition_variable notifier;
	std::string in;
	bool turning;
	bool working;
	bool testing;
	bool pos_running;
	//int curSpeedLeft, curSpeedRight;
	bool completed_actions[ACTION_STORAGE];


	int SPEED_MAX_NEG = 225; //235, 255
	int SPEED_MED_NEG = 192;
	int SPEED_SLOW_NEG = 150;
	int SPEED_STOP = 128;
	int SPEED_SLOW_POS = 106;
	int SPEED_MED_POS = 64;
	int SPEED_MAX_POS = 30; //20, 0


	int SLOWDOWN_MAX_DIST = 120;
	int SLOWDOWN_MED_DIST = 60;
	int SLOWDOWN_DISTANCE_ROT = 10;

	float ROTATION_CLOSE_ENOUGH = 1.0;
	float POSITION_CLOSE_ENOUGH = 2.0;

	//max increase from one encoder-reply to the next.
	int MAX_ENC_INCR = 980; // = one rotation. 

	//max difference between L and R encoder diffs.
	int MAX_ENC_DIFF = 440; // == half rotation. Should possibly be smaller
	int TOO_LONG = 20;             //timeout in ms. Currently not in use (?)

	// Game area resolution in mm:
	int XRES = 30000;
	int YRES = 20000;

	/*
	 * Encoder counts: 980 per output shaft turn
	 * Wheel diameter: 120mm
	 * Wheel circumference: 377mm
	 * Distance per count: 0.385mm  
	 */
	//#define ENCODER_CONSTANT 0.40	//0.385
	float ENCODER_CONSTANT = 0.327;	//0.385
	float DIST_PER_TICK = 0.382; //0.3265
	//#define ENC_PER_DEGREE 8.20 //8.55 		//6.2 		//7.5

	//Delays for serial communications
	int MAX_WAIT = 2000;


	struct speed{
		int neg_fast = 225; //235, 255
		int neg_med = 192;
		int neg_slow = 150;
		int stop = 128;
		int pos_slow = 106;
		int pos_med = 64;
		int pos_fast = 30; //20, 0
	} Speed;
};

#endif /* POSCONTROL_H */