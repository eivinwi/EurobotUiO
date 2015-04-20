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

#include "poscontrol.h"
//TODO: 	
//  - CODE CLEANUP. Program is extremely confusing atm
// 	- use IMU for angle untill beacon system is working
// 	- should continually check angle instead of once!! (impossible with only encoders)
//
//	- Overshoot protection

struct encoder {
	long prev;
	long diff;
	float diff_dist;
	long total;
	float total_dist;
	float speed;
} left_encoder, right_encoder;


auto encoder_timestamp = std::chrono::high_resolution_clock::now();
auto compass_timestamp = std::chrono::high_resolution_clock::now();

struct pos {
	float x;
	float y;
	float angle;
} cur_pos, goal_pos;

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




PosControl::PosControl(MotorCom *m, DynaCom *d, bool test) {
	mcom = m;
	dcom = d;
	testing = test;
	reset(0,0,0);
}


void PosControl::reset(int x, int y, int rot) {
	cur_pos.x = 0;
	cur_pos.y = 0;
	cur_pos.angle = 0;
	goal_pos.x = 0;
	goal_pos.y = 0;
	goal_pos.angle = 0;
	left_encoder.prev = 0;
	left_encoder.diff = 0;
	left_encoder.diff_dist = 0.0;
	left_encoder.total = 0;
	left_encoder.total_dist = 0.0;
	left_encoder.speed = 0.0;
	right_encoder.prev = 0;
	right_encoder.diff = 0;
	right_encoder.diff_dist = 0.0;
	right_encoder.total = 0;
	right_encoder.total_dist = 0.0;
	right_encoder.speed = 0.0;
	
	encoder_timestamp = std::chrono::high_resolution_clock::now();


	compass.angle = 0.0;
	compass_timestamp = std::chrono::high_resolution_clock::now();

	mcom->resetEncoders();
//	curSpeedRight = 0;/
//	curSpeedLeft = 0;
	//cur_pos->updatePosString();
	clearQueue();
	std::fill(std::begin(completed_actions), std::end(completed_actions), false);
	while(!q.empty()) {
		q.pop();
	}
}


bool PosControl::test() {
	enqueue(0, 1, 2, 3, 4, NONE);
	Cmd test = dequeue();
	return (test.id == 0 && test.x == 1 && test.y == 2 && test.rot == 3 && test.argument == 4 && test.type == NONE);
}


void PosControl::enqueue(int id, int x, int y, float rot, int arg, int type) {
	Cmd cmd = {id, x, y, rot, arg, type};
	std::lock_guard<std::mutex> lock(qMutex);	
	q.push(cmd);
	//TIMESTAMP("------- Sending notify");
	notifier.notify_one();
	//TIMESTAMP("------- Sent notify");
}


Cmd PosControl::dequeue() {
	std::unique_lock<std::mutex> lock(qMutex);
	//TIMESTAMP("------- Waiting");
	while(q.empty()) {
		notifier.wait(lock); //alternative implementation try_unlock with crono ms-timeout
		//TIMESTAMP("------- Lock Open");
	}
	//TIMESTAMP("------- outside loop");

	Cmd cmd = q.front();
	q.pop();
	usleep(100);
	return cmd;
}


void PosControl::clearQueue() {
	std::queue <Cmd> newQ;
	q = newQ;
}



// Main program, loops once every time a command is dequeued.
// If the testing-variable is set to true, automatically sets action to completed,
// and sets current position (and other vars) to the goal
void PosControl::controlLoop() {
	pos_running = true;
	while(true) {
		Cmd cmd = dequeue();
		working = true;

		//goal_pos = new goal_position(cmd.id, cmd.x, cmd.y, cmd.rot);
		goal_pos.x = cmd.x;
		goal_pos.y = cmd.y;
		goal_pos.angle = cmd.rot;
		LOG(INFO) << "[POS] thread <" << std::this_thread::get_id() << "> dequeued Cmd {"
					<< cmd.id << "," << cmd.x << "," << cmd.y << "," << cmd.rot << "," << cmd.type << "}";
		
		if(!testing) {
			if(cmd.type == ROTATION) {
				rotationLoop();
			} else if(cmd.type == FORWARD) {
				positionLoop();
			} else if(cmd.type == REVERSE) {
				//goToReverse();
			} else if(cmd.type == STRAIGHT) {
				//goStraight();
			}
			else if(cmd.type == LIFT) {
				LOG(INFO) << "DynaCom action: " << cmd.argument;				
				dcom->performAction(cmd.argument);
			}
			halt();
			completeCurrent();
		} 
		//CHECK: "testing"-functionality should perhaps be implemented in each of the Com's
		else {
			if(cmd.type == ROTATION) {
				cur_pos.angle = goal_pos.angle;
			} else if(cmd.type == FORWARD || cmd.type == REVERSE) {
				setCurrent(goal_pos.x, goal_pos.y, cur_pos.angle);
			} else if(cmd.type == STRAIGHT) {
				setCurrent(goal_pos.x, goal_pos.y, cur_pos.angle);
			} else if(cmd.type == LIFT) {
				dcom->performAction(cmd.argument);
			}
			completeCurrent();
		}
		//logTrace();
	}
	pos_running = false;
}



