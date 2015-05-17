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


auto encoder_timestamp = std::chrono::high_resolution_clock::now();
auto compass_timestamp = std::chrono::high_resolution_clock::now();
auto prev = std::chrono::high_resolution_clock::now();



PosControl::PosControl(MotorCom *m, DynaCom *d, bool test, std::string config_file) {
	mcom = m;
	dcom = d;
	testing = test;
	readConfig(config_file);
	reset(0,0,0);
}


void PosControl::reset(float x, float y, float angle) {
	LOG(INFO) << "[POS] RESET to: (" << x << "," << y << "," << angle << ")"; 
	exact_pos = (Exact) {x, y, angle, std::chrono::high_resolution_clock::now()};
	cur_pos = (Pos) {x, y, angle};
	goal_pos = (Pos) {x, y, angle};
	apr_pos = (Pos) {x, y, angle};
	pos_0 = (Pos) {0.0, 0.0, 0.0};
	pos_diff = (Pos) {0.0, 0.0, 0.0};
	left_encoder = (Encoder) {0, 0, 0, 0.0, 0, 0.0, 0.0};
	right_encoder = (Encoder) {0, 0, 0, 0.0, 0, 0.0, 0.0};

	/*
	exact_pos.x = x;
	exact_pos.y = y;
	exact_pos.angle = angle;
	cur_pos.x = x;
	cur_pos.y = y;
	cur_pos.angle = rot;
	goal_pos.x = x;
	goal_pos.y = y;
	goal_pos.angle = rot;
	apr_pos.x = x;
	apr_pos.y = y;
	apr_pos.angle = rot;
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
	right_encoder.speed = 0.0;*/

	left_motor.speed = speed_stop;
	right_motor.speed = speed_stop;
	encoder_timestamp = std::chrono::high_resolution_clock::now();
	compass_timestamp = std::chrono::high_resolution_clock::now();
	compass.angle = 0.0;
	mcom->resetEncoders();
	clearQueue();
	std::fill(std::begin(completed_actions), std::end(completed_actions), false);
	while(!q.empty()) {
		q.pop();
	}
}


bool PosControl::test() {
	//TODO
	//enqueue(0, 1, 2, 3, 4, NONE);
	//Cmd test = dequeue();
	//return (test.id == 0 && test.x == 1 && test.y == 2 && test.rot == 3 && test.argument == 4 && test.type == NONE);
	return true;
}


void PosControl::enqueue(std::vector<int> arr) {
	std::lock_guard<std::mutex> lock(qMutex);
	q.push(arr);
	notifier.notify_one();
}


std::vector<int> PosControl::dequeue() {
	std::unique_lock<std::mutex> lock(qMutex);
	while(q.empty()) {
		notifier.wait(lock); //alternative implementation: try_unlock with crono ms-timeout
	}
	std::vector<int> cmd = q.front();
	q.pop();
	return cmd;
}


void PosControl::clearQueue() {
	std::queue <std::vector<int>> newQ;
	q = newQ;
}



