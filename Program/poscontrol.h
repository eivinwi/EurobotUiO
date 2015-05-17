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
	void positionLoop(bool shouldOpen);
	void straightLoop(int dist);
	void reverseStraight(int dist);
	void reverseLoop();
	void crawlToRotation();

	float updateAngle();
	float updatePosition();
	float updatePositionReverse();

	void readEncoders();

	void setRotationSpeed(float angle_err);
	void setDriveSpeed(float straight_dist, float traveled);
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
	void printError();

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
		int pos_fast; //235, 255
		int pos_med;
		int pos_slow;
		int pos_crawl;
		int neg_fast; //20, 0
		int neg_med;
		int neg_slow;
		int neg_crawl;
	} speed_rot, speed_pos;

	int speed_stop = 128;

	struct slowdown{
		int dist_max = 120;
		int dist_med = 60;
		int rot_max = 70;
		int rot_med = 45;
		int rot_slow = 20;
		int straight_max = 200;
		int straight_med = 100;
		int startup_max = 200;
		int startup_med = 100;
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
		int position = 10000;
		int crawling = 1000;
		int action = 5000;
		long move_complete = 1000000;
	} TimeStep;

	int MAX_WAIT = 2000;
	int NO_ENCODER_ABORT = 500;
	int timeout_guard = 1000;


	struct gripper {
		int open_dist = 400;

	} Grippers;

	struct encoder {
		long e_0;
		long prev;
		long diff;
		float diff_dist;
		long total;
		float total_dist;
		float speed;
	} left_encoder, right_encoder;

	struct pos {
		float x;
		float y;
		float angle;
	} cur_pos, goal_pos, apr_pos;


	struct comp {
		float angle;
	} compass;

	// TODO: should be generalized instead of x,y,rot
	struct Cmd {
		int id;
		int x;
		int y;
		float rot;
		int argument;
		int type;
	};

	struct Motor {
		int speed;
	} left_motor, right_motor;

	struct exact {
		float x;
		float y;
		float angle;
		std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
	} Exact;


	float x_0, x_diff, goal_x;
	float y_0, y_diff, goal_y;
	float angle_0, angle_diff, goal_angle;



	int current_id = 0;


};

#endif /* POSCONTROL_H */