void PosControl::rotationLoop() {
	PRINTLINE("[POS]  rotation: " << cur_pos.angle << " -> " << goal_pos.angle);
	
	do {
		float angle_err = shortestRotation(cur_pos.angle, goal_pos.angle);
//		float compass_err = shortestRotation(compass.angle, goal_pos.angle);

		//logic to compare angle_err to compass_err

		setRotationSpeed(angle_err);

		//sleep(N)
		readEncoders();
		//calculate distance traveled

//		leftwheel_x = left_encoder.diffDist * cos_d(cur_pos.angle);
//		leftwheel_y = left_encoder.diffDist * sin_d(cur_pos.angle);
//		rightwheel_x = right_encoder.diffDist * cos_d(cur_pos.angle);
//		rightwheel_y = right_encoder.diffDist * sin_d(cur_pos.angle);

//		float leftwheel_turned = left_encoder.diff / ENC_PER_DEGREE; //8.05
//		float rightwheel_turned = right_encoder.diff / ENC_PER_DEGREE;

		//
		updateAngle();

		//calculate turning speed

	}
	while(!angleCloseEnough());


	LOG(INFO) << "[POS] Rotation completed. Goal=" << goal_pos.angle << " cur_pos: " << cur_pos.angle << " Compass: " << compass.angle;	
}


//should compare with compass
void PosControl::updateAngle() {
	long diffL = left_encoder.diff; 
	long diffR = right_encoder.diff;
  	// Average of encoders for reduced maximum-error
	long enc_avg = (abs(diffL) + abs(diffR))/2;

	if(abs(enc_avg) < 100) {//MAX_ENC_TURN_DIFF) {
		LOG(INFO) << "[POS] encoder diff too large (" << enc_avg << "), checking compass";
		//difference between encoders, scrap odometry and rely on compass
		auto now = std::chrono::high_resolution_clock::now();
		auto time_since_compass = std::chrono::duration<double, std::milli>(now - compass_timestamp).count();
		if(time_since_compass < 2000) {
			cur_pos.angle = compass.angle;
			LOG(INFO) << "[POS] current angle set to compass angle: " << compass.angle;
			return;
		} 
		else {
			LOG(WARNING) << "[POS] encoders too high difference (" << diffL << " , " << diffR << ") and compass outdated (" << time_since_compass << " ms!!!)";
		}
	}

	//average out compass and odometry??

	float turned = (diffL > 0)? -(enc_avg/ENC_PER_DEGREE) : (enc_avg/ENC_PER_DEGREE);

	cur_pos.angle += turned;
	if(cur_pos.angle >= 360.0) {
		cur_pos.angle -= 360.0;
	} 
	else if(cur_pos.angle < 0.0) {
		cur_pos.angle += 360.0;
	}
}

void PosControl::setRotationSpeed(float angle_err) {
	if(angle_err > 30) {
		
	}
	else if(angle_err > 0) {
		
	}
	else if(angle_err < -30) {
		
	}
	else if(angle_err < 0) {
		
	} 
	else {

	}
}


void PosControl::positionLoop() {
	float angle = cur_pos.angle;
//	float compass_angle = compass.angle;

	//check if matching


	do {
		float dist_x = goal_pos.x - cur_pos.x;
		float dist_y = goal_pos.y - cur_pos.y;
		float straight = distStraight(angle, dist_x, dist_y);

		//find vector towards target
		angle = atan2(dist_y, dist_x) *(180/M_PI);
		if(angle < 0) angle += 360;

		//define angle max offset
		if(abs(cur_pos.angle - angle) > 1) {
			//attempt to calculate necessary speed differences
			LOG(INFO) << "[POS] angle has drifted, attempting to compensate. (TODO)";
		} else {
			setDriveSpeed(straight);
		}


		usleep(3000); //needs tweaking

		readEncoders();
		updatePosition();
		//check absolute position
	} while(!positionCloseEnough());



	LOG(INFO) << "[POS] position reached . Goal=(" << goal_pos.x << "," << goal_pos.y << " cur_pos: (" << cur_pos.x << "," << cur_pos.y << ")";	
}