// Main program, loops once every time a command is dequeued.
// If the testing-variable is set to true, automatically sets action to completed,
// and sets current position (and other vars) to the goal
void PosControl::controlLoop() {
	pos_running = true;
	while(true) {
		std::vector<int> cmd = dequeue();
		working = true;

		if(cmd.size() < 2) {
			LOG(WARNING) << "[POS] dequeue invalid command";
		} 
		else {
			int args = cmd.size();
			int type = cmd[0];

			LOG(DEBUG) << "[POS] thread <" << std::this_thread::get_id() << "> dequeued Cmd {"<< cmd[0] << "," << cmd[1] << "," << cmd[2] << "}";
			if(!testing) {
				switch(type) {
					case 0:
						//reserved
						break;
					case 1:
						LOG(INFO) << "[POS] starting action: reverse (" << cmd[2] << "," << cmd[3] << ")";
						current_id = cmd[1];
						goal_pos.x = cmd[2];
						goal_pos.y = cmd[3];
						reverseLoop();
						break;
					case 2:
						LOG(INFO) << "[POS] starting action: forward (" << cmd[2] << "," << cmd[3] << ")";
						current_id = cmd[1];
						goal_pos.x = cmd[2];
						goal_pos.y = cmd[3];
						positionLoop(false);
						break;
					case 3:
						LOG(INFO) << "[POS] starting action: rotation (" << cmd[2];
						current_id = cmd[1];
						goal_pos.angle = cmd[2];
						rotationLoop();
						break;
					case 4:
						//deprecated gripper,shutter,lift
						current_id = cmd[1];
						dcom->performAction(cmd);
						break;
					case 5:
						//gripper
						current_id = cmd[1];
						dcom->performAction(cmd);
						break;
					case 6:
						LOG(INFO) << "[POS] starting action: goal (" << cmd[2] << "," << cmd[3] << ")";
						current_id = cmd[1];
						goal_pos.x = cmd[2];
						goal_pos.y = cmd[3];
						positionLoop(true);
						break;					
					case 7:
						current_id = cmd[1];
						straightLoop(cmd[2]);
						break;					
					case 8:
						halt();
						break;
					case 9:
						if(args > 3) reset(cmd[1], cmd[2], cmd[3]);
						break;
					default:
						LOG(WARNING) << "[POS] invalid command: " << type;
						break;
				}

				halt();
			} 
			else {
				switch(type) {
					case 1:
						//reverse
						break;
					case 2:
						goal_pos.x = cmd[2];
						goal_pos.y = cmd[3];
						goal_pos.angle = atan2Adjusted(goal_pos.x - cur_pos.x, goal_pos.y - cur_pos.y);
						cur_pos = (Pos) {goal_pos.x, goal_pos.y, goal_pos.angle}
						break;
					case 3: 
						goal_pos.angle = cmd[2];
						cur_pos.angle = goal_pos.angle;
						break;
					case 4:
					case 5:
						dcom->performAction(cmd);
						break;
					case 7:
						cur_pos = (Pos) {cmd[2], cmd[3], cur_pos.angle}
						break;
					case 9:
						if(args > 3) reset(cmd[1], cmd[2], cmd[3]);					
						break;
					default:
						break;
				}
			}

			completeCurrent();
		}
	}
	pos_running = false;
}


void PosControl::rotationLoop() {
	LOG(INFO) << "[POS]  rotation: " << cur_pos.angle << " -> " << goal_pos.angle;

	readEncoders();
	pos_0.angle = cur_pos.angle;
	pos_diff.angle = 0.0;
	left_encoder.e_0 = left_encoder.total;
	right_encoder.e_0 = right_encoder.total;

	float angle_err = shortestRotation(cur_pos.angle, goal_pos.angle);
	if(abs(angle_err) < 10) {
		crawlToRotation();
		return;
	}

	float total_rotation = angle_err;
	float rotated = 0.0;

	while( (abs(rotated) < (abs(total_rotation) - CloseEnough.rotation)) ) {
		cur_pos.angle = (pos_0.angle + pos_diff.angle);

		angle_err = shortestRotation(cur_pos.angle, goal_pos.angle);
		setRotationSpeed(angle_err);

		usleep(TimeStep.rotation);

		readEncoders();
		rotated = updateAngle();

		LOG_EVERY_N(10, INFO) << "[POS] rotating:  (" << rotated << " / " << total_rotation << ")   angle: " 
				<< cur_pos.angle << " goal: " << goal_pos.angle << "  error: " << angle_err;
	}

	halt();
	LOG(INFO) << "\nRotation completed: " << cur_pos.angle << " ~= " << goal_pos.angle << " distance turned: " << rotated;

	readEncoders();
	updateAngle();
	float angle_complete = cur_pos.angle;
	long left_enc_complete = left_encoder.total;
	long right_enc_complete = right_encoder.total;

	usleep(TimeStep.move_complete);
	readEncoders();
	updateAngle();
	float angle_final = cur_pos.angle;
	long left_enc_final = left_encoder.total;
	long right_enc_final = right_encoder.total;

	printf("       	| pos_0.angle  |  a_diff  |   0+d    |   goal   |    completed    |     final     | \n");
 	printf("angle:  | %8.3f | %8.3f | %8.3f | %8.3f | %8.3f(%4.1f%%) | %8.3f(%4.1f%%) |\n\n", pos_0.angle, pos_diff.angle, (pos_0.angle + pos_diff.angle), goal_pos.angle, angle_complete, percPos(pos_0.angle, angle_complete, goal_pos.angle), angle_final, percPos(pos_0.angle, angle_final, goal_pos.angle));
	printf("       |  e_0   | compl  |  final | diff | dist | \n");
	printf("left:  | %6ld | %6ld | %6ld | %6ld | %6f \n", left_encoder.e_0, left_enc_complete, left_enc_final, (left_encoder.total - left_encoder.e_0), (left_encoder.total - left_encoder.e_0) / Enc.per_degree);
	printf("right: | %6ld | %6ld | %6ld | %6ld | %6f \n\n", right_encoder.e_0, right_enc_complete, right_enc_final, (right_encoder.total - right_encoder.e_0), (right_encoder.total - right_encoder.e_0) / Enc.per_degree);

	apr_pos.angle = goal_pos.angle;
}



