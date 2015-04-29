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




PosControl::PosControl(MotorCom *m, DynaCom *d, bool test, std::string config_file) {
	mcom = m;
	dcom = d;
	testing = test;
	readConfig(config_file);
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
	
	left_motor.speed = Speed.stop;
	right_motor.speed = Speed.stop;

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

	readEncoders();
	long left_enc_start = left_encoder.total;
	long right_enc_start = right_encoder.total;
	float angle_start = cur_pos.angle;

	while(abs(turned) < (abs(total_rotation) - CloseEnough.rotation)) {
		//float compass_err = shortestRotation(compass.angle, goal_pos.angle);
		//TODO: logic to compare angle_err to compass_err
		usleep(5000);

		angle_err = shortestRotation(cur_pos.angle, goal_pos.angle);
		setRotationSpeed(angle_err);

		readEncoders();
		float turned_now = updateAngle();
		turned += fabs(turned_now);
		LOG_EVERY_N(5, INFO) << "[POS] rotating:  (" << turned << " / " << total_rotation << ")   angle: " 
				<< cur_pos.angle << " goal: " << goal_pos.angle << "  error: " << angle_err << "  this: " << turned_now;
	}
	halt();
	long left_enc_complete = mcom->getEncL();
	long right_enc_complete = mcom->getEncR();
	float angle_complete = cur_pos.angle;

	usleep(100000);
	readEncoders();
	long left_enc_final = left_encoder.total;
	long right_enc_final = right_encoder.total;

	updateAngle();
	float angle_final = cur_pos.angle;

	PRINTLINE("ROTATION completed: " << angle_start << " -> " << goal_pos.angle << ".  Distance rotated: " << turned);
	printf("                   | %5s | %6s| %5s | %5s | %s |\n", "angle", "remain", "perc ", "encL ", "encR ");
	PRINTLINE("-----------------------------------------------------------------------------------------------------------------");
	printf(" rotation    start | %5f | %5f | %4f%% | %5ld | %5ld |\n", angle_start, shortestRotation(angle_start, goal_pos.angle) ,perc(angle_start, angle_start, goal_pos.angle), left_enc_start, right_enc_start);
	printf(" rotation complete | %5f | %5f | %4f%% | %5ld | %5ld |\n", angle_complete, shortestRotation(angle_complete, goal_pos.angle), perc(angle_start, angle_complete, goal_pos.angle), left_enc_complete, right_enc_complete);
	printf(" rotation    final | %5f | %5f | %4f%% | %5ld | %5ld |\n", angle_final, shortestRotation(angle_final, goal_pos.angle), perc(angle_start, angle_final, goal_pos.angle), left_enc_final, right_enc_final);
	PRINTLINE("-----------------------------------------------------------------------------------------------------------------");
}


float PosControl::perc(float s, float a, float g) {
	float total = shortestRotation(s, a);
	float remaining = shortestRotation(a, g);
	return (total/100) * remaining;
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

	//diffL positive means CW 
	float turned = (diffL > 0)? (enc_avg/Enc.per_degree) : -(enc_avg/Enc.per_degree);

	cur_pos.angle += turned;
	if(cur_pos.angle >= 360.0) {
		cur_pos.angle -= 360.0;
	} 
	else if(cur_pos.angle < 0.0) {
		cur_pos.angle += 360.0;
	}
	return turned;
}


