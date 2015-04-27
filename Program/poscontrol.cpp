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

float ENC_PER_DEGREE = 8.05;

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




PosControl::PosControl(MotorCom *m, DynaCom *d, bool test, float e_p_d) {
	mcom = m;
	dcom = d;
	testing = test;

	if(e_p_d > 0.0 && e_p_d < 10.0) {
		ENC_PER_DEGREE = e_p_d;
	}

	reset(0,0,0);
}


void PosControl::reset(int x, int y, int rot) {
	cur_pos.x = x;
	cur_pos.y = y;
	cur_pos.angle = rot;
	goal_pos.x = x;
	goal_pos.y = y;
	goal_pos.angle = rot;

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
	
	left_motor.speed = SPEED_STOP;
	right_motor.speed = SPEED_STOP;

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
				//straightLoop(-1);
			} else if(cmd.type == STRAIGHT) {
				straightLoop();
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
				goal_pos.angle = cmd.rot;
				cur_pos.angle = goal_pos.angle;
			} else if(cmd.type == FORWARD) {
				goal_pos.x = cmd.x;
				goal_pos.y = cmd.y;
				goal_pos.angle = atan2Adjusted(goal_pos.x - cur_pos.x, goal_pos.y - cur_pos.y);
				setCurrent(goal_pos.x, goal_pos.y, goal_pos.angle);
			} else if(cmd.type == REVERSE) {
				//setCurrent(goal_pos.x, goal_pos.y, cur_pos.angle);

			}
			else if(cmd.type == STRAIGHT) {
				setCurrent(goal_pos.x, goal_pos.y, cur_pos.angle);
			} 
			else if(cmd.type == LIFT) {
				dcom->performAction(cmd.argument);
			}
			completeCurrent();
		}
		//logTrace();
	}
	pos_running = false;
}



void PosControl::rotationLoop() {
	LOG(INFO) << "[POS]  rotation: " << cur_pos.angle << " -> " << goal_pos.angle;
	
	float angle_err = shortestRotation(cur_pos.angle, goal_pos.angle);
	float total_rotation = angle_err;

	float turned = 0.0;

	while(turned < abs(total_rotation)) {
		//float compass_err = shortestRotation(compass.angle, goal_pos.angle);
		//logic to compare angle_err to compass_err

		angle_err = shortestRotation(cur_pos.angle, goal_pos.angle);
		setRotationSpeed(angle_err);

		readEncoders();
		float turned_now = updateAngle();
		turned += fabs(turned_now);
		
//		LOG(INFO) << "[POS] rotating:  " << cur_pos.angle-turned << " -> " << cur_pos.angle << " . Goal=" << goal_pos.angle << " err=" << angle_err;

		LOG(INFO) << "[POS] rotating:  (" << turned << " / " << total_rotation << ")   angle: " 
				<< cur_pos.angle << " goal: " << goal_pos.angle << "  error: " << angle_err << "  this: " << turned_now;

		//calculate turning speed
		usleep(3000);
	}

	usleep(100000);
	LOG(INFO) << "[POS] Rotation completed. Goal=" << goal_pos.angle << " cur_pos: " << cur_pos.angle << " Compass: " << compass.angle;	
	cur_pos.angle = goal_pos.angle;
	LOG(INFO) << "[POS] cur_pos.angle corrected to: " << goal_pos.angle;
}