//TODO: check compass and/or absolute
void PosControl::positionLoop(bool shouldOpen) {
	if(inGoal()) {
		LOG(INFO) << "[POS] Already at specified goal position: (" << cur_pos.x << "," << cur_pos.y << ") ~= (" << goal_pos.x << "," << goal_pos.y << ")";
		return;
	}

/*	pos_0.x = cur_pos.x;
	pos_0.y = cur_pos.y;
	pos_0.angle = cur_pos.angle;
	diff.x = 0.0;
	diff.y = 0.0;
	diff.angle = 0.0;*/
	pos_0 = (Pos) {cur_pos.x, cur_pos.y, cur_pos.angle};
	pos_diff = (Pos) {0.0, 0.0, 0.0};

	float dist_x = goal_pos.x - cur_pos.x; 
	float dist_y = goal_pos.y - cur_pos.y;
	float angle = atan2Adjusted(dist_x, dist_y); 
	goal_pos.angle = angle;
	if(!angleCloseEnough()) {
		rotationLoop();
	} 
	angle = cur_pos.angle;

	float total_dist = distStraight(angle, dist_x, dist_y);
	float straight = 0.0;
	float traveled = 0.0;

	readEncoders();
	left_encoder.e_0 = left_encoder.total;
	right_encoder.e_0 = right_encoder.total;

	bool open = false;
	if((fabs(straight) < 400) && !open && shouldOpen) {
		dcom->openGrippersNoSleep();
		usleep(500000); 
		open = true;
	}

	while( fabs(total_dist - traveled) > CloseEnough.position ) {
		cur_pos.x = pos_0.x + pos_diff.x;
		cur_pos.y = pos_0.y + pos_diff.y;
		straight = distStraight(angle, (goal_pos.x - cur_pos.x), (goal_pos.y - cur_pos.y));

		if((fabs(straight) < 400) && !open) {
			dcom->openGrippersNoSleep();
			open = true;
		}

		setDriveSpeed(straight, traveled);
		usleep(TimeStep.position);
		readEncoders();
		traveled = updatePosition(true);
		LOG_EVERY_N(10, INFO) << "[POS] driving:  (" << traveled << "/" << total_dist << ")  cx=" << cur_pos.x << " cy=" << cur_pos.y << " ca=" << cur_pos.angle; 
	}

	halt();
	readEncoders();
	updatePosition(true);
	float x_complete = cur_pos.x;
	float y_complete = cur_pos.y;
	long left_enc_complete = left_encoder.total;
	long right_enc_complete = right_encoder.total;
	usleep(TimeStep.move_complete);
	float x_final = cur_pos.x;
	float y_final = cur_pos.y;
	long left_enc_final = left_encoder.total;
	long right_enc_final = right_encoder.total;

	PRINTLINE("\nPOSITION completed: (" << pos_0.x << "," << pos_0.y << ") -> (" << goal_pos.x << "," << goal_pos.y << ").  Distance traveled: " << traveled);
	printf("       |   pos_0.x  |  diff.x  |  0+d  | goal  | com(perc) | fin(perc) | \n");
	PRINTLINE("-----------------------------------------------------------------------------------------------------------------");
	printf("x:     | %8.3f | %8.3f | %8.3f | %8.3f | %8.3f(%3.3f) | %8.3f(%3.3f) \n", pos_0.x, pos_diff.x, pos_0.x + pos_diff.x, goal_pos.x, x_complete, percPos(pos_0.x, x_complete, goal_pos.x), x_final, percPos(pos_0.x, x_final, goal_pos.x));
	printf("y:     | %8.3f | %8.3f | %8.3f | %8.3f | %8.3f(%3.3f) | %8.3f(%3.3f) \n\n", pos_0.y, pos_diff.y, pos_0.y + pos_diff.y, goal_pos.y, y_complete, percPos(pos_0.y, y_complete, goal_pos.y), y_final, percPos(pos_0.y, y_final, goal_pos.y));
	printf("left:  | %8ld  | %8ld  | %8ld  | %8d  | %8ld (?)     | %8ld (?)   \n\n", 
		left_encoder.e_0, (left_encoder.total-left_encoder.e_0), left_encoder.total,  0, left_enc_complete, left_enc_final);
	printf("right:  | %8ld  | %8ld  | %8ld  | %8d  | %8ld (?)     | %8ld (?)   \n\n", 
		right_encoder.e_0, (right_encoder.total-right_encoder.e_0), right_encoder.total,  0, right_enc_complete, right_enc_final);

	apr_pos.x = goal_pos.x;
	apr_pos.y = goal_pos.y;
}