//angle has already been checked when this is called, assume 100% correct.
void PosControl::updatePosition() {
	long diff = abs(left_encoder.diff_dist) - abs(right_encoder.diff_dist);

	//MAX_DIFF
	if(diff > 100) {
		LOG(WARNING) << "[POS] large distance difference, probably spinning or hitting something.";
	}


	float angle = cur_pos.angle;

	float avg_dist = (abs(left_encoder.diff_dist) + abs(right_encoder.diff_dist)) / 2;
	float x_distance = cos_d(angle) * avg_dist; 
	float y_distance = sin_d(angle) * avg_dist; 


	cur_pos.x += (x_distance);// dir*x_distance );
	cur_pos.y += (y_distance);// dir*y_distance );
}


void PosControl::setDriveSpeed(float straight_dist) {
	if(straight_dist > 100) {
		setSpeeds(SPEED_MED_POS, SPEED_MED_POS);
	} 
	else if(straight_dist > 0) {
		setSpeeds(SPEED_SLOW_POS, SPEED_SLOW_POS);
	}
	else if(straight_dist < -100) {
		setSpeeds(SPEED_MED_NEG, SPEED_MED_NEG);
	}
	else if(straight_dist < 0) {
		setSpeeds(SPEED_SLOW_NEG, SPEED_SLOW_NEG);
	} 
	else {
		setSpeeds(SPEED_STOP, SPEED_STOP);
	}
}


// check if speed is the same, to avoid clogging communication
void PosControl::setSpeeds(int l, int r) {
	if(left_motor.speed != l) {
		mcom->setSpeedL(l);
	}
	if(right_motor.speed != r) {
		mcom->setSpeedL(l);
	}	
}


float PosControl::shortestRotation(float angle, float goal) {
	float dist_left = (goal >= angle)? (goal - angle) : ((360 - angle) + goal);
	float dist_right = (goal >= angle)? -(angle + (360-goal)) : (goal - angle); 
	return (abs(dist_left) < abs(dist_right))? dist_left : dist_right;
}



void PosControl::readEncoders() {
	long left_enc = mcom->getEncL();
	long right_enc = mcom->getEncR();
	
	auto now = std::chrono::high_resolution_clock::now();
	auto timespan = std::chrono::duration<double, std::milli>(now - encoder_timestamp).count(); //left and right timestamps are the same. should perhaps do 2

	long left_diff = left_enc - left_encoder.prev;
	long right_diff = right_enc - right_encoder.prev;

	float left_diff_dist = left_diff * DIST_PER_TICK;
	float right_diff_dist = left_diff * DIST_PER_TICK;


	left_encoder.prev = left_enc;
	left_encoder.diff = left_diff;
	left_encoder.diff_dist = left_diff_dist;
	left_encoder.speed = left_diff_dist / timespan;

	right_encoder.prev = right_enc;
	right_encoder.diff = right_diff;
	right_encoder.diff_dist = right_diff_dist;
	right_encoder.speed = right_diff_dist / timespan;

	encoder_timestamp = now;
}



float PosControl::distStraight(float angle, float distX, float distY) {
	return (sin_d(angle) == 0)? distX : (distY/sin_d(angle));
}


bool PosControl::angleCloseEnough() {
	return (abs(cur_pos.angle) - abs(goal_pos.angle)) < ROTATION_CLOSE_ENOUGH;
}

bool PosControl::xCloseEnough() {
	LOG(INFO) << "[POS] " << cur_pos.x << " - " << goal_pos.x << "<"  << POSITION_CLOSE_ENOUGH;
	return abs(abs(cur_pos.x) - abs(goal_pos.x)) < POSITION_CLOSE_ENOUGH;
}

bool PosControl::yCloseEnough() {
	LOG(INFO) << "[POS] " << cur_pos.y << " - " << goal_pos.y << "<"  << POSITION_CLOSE_ENOUGH;
	return abs(abs(cur_pos.y) - abs(goal_pos.y)) < POSITION_CLOSE_ENOUGH;
}

bool PosControl::positionCloseEnough() {
	return xCloseEnough() && yCloseEnough();
}

bool PosControl::inGoal() {
	return positionCloseEnough() && angleCloseEnough();
}

int PosControl::getCurrentId() {
	return 0;//cur_pos.id;
}

std::string PosControl::getCurrentPos() {
//	cur_pos->updatePosString();
	std::stringstream ss;
	if(working) {
		ss << "w,";
	}
	else {
		ss << "s,";
	}
//	ss << cur_pos->getPosString();
	return ss.str();
}


int PosControl::getLiftPos() {
	return dcom->liftPosition();
}


bool PosControl::running() {
	return pos_running;
}

void PosControl::halt() {
	setSpeeds(SPEED_STOP, SPEED_STOP);
}

void PosControl::completeCurrent() {

}

void PosControl::setCurrent(float x, float y, float angle) {
	goal_pos.x = x;
	goal_pos.y = y;
	goal_pos.angle = angle;
}


float PosControl::sin_d(float angle) {
	return sin(angle*M_PI/180);
}


float PosControl::cos_d(float angle) {
	return cos(angle*M_PI/180);
}