void PosControl::crawlToRotation() {
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
	float x_start = cur_pos.x;
	float y_start = cur_pos.y;
	float angle_start = cur_pos.angle;
	long left_enc_start = left_encoder.total;
	long right_enc_start = right_encoder.total;

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
	halt();
	readEncoders();
	updatePosition();
	float x_complete = cur_pos.x;
	float y_complete = cur_pos.y;
	float angle_complete = cur_pos.angle;
	long left_enc_complete = left_encoder.total;
	long right_enc_complete = right_encoder.total;


	LOG(INFO) << "[POS] position reached . Goal_pos=(" << goal_pos.x << "," << goal_pos.y << "," << goal_pos.angle << ") cur_pos=(" << cur_pos.x << "," << cur_pos.y << "," << cur_pos.angle << ")";	
	usleep(100000);

	readEncoders();
	updatePosition();
	float x_final = cur_pos.x;
	float y_final = cur_pos.y;
	float angle_final = cur_pos.angle;
	long left_enc_final = left_encoder.total;
	long right_enc_final = right_encoder.total;



	PRINTLINE("POSITION completed: (" << x_start << "," << y_start << ") -> (" << goal_pos.x << "," << goal_pos.y << ").  Distance traveled: " << dist_traveled);
	printf("                   | %11s | %11s | %5s | %5s | %5s |\n", "x(compl)", " y(compl)", "angle", "encL ", "encR ");
	PRINTLINE("-----------------------------------------------------------------------------------------------------------------");
	printf("position    start | %5f (%3f%%) | %5f (%3f%%) | %4f  | %5ld | %5ld | \n", x_start, perc(x_start, x_start, goal_pos.x), y_start, perc(y_start, y_start, goal_pos.y), angle_start, left_enc_start, right_enc_start);
	printf("position complete | %5f (%3f%%) | %5f (%3f%%) | %4f  | %5ld | %5ld | \n", x_complete, perc(x_complete, x_complete, goal_pos.x), y_complete, perc(y_complete, y_complete, goal_pos.y), angle_complete, left_enc_complete, right_enc_complete);
	printf("position    final | %5f (%3f%%) | %5f (%3f%%) | %4f  | %5ld | %5ld | \n", x_final, perc(x_final, x_final, goal_pos.x), y_final, perc(y_final, y_final, goal_pos.y), angle_final, left_enc_final, right_enc_final);
	PRINTLINE("-----------------------------------------------------------------------------------------------------------------");
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
	//positive = CW
	// CW = left_pos + right_neg
	// CCW = left_neg + right_pos


	if(angle_err > 0) {
		if(angle_err > Slowdown.max_rot) {
			setSpeeds(Speed.pos_med, Speed.neg_med);
		} else if(angle_err > Slowdown.med_rot) {
			setSpeeds(Speed.pos_med, Speed.neg_med);		
		} else {
			setSpeeds(Speed.pos_slow, Speed.neg_slow);
		}
	}

	else if(angle_err < 0) {
		if(angle_err < -Slowdown.max_rot) {
			setSpeeds(Speed.neg_med, Speed.pos_med);
		} else if(angle_err < -Slowdown.med_rot) {
			setSpeeds(Speed.neg_med, Speed.pos_med);		
		} else {
			setSpeeds(Speed.neg_slow, Speed.pos_slow);
		} 
	}


	else {	
		setSpeeds(Speed.stop, Speed.stop);
	}
}


//TODO: dynamically change speeds according to distance left. Start off slow
void PosControl::setDriveSpeed(float straight_dist) {
	if(abs(straight_dist) > 200) {
		setSpeeds(Speed.pos_fast, Speed.pos_fast);
	} 
	else if(abs(straight_dist) > 100) {
		setSpeeds(Speed.pos_med, Speed.pos_med);
	} 
	else if(abs(straight_dist) > 0) {
		setSpeeds(Speed.pos_slow, Speed.pos_slow);
	}
	else if(straight_dist < -200) {
		setSpeeds(Speed.neg_med, Speed.neg_med);
	}
	else if(straight_dist < 0) {
		setSpeeds(Speed.neg_slow, Speed.neg_slow);
	} 
	else {
		PRINTLINE("IN HERE");
		setSpeeds(Speed.stop, Speed.stop);
	}
}


auto prev = std::chrono::high_resolution_clock::now();
int timeout_guard = 1000;
// check if speed is the same, to avoid clogging communication
void PosControl::setSpeeds(int l, int r) {
	auto now = std::chrono::high_resolution_clock::now();
	auto timespan = std::chrono::duration<double, std::milli>(now - prev).count();

	if(timespan > timeout_guard) {
		mcom->setSpeedL(l);
		mcom->setSpeedR(r);
	} else {
		if(left_motor.speed != l) {
			mcom->setSpeedL(l);
			left_motor.speed = l;
		}

		if(right_motor.speed != r) {
			mcom->setSpeedR(r);
			right_motor.speed = r;
		}	
	}
	prev = now;
}