//TODO: rewrite to x/y
void PosControl::reverseLoop() {
/*	pos_0.x = cur_pos.x;
	pos_0.y = cur_pos.y;
	pos_0.angle = cur_pos.angle;
	pos_diff.x = 0.0;
	pos_diff.y = 0.0;
	pos_diff.angle = 0.0;
*/	
	pos_0 = (Pos) {cur_pos.x, cur_pos.y, cur_pos.angle};
	pos_diff = (Pos) {0.0, 0.0, 0.0};

	float dist_x = goal_pos.x - cur_pos.x; 
	float dist_y = goal_pos.y - cur_pos.y;
	float angle = atan2AdjustedReverse(dist_x, dist_y);
	goal_pos.angle = angle;
	if(!angleCloseEnough()) {
		rotationLoop();
	} 
	angle = cur_pos.angle;

	float total_dist = distStraight(angle, dist_x, dist_y);
	float straight = 0.0;
	float traveled = 0.0;

	readEncoders();
	left_encoder.e_0 = left_encoder.total;
	right_encoder.e_0 = right_encoder.total;

	while( fabs(fabs(total_dist) - fabs(traveled)) > CloseEnough.position ) {
		straight = distStraight(angle, dist_x, dist_y);
		straight = total_dist - traveled;
		setDriveSpeed(straight, traveled);
		usleep(TimeStep.position);
		readEncoders();
		traveled = updatePosition(false);
		LOG_EVERY_N(10, INFO) << "[POS] reversing:  (" << traveled << "/" << total_dist << ") "; 
	}

	halt();
	readEncoders();
	updatePosition(false);
	LOG(INFO) << "[POS] reverse move complete, cur_pos is now:  (" << cur_pos.x << "," << cur_pos.y << ")"; 

	apr_pos.x = goal_pos.x;
	apr_pos.y = goal_pos.y;
}


//dir negative means backwards. Else forwards
//check: should probably work for negative dist
void PosControl::straightLoop(int dist) {
	goal_pos.x = cur_pos.x + ( cos_d(cur_pos.angle) * dist );
	goal_pos.y = cur_pos.y + ( sin_d(cur_pos.angle) * dist );
	goal_pos.angle = cur_pos.angle;
	if(dist > 0) {
		positionLoop(false);
	} else {
		reverseLoop();
	}
}


//TODO: test overshoot fix
void PosControl::crawlToRotation() {
	float start_angle = cur_pos.angle;
	float angle_err = shortestRotation(cur_pos.angle, goal_pos.angle);
	float total_rotation = fabs(angle_err);
	float traveled = 0.0;

	while( (total_rotation - fabs(traveled) > 0.05) ) { 
		angle_err = shortestRotation(cur_pos.angle, goal_pos.angle);
		if(angle_err > 0) {
			setSpeeds(speed_rot.pos_crawl, speed_rot.neg_crawl);
		} 
		else if(angle_err < 0) {
			setSpeeds(speed_rot.neg_crawl, speed_rot.pos_crawl);			
		}
		else {
			setSpeeds(128, 128);
		}
		usleep(TimeStep.crawling);
		readEncoders();
		traveled = updateAngle();
		LOG_EVERY_N(10, INFO) << "[POS] crawling:  " << cur_pos.angle << " goal: " << goal_pos.angle << "  error: " << angle_err;
	}

	halt();
	usleep(TimeStep.move_complete);
	LOG(INFO) << "[POS] crawl complete.  from=" << start_angle << " to=" << cur_pos.angle << " goal=" << goal_pos.angle << " %%=" << perc(start_angle, cur_pos.angle, goal_pos.angle);
	apr_pos.angle = goal_pos.angle;
}



//TODO: incorrect if distance is larger than goal 
float PosControl::perc(float s, float a, float g) {
	float total = fabs(shortestRotation(s, g));
	float remaining = fabs(shortestRotation(a, g));
	return (total == 0)? 100.0 : ((total - remaining)/total * 100.0);
}


//TODO: incorrect if distance is longer than goal
float PosControl::percPos(float s, float c, float g) {
	float total = (g - s);
	float remaining = (g - c);
	return (total == 0)? 100.0 : ((total - remaining)/total * 100.0);
}


