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

//int curSpeedLeft, curSpeedRight;

#define DIST_PER_TICK 0.3265


class PosControl {
public:
	PosControl(MotorCom* m, DynaCom* d, bool test);
	~PosControl();


	void reset(int x, int y, int rot);
	bool test();
	void enqueue(int id, int x, int y, float rot, int arg, int type);
	struct Cmd dequeue();
	void clearQueue();
	void controlLoop();
	void rotationLoop();
	void updateAngle();
	void setRotationSpeed(float angle_err);
	void positionLoop();
	void updatePosition();
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

	int getLiftPos();
	bool running();
	void halt();
	void completeCurrent();
	void setCurrent(float x, float y, float angle);
	float sin_d(float angle);
	float cos_d(float angle);

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
};

#endif /* POSCONTROL_H */