//should compare with compass
//should attempt to match speed to hit target exactly
float PosControl::updateAngle() {
	long diffL = left_encoder.diff; 
	long diffR = right_encoder.diff;
  	// Average of encoders for reduced maximum-error

  	long enc_avg = abs(abs(diffL) + abs(diffR))/2;
	long enc_diff = abs(abs(diffL) - abs(diffR));

	if(enc_diff > 100) {  //MAX_ENC_TURN_DIFF) {
		LOG(INFO) << "[POS] encoder diff too large (" << enc_diff << "), checking compass";
		//difference between encoders, scrap odometry and rely on compass
		auto now = std::chrono::high_resolution_clock::now();
		auto time_since_compass = std::chrono::duration<double, std::milli>(now - compass_timestamp).count();
		if(time_since_compass < 2000) {
			cur_pos.angle = compass.angle;
			LOG(INFO) << "[POS] current angle set to compass angle: " << compass.angle;
			return 0.0;
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
	return turned;
}


//dir negative means backwards. Else forwards
void PosControl::straightLoop() {
	float angle = cur_pos.angle;
	float dist = goal_pos.x;

	goal_pos.x = cur_pos.x + cos_d(angle) * dist;
	goal_pos.y = cur_pos.y + sin_d(angle) * dist;
	goal_pos.angle = angle;

	positionLoop();
}



void PosControl::positionLoop() {
	if(inGoal()) {
		LOG(INFO) << "[POS] already in goal: (" << cur_pos.x << ", " << cur_pos.y << ") ~= (" << goal_pos.x << ", " << goal_pos.y << ")";
		return;
	}

	float dist_x = goal_pos.x - cur_pos.x; 
	float dist_y = goal_pos.y - cur_pos.y;
	float angle = atan2Adjusted(dist_x, dist_y); 
	goal_pos.angle = angle;
	float total_dist = distStraight(angle, dist_x, dist_y);

	if(!angleCloseEnough()) {
		rotationLoop();
	} 
	// float compass_angle = compass.angle;
	// check if matching

	float straight = 0.0;
	float dist_traveled = 0.0;

	while(dist_traveled < total_dist) {
		dist_x = goal_pos.x - cur_pos.x;
		dist_y = goal_pos.y - cur_pos.y;
		straight = distStraight(angle, dist_x, dist_y);

		//find vector towards target
		angle = atan2(dist_y, dist_x) *(180/M_PI);
		if(angle < 0) angle += 360;

		//TODO: check with compass
		//		define angle max offset
		if(abs(cur_pos.angle - angle) > 1) {
			//attempt to calculate necessary speed differences
			LOG(INFO) << "[POS] angle has drifted, attempting to compensate. (TODO)";

			//find angle difference
			//find distance left
			//calculate speed difference to compensate angle over that distance
		} else {
			setDriveSpeed(straight);
		}

		usleep(5000); //needs tweaking


		readEncoders();
		float traveled = updatePosition();
		if(traveled < 0) {
			PRINTLINE("NEGATIVE!!!!");
			traveled = abs(traveled);
		}
		dist_traveled += traveled;

		LOG_EVERY_N(5, INFO) << "[POS] driving:  (" << dist_traveled << "/" << total_dist 
			<< ")  x=" << cur_pos.x << " y=" << cur_pos.y << " -> (" << goal_pos.x << ", " 
			<< goal_pos.y << ").  Speed=(" << left_encoder.speed << ", " 
			<< right_encoder.speed << " )   traveled: " << traveled;
		//TODO: check absolute position
	} 

	usleep(10000);
	LOG(INFO) << "[POS] position reached . Goal_pos=(" << goal_pos.x << "," << goal_pos.y << "," << goal_pos.angle << ") cur_pos=(" << cur_pos.x << "," << cur_pos.y << "," << cur_pos.angle << ")";	
}


//angle has already been checked when this is called, assume 100% correct.
float PosControl::updatePosition() {
	long diff = abs(left_encoder.diff_dist - right_encoder.diff_dist);

	//MAX_DIFF
	if(diff > 100) {
		LOG(WARNING) << "[POS] large distance difference, probably spinning or hitting something.";
	}

	float angle = cur_pos.angle;
	float avg_dist = (abs(left_encoder.diff_dist + right_encoder.diff_dist)) / 2;
	float x_distance = cos_d(angle) * avg_dist; 
	float y_distance = sin_d(angle) * avg_dist; 
	cur_pos.x += (x_distance);// dir*x_distance );
	cur_pos.y += (y_distance);// dir*y_distance );

	return avg_dist;
}


float PosControl::updatePositionReverse() {
	long diff = abs(left_encoder.diff_dist - right_encoder.diff_dist);

	//MAX_DIFF
	if(diff > 100) {
		LOG(WARNING) << "[POS] large distance difference, probably spinning or hitting something.";
	}

	float angle = cur_pos.angle;
	float avg_dist = (abs(left_encoder.diff_dist + right_encoder.diff_dist)) / 2;
	float x_distance = cos_d(angle) * avg_dist; 
	float y_distance = sin_d(angle) * avg_dist; 
	cur_pos.x -= (x_distance);// dir*x_distance );
	cur_pos.y -= (y_distance);// dir*y_distance );

	return avg_dist;
}


//TODO: dynamically change speeds according to distance left. Start off slow
void PosControl::setRotationSpeed(float angle_err) {
	if(angle_err > 30) {
		setSpeeds(SPEED_MED_POS, SPEED_MED_NEG);
	}
	else if(angle_err > 0) {
		setSpeeds(SPEED_MED_POS, SPEED_MED_NEG);
	}
	else if(angle_err < -30) {
		setSpeeds(SPEED_MED_NEG, SPEED_MED_POS);
	}
	else if(angle_err < 0) {
		setSpeeds(SPEED_MED_NEG, SPEED_MED_POS);
	} 
	else {	
		setSpeeds(SPEED_STOP, SPEED_STOP);
	}
}


//TODO: dynamically change speeds according to distance left. Start off slow
void PosControl::setDriveSpeed(float straight_dist) {
	if(abs(straight_dist) < 10000) {
		setSpeeds(SPEED_MAX_POS, SPEED_MAX_POS);
	} 
	else if(abs(straight_dist) > 0) {
		setSpeeds(SPEED_SLOW_POS, SPEED_SLOW_POS);
	}
	else if(straight_dist < -100) {
		setSpeeds(SPEED_MED_NEG, SPEED_MED_NEG);
	}
	else if(straight_dist < 0) {
		setSpeeds(SPEED_SLOW_NEG, SPEED_SLOW_NEG);
	} 
	else {
		PRINTLINE("IN HERE");
		setSpeeds(SPEED_STOP, SPEED_STOP);
	}
}


// check if speed is the same, to avoid clogging communication
void PosControl::setSpeeds(int l, int r) {
	//if(left_motor.speed != l) {
		mcom->setSpeedL(l);
	//	left_motor.speed = l;
	//}
	//if(right_motor.speed != r) {
		mcom->setSpeedR(r);
	//	right_motor.speed = r;
	//}	
}


float PosControl::shortestRotation(float angle, float goal) {
	float dist_left = (goal >= angle)? (goal - angle) : ((360 - angle) + goal);
	float dist_right = (goal >= angle)? -(angle + (360-goal)) : (goal - angle); 
	return (abs(dist_left) < abs(dist_right))? dist_left : dist_right;
}


//encoders are opposite? but still okay
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
	left_encoder.speed = -left_diff_dist / timespan;

//	LOG(INFO) << " prev= " << left_encoder.prev << " diff=" << left_encoder.diff << " dist=" << left_encoder.diff_dist;

	right_encoder.prev = right_enc;
	right_encoder.diff = right_diff;
	right_encoder.diff_dist = right_diff_dist;
	right_encoder.speed = -right_diff_dist / timespan;

	encoder_timestamp = now;
}



float PosControl::distStraight(float angle, float x, float y) {
	float sin_a = sin_d(angle);
	float cos_a = cos_d(angle);
	float straight = 0.0;

	if(sin_a != 0) {
		straight = y / sin_a;
	} else if(cos_a != 0) {
		straight = x / cos_a;
	}
	return straight;
}


bool PosControl::angleCloseEnough() {
	return abs(cur_pos.angle - goal_pos.angle) < ROTATION_CLOSE_ENOUGH;
}

bool PosControl::xCloseEnough() {
	return abs(cur_pos.x - goal_pos.x) < POSITION_CLOSE_ENOUGH;
}

bool PosControl::yCloseEnough() {
	return abs(cur_pos.y - goal_pos.y) < POSITION_CLOSE_ENOUGH;
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


//TODO: test thread safety
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
	ss << cur_pos.x << "," << cur_pos.y << "," << cur_pos.angle;
	return ss.str();
}


//TODO: test thread safety
std::string PosControl::getState() {
	std::stringstream ss;
	ss << cur_pos.x << "," << cur_pos.y << "," << cur_pos.angle << "," << getSpeed();
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
	LOG(INFO) << "[POS] current action completed. TODO: id";
	halt();
	//mcom->resetEncoders();
	usleep(1000000);
	readEncoders();
	usleep(10000);
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

float PosControl::atan2Adjusted(float x, float y) {
	float angle = atan2(y, x) *(180/M_PI);
	return (angle < 0)? angle + 360 : angle;
}

float PosControl::atan2AdjustedReverse(float x, float y) {
	float angle = atan2Adjusted(x, y);
	angle += 180;
	if(angle > 360) angle -= 360;
	if(angle == 360) angle = 0;	
	return angle;
}

//speed in current heading in m/s
float PosControl::getSpeed() {
	float ls = left_encoder.speed;
	float rs = right_encoder.speed;

	if( (ls / rs) < 0 ) {
		//negative means speed in opposite directions, i.e. rotation
		return 0.0;
	}
	return (rs + ls) / 2;
}