float PosControl::updateAngle() {
	long diffL = left_encoder.diff; 
	long diffR = right_encoder.diff;

	//MAdiff.x
	if(abs(abs(diffL) - abs(diffR)) > 100) {
		LOG(WARNING) << "[POS] rotation: large distance difference, probably spinning or hitting something.";
	}

	float left_dist = (left_encoder.total - left_encoder.e_0) / Enc.per_degree; //in degrees
	float right_dist = (right_encoder.total - right_encoder.e_0) / Enc.per_degree; //in degrees
	float avg_dist = (fabs(left_dist) + fabs(right_dist)) / 2;

	//if CCW
	if(left_dist < 0) avg_dist = -avg_dist;

	pos_diff.angle = avg_dist;
	cur_pos.angle = pos_0.angle + pos_diff.angle;
	if(cur_pos.angle >= 360.0) cur_pos.angle -= 360.0;
	else if(cur_pos.angle < 0.0) cur_pos.angle += 360.0;
	
	return avg_dist;
}



//angle has already been checked when this is called, assume 100% correct.
float PosControl::updatePosition(bool forward) {
	long diff = abs(left_encoder.diff_dist - right_encoder.diff_dist);

	//MAdiff.x
	if(diff > 100) {
		LOG(WARNING) << "[POS] large distance difference, probably spinning or hitting something.";
	}

	float left_dist = (left_encoder.total - left_encoder.e_0) * Enc.constant;
	float right_dist = (right_encoder.total - right_encoder.e_0) * Enc.constant;
	float avg_dist = (fabs(left_dist) + fabs(right_dist)) / 2;
	if(!forward) avg_dist = -avg_dist;

	pos_diff.x = cos_d(cur_pos.angle) * avg_dist; 
	pos_diff.y = sin_d(cur_pos.angle) * avg_dist; 
	cur_pos.x = pos_0.x + pos_diff.x;
	cur_pos.y = pos_0.y + pos_diff.y;
	return avg_dist;
}


//TODO: dynamically change speeds according to distance left. Start off slow
//positive = CW
void PosControl::setRotationSpeed(float angle_err) {
	if(angle_err > 0) {
		if(angle_err > Slowdown.rot_max) {
			setSpeeds(speed_rot.pos_med, speed_rot.neg_med);
		} else if(angle_err > Slowdown.rot_med) {
			setSpeeds(speed_rot.pos_med, speed_rot.neg_med);		
		} else if(angle_err > Slowdown.rot_slow) {
			setSpeeds(speed_rot.pos_slow, speed_rot.neg_slow);
		} else {
			setSpeeds(speed_rot.pos_crawl, speed_rot.neg_crawl);			
		}
	}
	else if(angle_err < 0) {
		if(angle_err < -Slowdown.rot_max) {
			setSpeeds(speed_rot.neg_med, speed_rot.pos_med);
		} else if(angle_err < -Slowdown.rot_med) {
			setSpeeds(speed_rot.neg_med, speed_rot.pos_med);		
		} else if(angle_err < -Slowdown.rot_slow) {
			setSpeeds(speed_rot.neg_slow, speed_rot.pos_slow);
		} else {
			setSpeeds(speed_rot.neg_crawl, speed_rot.pos_crawl);			
		}
	}
	else {	
		setSpeeds(speed_stop, speed_stop);
	}
}


//TODO: dynamically change speeds according to distance left. Start off slow
void PosControl::setDriveSpeed(float straight_dist, float traveled) {
	float t = fabs(traveled);
	if(straight_dist > 0) {
		if(straight_dist > Slowdown.straight_max && t > Slowdown.startup_max) {
			setSpeeds(speed_pos.pos_fast, speed_pos.pos_fast);
		} 
		else if(straight_dist > Slowdown.straight_med && t > Slowdown.startup_med) {
			setSpeeds(speed_pos.pos_med, speed_pos.pos_med);
		} else {
			setSpeeds(speed_pos.pos_slow, speed_pos.pos_slow);
		}
	} 
	else if(straight_dist < 0) {
		if(straight_dist < -Slowdown.straight_max && t > Slowdown.startup_med) {
			setSpeeds(speed_pos.neg_med, speed_pos.neg_med);
		}
		else {
			setSpeeds(speed_pos.neg_slow, speed_pos.neg_slow);
		} 
	} 
	else {
		setSpeeds(speed_stop, speed_stop);
	}
}


// check if speed is the same, to avoid clogging communication
void PosControl::setSpeeds(int l, int r) {
	auto now = std::chrono::high_resolution_clock::now();
	auto timespan = std::chrono::duration<double, std::milli>(now - prev).count();

	if(timespan > timeout_guard) {
		mcom->setSpeedR(r);
		mcom->setSpeedL(l);
		right_motor.speed = r;
		left_motor.speed = l;
		prev = now;
	} else {
		if(right_motor.speed != r) {
			mcom->setSpeedR(r);
			right_motor.speed = r;
		}
		if(left_motor.speed != l) {
			mcom->setSpeedL(l);
			left_motor.speed = l;
		}
	}
}