//TOCHANGE slightly
float PosControl::shortestRotation(float angle, float goal) {
	float dist_cw = (goal >= angle)? (goal - angle) : ((360 - angle) + goal);
	float dist_ccw = (goal >= angle)? (-angle - (360 - goal)) : (angle - goal);
	if(dist_ccw > 0) dist_ccw /= -1;
	return (abs(dist_ccw) <= abs(dist_cw))? dist_ccw : dist_cw;
}
//positive = CW
//negative = CCW

//encoders are opposite? but still okay
void PosControl::readEncoders() {
	long left_enc = 0;
	long right_enc = 0;
	auto t =  mcom->getEncoders();
	left_enc = std::get<0>(t);
	right_enc = std::get<1>(t);


	auto now = std::chrono::high_resolution_clock::now();
	auto timespan = std::chrono::duration<double, std::milli>(now - encoder_timestamp).count(); //left and right timestamps are the same. should perhaps do 2

	long left_diff = left_enc - left_encoder.prev;
	long right_diff = right_enc - right_encoder.prev;

	float left_diff_dist = left_diff * Enc.dist_per_tick;
	float right_diff_dist = left_diff * Enc.dist_per_tick;

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


//TOCHANGE
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
	return abs(cur_pos.angle - goal_pos.angle) < CloseEnough.rotation;
}

bool PosControl::xCloseEnough() {
	return abs(cur_pos.x - goal_pos.x) < CloseEnough.position;
}

bool PosControl::yCloseEnough() {
	return abs(cur_pos.y - goal_pos.y) < CloseEnough.position;
}

bool PosControl::positionCloseEnough() {
	return xCloseEnough() && yCloseEnough();
}

bool PosControl::inGoal() {
	return positionCloseEnough() && angleCloseEnough();
}

//TODO
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
	setSpeeds(Speed.stop, Speed.stop);
}


void PosControl::completeCurrent() {
	LOG(INFO) << "[POS] current action completed. TODO: id";
	//mcom->resetEncoders();
	usleep(1000000);
}


void PosControl::setCurrent(float x, float y, float angle) {
	goal_pos.x = x;
	goal_pos.y = y;
	goal_pos.angle = angle;
}


float PosControl::sin_d(float angle) {
	return sin(angle * M_PI/180);
}


float PosControl::cos_d(float angle) {
	return cos(angle * M_PI/180);
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


void PosControl::readConfig(std::string filename) {
	YAML::Node config;
	if(filename == "") {
		LOG(INFO) << "[POS] reading configuration file: config.yaml";
		config = YAML::LoadFile("config.yaml");
	}
	else {
		LOG(INFO) << "[POS] reading configuration file: " << filename;
		config = YAML::LoadFile("filename");		
	}


	Speed.pos_slow = config["SPEED_SLOW_POS"].as<int>();
	Speed.pos_med = config["SPEED_MED_POS"].as<int>();
	Speed.pos_fast = config["SPEED_MAX_POS"].as<int>();
	Speed.neg_slow = config["SPEED_SLOW_NEG"].as<int>();
	Speed.neg_med = config["SPEED_MED_NEG"].as<int>();
	Speed.neg_fast = config["SPEED_MAX_NEG"].as<int>();
	Speed.stop = config["SPEED_STOP"].as<int>();

	Slowdown.max_dist = config["SLOWDOWN_MAX_DIST"].as<int>();
	Slowdown.med_dist = config["SLOWDOWN_MED_DIST"].as<int>();
	Slowdown.max_rot = config["SLOWDOWN_MAX_ROT"].as<int>();
	Slowdown.med_rot = config["SLOWDOWN_MED_ROT"].as<int>();

	CloseEnough.rotation = 	config["ROTATION_CLOSE_ENOUGH"].as<float>();
	CloseEnough.position = config["POSITION_CLOSE_ENOUGH"].as<float>();

	Enc.max_incr = config["MAX_ENC_INCR"].as<int>();
	Enc.max_diff = config["MAX_ENC_DIFF"].as<int>();
	Enc.constant = config["ENCODER_CONSTANT"].as<float>();
	Enc.dist_per_tick = config["DIST_PER_TICK"].as<float>();
	Enc.per_degree = config["ENC_PER_DEGREE"].as<float>();

	MAX_WAIT = config["MAX_WAIT"].as<int>();

}