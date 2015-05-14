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

#include <bitset>
#include <condition_variable>
#include <math.h>
#include <mutex>
#include <queue>
#include <string>
#include <sstream>
#include <thread>
#include <time.h>
#include <vector>
#include "yaml-cpp/yaml.h"
 
#include "dynacom.h"
#include "motorcom.h"
#include "printing.h"


//number of action IDs to store
#define ACTION_STORAGE 2000



class PosControl {
public:
	PosControl(MotorCom* m, DynaCom* d, bool test, std::string config_file);
	~PosControl();
	void reset(int x, int y, int rot);
	bool test();

	void enqueue(std::vector<int> arr);
	std::vector<int> dequeue();
	void clearQueue();

	void controlLoop();
	void rotationLoop();
	void positionLoop();
	void straightLoop(int dist);
	void reverseStraight(int dist);
	void reverseLoop();
	void crawlToRotation();

	float updateAngle();
	float updatePosition();
	float updatePositionReverse();

	void readEncoders();

	void setRotationSpeed(float angle_err);
	void setDriveSpeed(float straight_dist);
	void setSpeeds(int l, int r);

	float shortestRotation(float angle, float goal);
	float distStraight(float angle, float distX, float distY);

	bool angleCloseEnough();
	bool xCloseEnough();
	bool yCloseEnough();
	bool positionCloseEnough();
	bool inGoal();

	std::string getPosStr();
	std::string getState();
	std::string getGripperPos();
	void completeCurrent();

	float perc(float angle, float starting_angle, float goal);
	float percPos(float s, float c, float g);
	void setCurrent(float x, float y, float angle);
	bool running();
	void halt();
	int completed(int id);
	float getSpeed();

	float sin_d(float angle);
	float cos_d(float angle);
	float atan2Adjusted(float x, float y);
	float atan2AdjustedReverse(float x, float y);

	void readConfig(std::string filename);

	void pickUpLoop();

	void setExactPos(std::vector<float> v);


private:	
	MotorCom *mcom;
	DynaCom *dcom;

	std::queue<std::vector<int>> q;

	std::mutex qMutex;
	std::condition_variable notifier;
	std::string in;
	bool turning;
	bool working;
	bool testing;
	bool pos_running;
	bool completed_actions[ACTION_STORAGE];

	struct speed{
		int pos_fast = 225; //235, 255
		int pos_med = 192;
		int pos_slow = 150;
		int neg_slow = 106;
		int neg_med = 64;
		int neg_fast = 30; //20, 0
	} speed_rot, speed_pos;

	int speed_stop = 128;

	struct slowdown{
		int max_dist = 120;
		int med_dist = 60;
		int max_rot = 30;
		int med_rot = 15;
		int max_straight = 200;
		int med_straight = 100;
	} Slowdown;


	struct encoder_stuff{
		int max_incr = 980;
		int max_diff = 440;
		float constant = 0.327;
		float dist_per_tick = 0.382;
		float per_degree = 8.05;
	} Enc;

	struct closeenough{
		float rotation = 2.0;
		float position = 2.0;
	} CloseEnough;


	struct timestep {
		int rotation = 5000;
		int position = 5000;
		int crawling = 1000;
		int action = 5000;
		long move_complete = 800000;
	} TimeStep;

	int MAX_WAIT = 2000;
	int NO_ENCODER_ABORT = 500;
	int timeout_guard = 1000;

};

#endif /* POSCONTROL_H */