//positive = CW
float PosControl::shortestRotation(float angle, float goal) {
	float dist_cw = (goal >= angle)? (goal - angle) : ((360 - angle) + goal);
	float dist_ccw = (goal >= angle)? (-angle - (360 - goal)) : (angle - goal);
	if(dist_ccw > 0) dist_ccw /= -1;
	return (abs(dist_ccw) <= abs(dist_cw))? dist_ccw : dist_cw;
}


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
	float left_diff_dist = left_diff * Enc.constant; //dist_per_tick;
	float right_diff_dist = left_diff * Enc.constant; //dist_per_tick;

	left_encoder.prev = left_enc;
	left_encoder.diff = left_diff;
	left_encoder.diff_dist = left_diff_dist;
	left_encoder.speed = -left_diff_dist / timespan;
	left_encoder.total += left_diff;

	right_encoder.prev = right_enc;
	right_encoder.diff = right_diff;
	right_encoder.diff_dist = right_diff_dist;
	right_encoder.speed = -right_diff_dist / timespan;
	right_encoder.total += right_diff;
	encoder_timestamp = now;

	LOG(DEBUG) << " prev= " << left_encoder.prev << " diff=" << left_encoder.diff << " dist=" << left_encoder.diff_dist;
	//printError();
}


float PosControl::distStraight(float angle, float x, float y) {
	float sin_a = sin_d(angle);
	float cos_a = cos_d(angle);
	float straight = 0.0;

	if(fabs(sin_a) >= fabs(cos_a)) {
		straight = (fabs(y) > 0.001)? (y/sin_a) : x;
	} else {
		straight = (fabs(x) > 0.001) ? (x/cos_a) : y;
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


std::string PosControl::getPosStr() {
	std::stringstream ss;
	ss << cur_pos.x << "," << cur_pos.y << "," << cur_pos.angle;
	return ss.str();
}

std::string PosControl::getState() {
	std::stringstream ss;
	ss << current_id << "," << completed(current_id) << "," << getPosStr();
	//ss << apr_pos.x << "," << apr_pos.y << "," << apr_pos.angle;
	return ss.str();
}


int PosControl::completed(int id) {
	return ((completed_actions[id])? 1 : 0);
}


std::string PosControl::getGripperPos() {
	return dcom->getGripperPosition();
}


bool PosControl::running() {
	return pos_running;
}


void PosControl::halt() {
	setSpeeds(speed_stop, speed_stop);
}


void PosControl::completeCurrent() {
	LOG(INFO) << "[POS] action " << current_id << " completed.";
	completed_actions[current_id] = true;
}


float PosControl::sin_d(float angle) {
	float r = sin(angle * M_PI/180);
	return (fabs(r) < 0.001)? 0.0 : r;
}


float PosControl::cos_d(float angle) {
	float r = cos(angle * M_PI/180);
	return (fabs(r) < 0.001)? 0.0 : r;
}


float PosControl::atan2Adjusted(float x, float y) {
	float angle = atan2(y, x) *(180/M_PI);
	return (angle < 0)? angle + 360 : angle;
}


float PosControl::atan2AdjustedReverse(float x, float y) {
	float angle = atan2Adjusted(x, y) + 180;
	if(angle > 360) angle -= 360;
	return (angle == 360)? 0 : angle;
}


//speed in current heading, in m/s
float PosControl::getSpeed() {
	float ls = left_encoder.speed;
	float rs = right_encoder.speed;

	if( (ls / rs) < 0 ) {
		//negative means wheels speeds are in opposite directions, i.e. rotation
		return 0.0;
	}
	return (ls + rs) / 2;
}


//TODO: check if reasonable
void PosControl::setexact_posPos(std::vector<float> v) {
	if(v.size() == 3) {
		float x = v[0];
		float y = v[1];
		float angle = v[2];
		if((x > 3000) || (x < 0) || (y > 2000) || (y < 0) || (angle > 360) || angle < 0)  {
			LOG(WARNING) << "exact_pospos is outside area: (" << x << "," << y << "," << angle;
			return;
		}
		exact_pos = (Pos) {v[0], v[1], v[2], std::chrono::high_resolution_clock::now()};
		LOG(INFO) << "exact_pospos=(" << x << "," << y << "," << angle;
	}
	else {
		LOG(WARNING) << "[POS] vector from POS too short: " << v.size();
	}
}


void PosControl::printError() {
	uint8_t err = mcom->getError();
	if(err != 0x00) {
		std::bitset<8>b1(err);
		LOG(WARNING) << "MD49 error: " << (int) err << " = " << b1; 
	}
}


void PosControl::readConfig(std::string filename) {
	YAML::Node config;
	if(filename == "") {
		LOG(INFO) << "[POS] reading configuration file: config.yaml";
		config = YAML::LoadFile("config.yaml");
	}
	else {
		LOG(INFO) << "[POS] reading configuration file:" << filename << ".";
		config = YAML::LoadFile(filename);		
	}

	speed_stop = config["speed_stop"].as<int>();

	int pos_max = config["pos_max_speed"].as<int>();
	int pos_med = config["pos_med_speed"].as<int>();
	int pos_slow = config["pos_slow_speed"].as<int>();
	int pos_crawl = config["pos_crawl_speed"].as<int>();

	int rot_max = config["rot_max_speed"].as<int>();
	int rot_med = config["rot_med_speed"].as<int>();
	int rot_slow = config["rot_slow_speed"].as<int>();
	int rot_crawl = config["rot_crawl_speed"].as<int>();

	int stop = speed_stop;
	speed_pos = {stop+pos_max, stop+pos_med, stop+pos_slow, stop+pos_crawl, stop-pos_max, stop-pos_med, stop-pos_slow, stop-pos_crawl};
	speed_rot = {stop+rot_max, stop+rot_med, stop+rot_slow, stop+rot_crawl, stop-rot_max, stop-rot_med, stop-rot_slow, stop-rot_crawl};

	Slowdown.dist_max = config["slowdown_dist_max"].as<int>();
	Slowdown.dist_med = config["slowdown_dist_med"].as<int>();
	Slowdown.rot_max = config["slowdown_rot_max"].as<int>();
	Slowdown.rot_med = config["slowdown_rot_med"].as<int>();
	Slowdown.rot_slow = config["slowdown_rot_slow"].as<int>();
	Slowdown.startup_max = config["slowdown_startup_max"].as<int>();
	Slowdown.startup_med = config["slowdown_startup_med"].as<int>();

	CloseEnough.rotation = 	config["rotation_close_enough"].as<float>();
	CloseEnough.position = config["position_close_enough"].as<float>();

	Enc.max_incr = config["max_enc_incr"].as<int>();
	Enc.madiff.x = config["max_enc_diff"].as<int>();
	Enc.constant = config["encoder_constant"].as<float>();
	Enc.dist_per_tick = config["dist_per_tick"].as<float>();
	Enc.per_degree = config["enc_per_degree"].as<float>();

	MAX_WAIT = config["max_wait"].as<int>();
}


/*
//TODO: check compass and/or absolute
void PosControl::pickUpLoop() {
	if(inGoal()) {
		LOG(INFO) << "[POS] Already at specified goal position: (" << cur_pos.x << "," << cur_pos.y << ") ~= (" << goal_pos.x << "," << goal_pos.y << ")";
		return;
	}

	pos_0.x = cur_pos.x;
	pos_0.y = cur_pos.y;
	pos_0.angle = cur_pos.angle;
	diff.x = 0.0;
	diff.y = 0.0;
	diff.angle = 0.0;

	float dist_x = goal_pos.x - cur_pos.x; 
	float dist_y = goal_pos.y - cur_pos.y;
	float angle = atan2Adjusted(dist_x, dist_y); 
	goal_pos.angle = angle;
	if(!angleCloseEnough()) {
		rotationLoop();
	} 
	angle = cur_pos.angle;

	float total_dist = distStraight(angle, dist_x, dist_y);
	float straight = 0.0;
	float traveled = 0.0;

	readEncoders();
	left_encoder.e_0 = left_encoder.total;
	right_encoder.e_0 = right_encoder.total;

	bool open = false;
	if((fabs(straight) < 400) && !open) {
		dcom->openGrippersNoSleep();
		usleep(500000); 
		open = true;
	}

	while( fabs(total_dist - traveled) > CloseEnough.position ) {
		cur_pos.x = pos_0.x + diff.x;
		cur_pos.y = pos_0.y + diff.y;
		straight = distStraight(angle, (goal_pos.x - cur_pos.x), (goal_pos.y - cur_pos.y));

		if((fabs(straight) < 400) && !open) {
			dcom->openGrippersNoSleep();
			open = true;
		}

		setDriveSpeed(straight, traveled);
		usleep(TimeStep.position);
		readEncoders();
		traveled = updatePosition(true);
		LOG_EVERY_N(10, INFO) << "[POS] driving(pickup):  (" << traveled << "/" << total_dist << ") "; 
	}

	halt();
	readEncoders();
	updatePosition(true);
	float x_complete = cur_pos.x;
	float y_complete = cur_pos.y;
	usleep(TimeStep.move_complete);
	float x_final = cur_pos.x;
	float y_final = cur_pos.y;

	PRINTLINE("\nPOSITION completed: (" << pos_0.x << "," << pos_0.y << ") -> (" << goal_pos.x << "," << goal_pos.y << ").  Distance traveled: " << traveled);
	printf("       |   pos_0.x  |  diff.x  |  0+d  | goal  | com(perc) | fin(perc) | \n");
	PRINTLINE("-----------------------------------------------------------------------------------------------------------------");
	printf("x:     | %8.3f | %8.3f | %8.3f | %8.3f | %8.3f(%3.3f) | %8.3f(%3.3f) \n", pos_0.x, diff.x, pos_0.x + diff.x, goal_pos.x, x_complete, percPos(pos_0.x, x_complete, goal_pos.x), x_final, percPos(pos_0.x, x_final, goal_pos.x));
	printf("y:     | %8.3f | %8.3f | %8.3f | %8.3f | %8.3f(%3.3f) | %8.3f(%3.3f) \n\n", pos_0.y, diff.y, pos_0.y + diff.y, goal_pos.y, y_complete, percPos(pos_0.y, y_complete, goal_pos.y), y_final, percPos(pos_0.y, y_final, goal_pos.y));
	printf("left:  | %5ld | %5ld | %5ld | %5f \n", left_encoder.e_0, left_encoder.total, (left_encoder.total - left_encoder.e_0), (left_encoder.total - left_encoder.e_0)*Enc.constant);
	printf("right: | %5ld | %5ld | %5ld | %5f \n\n", right_encoder.e_0, right_encoder.total, (right_encoder.total - right_encoder.e_0), (right_encoder.total - right_encoder.e_0)*Enc.constant);

	LOG(INFO) << "Adjusting position from (" << cur_pos.x << "," << cur_pos.y << ") to (" << goal_pos.x << "," << goal_pos.y << ")";
	apr_pos.x = goal_pos.x;
	apr_pos.y = goal_pos.y;
}
*/
/*

//TODO: rewrite to x/y
void PosControl::reverseStraight(int dist) {
	goal_pos.x = cur_pos.x + ( cos_d(cur_pos.angle) * dist );
	goal_pos.y = cur_pos.y + ( sin_d(cur_pos.angle) * dist );
	goal_pos.angle = cur_pos.angle;

	float angle = cur_pos.angle;
	float dist_x = goal_pos.x - cur_pos.x; 
	float dist_y = goal_pos.y - cur_pos.y;
	float total_dist = distStraight(angle, dist_x, dist_y);
	float straight = 0.0;
	float traveled = 0.0;

	readEncoders();
	pos_0.x = cur_pos.x;
	pos_0.y = cur_pos.y;
	pos_0.angle = cur_pos.angle;
	diff.x = 0.0;
	diff.y = 0.0;
	diff.angle = 0.0;
	left_encoder.e_0 = left_encoder.total;
	right_encoder.e_0 = right_encoder.total;

	while( fabs(fabs(total_dist) - fabs(traveled)) > CloseEnough.position ) {
		straight = distStraight(angle, dist_x, dist_y);
		straight = total_dist - traveled;
		setDriveSpeed(straight, traveled);
		usleep(TimeStep.position);
		readEncoders();
		traveled = updatePosition(false);
		LOG_EVERY_N(10, INFO) << "[POS] reversing:  (" << traveled << "/" << total_dist << ") "; 
	}

	halt();
	readEncoders();
	updatePosition(false);
	LOG(INFO) << "[POS] reverse move complete, cur_pos is now:  (" << cur_pos.x << "," << cur_pos.y << ")"; 
	apr_pos.x = goal_pos.x;
	apr_pos.y = goal_pos.y;
}

float PosControl::updatePositionReverse() {
	long diff = abs(left_encoder.diff_dist - right_encoder.diff_dist);

	if(diff > 100) {
		LOG(WARNING) << "[POS] large distance difference, probably spinning or hitting something.";
	}

	float left_dist = (left_encoder.total - left_encoder.e_0) * Enc.constant;
	float right_dist = (left_encoder.total - left_encoder.e_0) * Enc.constant;
	float avg_dist = -(fabs(left_dist + right_dist)) / 2;

	diff.x = cos_d(cur_pos.angle) * avg_dist;
	diff.y = sin_d(cur_pos.angle) * avg_dist;
	cur_pos.x = pos_0.x + diff.x;
	cur_pos.y = pos_0.y + diff.y;
	return avg_dist;
}

*/