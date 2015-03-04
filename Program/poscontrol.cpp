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
// 	- use IMU for angle untill beacon system is workinh
// 	- should continually check angle instead of once!! (impossible with only encoders)
//
//	- Overshoot protection

struct encoder {
	long prev;
	long diff;
	float diffDist;
	long total;
	float totalDist;
} leftEncoder, rightEncoder;


struct Cmd {
	int id;
	int x;
	int y;
	float rot;
	int argument;
	int type;
};


PosControl::PosControl(MotorCom *m, LiftCom *l, DynaCom *d, bool test) {
	mcom = m;
	lcom = l;
	dcom = d;
	testing = test;
	goalPos = new GoalPosition;
	curPos = new Position;
	exactPos = new Position;
	reset();
}


PosControl::~PosControl() {
}


void PosControl::reset() {
	curPos->reset();
	goalPos->reset();
	
	leftEncoder.prev = 0;
	leftEncoder.diff = 0;
	leftEncoder.diffDist = 0.0;
	leftEncoder.total = 0;
	leftEncoder.totalDist = 0.0;

	rightEncoder.prev = 0;
	rightEncoder.diff = 0;
	rightEncoder.diffDist = 0.0;
	rightEncoder.total = 0;
	rightEncoder.totalDist = 0.0;

	curSpeedRight = 0;
	curSpeedLeft = 0;
	curPos->updatePosString();

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
	Cmd  cmd= {id, x, y, rot, arg, type};
	std::lock_guard<std::mutex> lock(qMutex);	
	q.push(cmd);
	notifier.notify_one();
}


Cmd PosControl::dequeue() {
	std::unique_lock<std::mutex> lock(qMutex);
	while(q.empty()) {
		notifier.wait(lock); //alternative implementation try_unlock with crono ms-timeout
	}

	Cmd cmd = q.front();
	q.pop();
	usleep(100);
	return cmd;
}


//CHECK: should check if position is in goal?
void PosControl::setGoalRotation(int rot) {
	if(rot == 360) rot = 0;
	else if(rot < 0) rot += 360;

	goalPos->setAngle(rot);
	goToRotation();
}


//CHECK: should check rotation? (probably not) 
void PosControl::setGoalPosition(int x, int y) {
	LOG(DEBUG) << "[POS] setGoalPos(" << x << "," << y << ")";
	goalPos->setPosition(x, y);
	goToPosition();
}



// Main program, loops once every time a command is dequeued.
// If the testing-variable is set to true, automatically sets action to completed,
// and sets current position (and other vars) to the goal
void PosControl::controlLoop() {
	pos_running = true;
	while(true) {
		Cmd cmd = dequeue();
		working = true;

		goalPos = new GoalPosition(cmd.id, cmd.x, cmd.y, cmd.rot);
		LOG(DEBUG) << "[POS] [" << std::this_thread::get_id() << "]dequeued Cmd {" << cmd.id << "," << cmd.x << "," << cmd.y << "," << cmd.rot << "," << cmd.type << "}";
		
		if(!testing) {
			if(cmd.type == ROTATION) {
				goToRotation();
			} else if(cmd.type == FORWARD) {
				goToPosition();
			} else if(cmd.type == REVERSE) {
				goToReverse();
			} else if(cmd.type == LIFT) {
				goToLift(cmd.argument);	
			}
		} 
		else {
			if(cmd.type == ROTATION) {
				curPos->setAngle(goalPos->getAngle());
			} else if(cmd.type == FORWARD || cmd.type == REVERSE) {
				curPos->set(goalPos->getX(), goalPos->getY(), curPos->getAngle());
			} else if(cmd.type == LIFT) {
				lcom->setCurrentPos(cmd.argument);
			}
			completeCurrent();
		}
		logTrace();
	}
	pos_running = false;
}


void PosControl::goToRotation() {
	LOG(INFO) << "[POS] goToRotation " << curPos->getAngle() << "->" << goalPos->getAngle();
	float distR = 0.0;
	if(!closeEnoughAngle()) {
		do {

			distR = distanceAngle();
			rotate(distR);
			updateRotation();
			logTrace();
			usleep(1000);
		} while(!closeEnoughAngle());
		LOG(INFO) << "[POS] rotation finished: " << curPos->getAngle() << "=" << goalPos->getAngle();
	} 
	else {
		LOG(INFO) << "[POS] already at specified rotation(" << goalPos->getId() << "): " << curPos->getAngle() << "=" << goalPos->getAngle();
	}
	completeCurrent();
	//fullStop();
	curPos->setAngle(goalPos->getAngle());
}


//CHECK: delays or not?
void PosControl::goToPosition() {
	LOG(DEBUG) << "[POS] goToPosition (" << curPos->getX() << "," << curPos->getY() << ") -> (" << goalPos->getX() << "," << goalPos->getY() << ")";
	float distX = distanceX(); 
	float distY = distanceY();
	float distR = 0.0;
	float angle = 0.0;
	float dist = 0.0;


	if(!inGoalPosition()) {
		//get angle we need to rotate to before driving
		angle = atan2(distY, distX) *(180/M_PI);
		if(angle < 0) angle += 360;
		goalPos->setAngle(angle);
		LOG(DEBUG) << "[POS] CALCULATED ANGLE=" << angle;
		
		//calculate straight distance
		dist = updateDist(angle, distX, distY);
		
		do {
			distX = distanceX(); 
			distY = distanceY();
			distR = distanceAngle();
			dist = updateDist(angle, distX, distY);

			LOG(DEBUG) << "CURRENT: " << curPos->getX() << " | " << curPos->getY() << " | " << curPos->getAngle();
			LOG_EVERY_N(5, INFO) << "pos_now: (" << curPos->getX() << ", " << curPos->getY() << ", " << curPos->getAngle() << ")";

			if(!closeEnoughAngle()) {				
				LOG(INFO) << "[LOOP] ROTATION: " << distR;
				rotate(distR);
				updateRotation();
			} else {
				curPos->setAngle(goalPos->getAngle()); 
				LOG(INFO) << "[LOOP] DRIVE: " << distX << "," << distY << " => " << dist;
				drive(dist);
				updatePosition();
			} 
			logTrace();
			
			int error = (int) mcom->getError();
			if(error != 0) {
				LOG(INFO) << "||||||||||||||||||| ERROR: " << error << "||||||||||||||||";
			}	
			usleep(3000);


		} while(!closeEnoughX() || !closeEnoughY());
	}

	completeCurrent();
	fullStop();
	LOG(INFO) << "[POS] IN GOAL!  (" << curPos->getX() << " , " <<  curPos->getY() << ") ~= (" << goalPos->getX() << " , " << goalPos->getY() << ")";
}


void PosControl::goToReverse() {
	LOG(INFO) << "[POS] goToReverse (" << curPos->getX() << "," << curPos->getY() << ") -> (" << goalPos->getX() << "," << goalPos->getY() << ")";
	float distX = distanceX(); 
	float distY = distanceY();
	float distR = 0.0;
	float angle = 0.0;
	float dist = 0.0;

	if(!inGoalPosition()) {
		angle = atan2(distY, distX) *(180/M_PI);
		if(angle < 0) angle += 360;

		//could be more efficient..
		angle += 180;
		if(angle > 360) angle -= 360;
		if(angle == 360) angle = 0;	

		goalPos->setAngle(angle);
		LOG(DEBUG) << "[POS] CALCULATED ANGLE (reverse)=" << angle;

		distR = distanceAngle();
		
		while(!closeEnoughAngle()) {
		//	LOG(DEBUG) << "[LOOP] ROTATION: " << distR;
			rotate(distR);
			updateRotation();
		}
		do {
			distX = distanceX(); 
			distY = distanceY();
			distR = distanceAngle();
			dist = updateDistReverse(angle, distX, distY);

			curPos->setAngle(goalPos->getAngle()); 
		//	LOG(INFO) << "[LOOP] DRIVE: " << distX << "," << distY;
			drive_reverse(dist); // <- reverse
			//drive(dist);
			updatePositionReverse();
			LOG(INFO) << "distR: " << distR << " distX:" << distX << " distY:" << distY;
		} while(!inGoal());
	}

	completeCurrent();
	fullStop();
	LOG(INFO) << "[POS] IN GOAL (reverse)!  (" << curPos->getX() << " , " <<  curPos->getY() << ") ~= (" << goalPos->getX() << " , " << goalPos->getY() << ")";
}


//TODO: this is an ugly way of doing lift/grabber
void PosControl::goToLift(int arg) {
	PRINTLINE("[POS] goToLift");	
	if(arg == 3) {
		dcom->openGrip();
	} else if(arg == 4) {
		dcom->closeGrip();
	}

	else {
		lcom->goTo(arg);
		bool success = lcom->waitForResponse();
		PRINTLINE("[POS] goToLift done waiting");
		if(success) {
			LOG(INFO) << "[LIFT] Successful movement";
		} else {
			LOG(WARNING) << "[LIFT] UNSUCCESSFULL movement";
		}
	}
	completeCurrent();
}


float PosControl::updateDist(float angle, float distX, float distY) {
	if(sin_d(angle) == 0) {
		return distX;
	} else {
		return distY/sin_d(angle);
	}
}


float PosControl::updateDistReverse(float angle, float distX, float distY) {
	if(sin_d(angle) == 0) {
		return distX;
	} else {
		return distY/sin_d(angle);
	}
}


//TODO: get input from IMU
//need exact rotation to do small angle adjustments
void PosControl::rotate(float distR) {
	LOG(DEBUG) << "[POS] turning: ";
	if(distR == 0) {
		LOG(DEBUG) << "Error, turn is 0.";
	} else if(distR > 0) {
		LOG(DEBUG) << "Positive dir (" << distR << ")";
		if(distR > SLOWDOWN_DISTANCE_ROT) {
			setSpeed(SPEED_MED_POS, SPEED_MED_NEG);
		} else {
			setSpeed(SPEED_SLOW_POS, SPEED_SLOW_NEG);
		}
	} else {
		LOG(DEBUG) << "Negative dir (" << distR << ")";
		if(distR < -SLOWDOWN_DISTANCE_ROT) {
			setSpeed(SPEED_MED_NEG, SPEED_MED_POS);
		} else {
			setSpeed(SPEED_SLOW_NEG, SPEED_SLOW_POS);
		}
	}
}


//CHECK: should this at any point be negative??
// TOOD: NEEDS OVERSHOOT PROTECTION!!!!
void PosControl::drive(float dist) {	
	LOG(DEBUG) << "DISTR: " << dist;
	float rotation = goalPos->getAngle();
	if(closeEnoughAngle()) {
		//LOG(INFO) << "[POS] driving with rotation:" << rotation << " dist:" << dist;

		//CHECK: unneccsary test?
		if(!inGoal()) {
			/*if(dist < 0) {
				if(dist < -SLOWDOWN_MAX_DIST) {
					LOG(INFO) << "[D] max_neg (" << dist << ")";
					setSpeed(SPEED_MAX_NEG, SPEED_MAX_NEG);
				} else if(dist < -SLOWDOWN_MED_DIST) {
					LOG(INFO) << "[D] med_neg (" << dist << ")";
					setSpeed(SPEED_MED_NEG, SPEED_MED_NEG);					
				} else {
					LOG(INFO) << "[D] min_neg (" << dist << ")";
					setSpeed(SPEED_SLOW_NEG, SPEED_SLOW_NEG);
				}
			}
			else {*/
				if(abs(dist) > SLOWDOWN_MAX_DIST) {
					//LOG(INFO) << "[D] max_pos (" << dist << ")";
					setSpeed(SPEED_MAX_POS, SPEED_MAX_POS);
				} 
				else if(abs(dist) > SLOWDOWN_MED_DIST) {
					//LOG(INFO) << "[D] med_pos (" << dist << ")";
					setSpeed(SPEED_MED_POS, SPEED_MED_POS);
				} 
				else {
					//LOG(INFO) << "[D] min_pos (" << dist << ")";
					setSpeed(SPEED_SLOW_POS, SPEED_SLOW_POS);
				}
			//}
		} else {
			LOG(DEBUG) << "[POS] ERROR; already in goal";
		}
	} 
	else {
		LOG(ERROR) << "[POS] ERROR; cur->angle is" << rotation << " should be " << goalPos->getAngle();
	}
}


// must reverse slowly because of poor weight distribution on the robot
void PosControl::drive_reverse(float dist) {	
	LOG(DEBUG) << "DISTR: " << dist;
	float rotation = goalPos->getAngle();
	if(closeEnoughAngle()) {
		LOG(DEBUG) << "[POS] driving reverse with rotation:" << rotation << " dist:" << dist;

		//CHECK: unneccsary test?
		if(!inGoal()) {
			if(dist < 0) {
				setSpeed(SPEED_SLOW_NEG, SPEED_SLOW_NEG);
			}
			else {
				setSpeed(SPEED_SLOW_POS, SPEED_SLOW_POS);
			}
		} else {
			LOG(DEBUG) << "[POS] ERROR; already in goal";
		}
	} 
	else {
		LOG(ERROR) << "[POS] ERROR; cur->angle is" << rotation << " should be " << goalPos->getAngle();
	}
}


void PosControl::completeCurrent() {
	fullStop();
	LOG(DEBUG) << "THREAD [" << std::this_thread::get_id() << "] is sleeping";
	usleep(1000000);
	LOG(DEBUG) << "THREAD [" << std::this_thread::get_id() << "] is awake";
	working = false;
	if(completed_actions[goalPos->getId()]) {
		LOG(INFO) << "[POS] action " << goalPos->getId() << " was already finished.";
	} else {
		LOG(INFO) << "[POS] action " << goalPos->getId() << " completed.";	
		completed_actions[goalPos->getId()] = true;
	}
}


int PosControl::getCurrentId() {
	return goalPos->getId();
}


std::string PosControl::getCurrentPos() {
	curPos->updatePosString();
	std::stringstream ss;
	if(working) {
		ss << "w,";
	} else {
		ss << "s,";
	}
	ss << curPos->getPosString();
	return ss.str();
}


int PosControl::getLiftPos() {
	return lcom->getPosition();
}


bool PosControl::running() {
	return pos_running;
}



/*** PRIVATE FUNCTIONS: ***/

void PosControl::logTrace() {
	LOG(TRACE) << goalPos->getId() << "," << curPos->getPosString();;
}


/*
 * Angle calculation:
 * omkrets = 2*pi*r = 2*pi*(330/2)=pi*330 = 1036
 * mm per grad = 1036/360 = 2.88
 * enc per mm = 980/337 = 2.6
 * enc per grad = 2.88*2.6 = 7.5
 */
 // CHECK: is avg_dist optimal? probably not
void PosControl::updatePosition() {
	LOG(DEBUG) << "[POS] updatePos ";
	
	updateLeftEncoder();
	updateRightEncoder();	
	LOG(DEBUG) << "[POS] encoders updated";
	

	int ediff = encoderDifference();
	if(ediff > 50) {
		LOG(WARNING) << "[POS] Warning, large encoder difference(drive): " << ediff;
		resetEncoders();
		return;
	}

	float angle = goalPos->getAngle();

//	LOG(INFO) << "L: " << leftEncoder.diffDist << "  R: " << rightEncoder.diffDist;
//	LOG(INFO) << "Lt: " << leftEncoder.total << " Rt: " << rightEncoder.total;
	float avg_dist = (abs(leftEncoder.diffDist) + abs(rightEncoder.diffDist)) / 2;
	
	float x_distance = cos_d(angle)*avg_dist; //45 = +
	float y_distance = sin_d(angle)*avg_dist; //45 = +


	curPos->updateX( x_distance );
	curPos->updateY( y_distance );
}


void PosControl::updatePositionReverse() {
	LOG(DEBUG) << "[POS] updatePos ";
	
	updateLeftEncoder();
	updateRightEncoder();	
	LOG(DEBUG) << "[POS] encoders updated";
	
	float angle = goalPos->getAngle();

	int ediff = encoderDifference();
	if(ediff > 50) {
		LOG(WARNING) << "[POS] Warning, large encoder difference(drive): " << ediff;
		resetEncoders();
		return;
	}

//	LOG(INFO) << "L: " << leftEncoder.diffDist << "  R: " << rightEncoder.diffDist;
//	LOG(INFO) << "Lt: " << leftEncoder.total << " Rt: " << rightEncoder.total;
	float avg_dist = (abs(leftEncoder.diffDist) + abs(rightEncoder.diffDist)) / 2;
	
	float x_distance = cos_d(angle)*avg_dist; //45 = +
	float y_distance = sin_d(angle)*avg_dist; //45 = +


	curPos->updateX( -x_distance );
	curPos->updateY( -y_distance );
}


void PosControl::updateRotation() {
	updateLeftEncoder();
	updateRightEncoder();

//	LOG(INFO) << "L: " << leftEncoder.diffDist << "  R: " << rightEncoder.diffDist;
//	LOG(INFO) << "Lt: " << leftEncoder.total << " Rt: " << rightEncoder.total;

	curPos->updateAngle(leftEncoder.diff, rightEncoder.diff);
}


void PosControl::updateEncoder(long e, struct encoder *enc) {
	long diff = e - enc->prev;
	long absDiff = abs(diff);
	float distance = diff*ENCODER_CONSTANT;

	if(absDiff > REASONABLE_ENC_DIFF) {
		//TODO: reset encoders while taking care of values in a controlled manner
		LOG(WARNING) << "Unreasonable encoder value.";
		resetEncoders();
		return;
	}

	enc->prev = e;
	enc->diff = diff;
	enc->diffDist = distance;
	enc->total += diff;
	enc->totalDist += distance;

	LOG(DEBUG) << "enc: " << e << " diff: " << diff << " distance: " << distance;
	LOG(DEBUG) << "total: " << enc->total << " totalDist: " << enc->totalDist;
}


void PosControl::updateLeftEncoder() {
	long enc = mcom->getEncL();
	updateEncoder(enc, &leftEncoder);
}


void PosControl::updateRightEncoder() {
	long enc = mcom->getEncR();
	updateEncoder(enc, &rightEncoder);
}


void PosControl::resetEncoders() {
	leftEncoder.prev = 0;
	leftEncoder.diff = 0;
	leftEncoder.diffDist = 0;
	leftEncoder.total = 0;
	leftEncoder.totalDist = 0;
	rightEncoder.prev = 0;
	rightEncoder.diff = 0;
	rightEncoder.diffDist = 0;	
	rightEncoder.total = 0;
	rightEncoder.totalDist = 0;
	mcom->resetEncoders();
	usleep(10000);
}


void PosControl::fullStop() {
	LOG(DEBUG) << "[POS] Stopping";
	setSpeed(SPEED_STOP, SPEED_STOP);
}


/* >0 : goal is in positive x-direction
 * =0 : in exact x-coordinate
 * <0 : goal is in negative x-direction
 */
float PosControl::distanceX() {
	return (goalPos->getX() - curPos->getX());
}


/* >0 : goal is in positive x-direction
 * =0 : in exact x-coordinate
 * <0 : goal is in negative x-direction
 */
float PosControl::distanceY() {
	return (goalPos->getY() - curPos->getY());	
}


/* >0 : goal is in positive direction
 * =0 : at exact angle
 * <0 : goal angle in negative direction
 */
float PosControl::distanceAngle() {
	float dist = curPos->distanceRot(goalPos->getAngle());
	//PRINTLINE("distA calc:  " << curPos->getAngle() << "-" << goalPos->getAngle() << " = " << dist);
	return dist;
}


bool PosControl::closeEnoughX() {
	return (abs(distanceX()) < (POSITION_CLOSE_ENOUGH + 2.0)); // hack to fix erratic behavior! should investigate cause
}


bool PosControl::closeEnoughY() {
	return (abs(distanceY()) < (POSITION_CLOSE_ENOUGH + 2.0));
}


bool PosControl::closeEnoughAngle() {
	return (abs(distanceAngle()) < ROTATION_CLOSE_ENOUGH);
}


bool PosControl::inGoalPosition() {
	return closeEnoughX() && closeEnoughY();	
}


bool PosControl::inGoal() {
	return inGoalPosition() && closeEnoughAngle();
}


long PosControl::encoderDifference() {
	return (abs(leftEncoder.diff) - abs(rightEncoder.diff));
}


void PosControl::printCurrent() {
	LOG(INFO) << "(" << curPos->getX() << ", " << curPos->getY() << ", " << curPos->getAngle() << ")";
}


void PosControl::printGoal() {
	LOG(INFO) << "[POS] Goal: (" 
		<< goalPos->getX() << "," << goalPos->getY() << "," << goalPos->getAngle() << ")";
}


void PosControl::printDist() {
	PRINTLINE("[POS] dist: " << distanceX() << "," << distanceY() << "," << distanceAngle());
}


void PosControl::setSpeed(int l, int r) {
	LOG(INFO) << "SetSpeed( " << l << ", " << r << ")";
	//if(curSpeedLeft != l) {
		LOG(INFO) << "	speed left set=" << l; 
		mcom->setSpeedL(l);
		curSpeedLeft = l;
	//} 
	//if(curSpeedRight != r) {
		LOG(INFO) << "	speed right set=" << r; 
		mcom->setSpeedR(r);
		curSpeedRight = r;
	//}
}


float PosControl::sin_d(float angle) {
	return sin(angle*M_PI/180);
}


float PosControl::cos_d(float angle) {
	return cos(angle*M_PI/180);
}

void PosControl::printEncoder(struct encoder *e) {
	PRINTLINE("prev: " << e->prev);
	PRINTLINE("diff: " << e->diff);
	PRINTLINE("diffDist: " << e->diffDist);
	PRINTLINE("total: " << e->total);
	PRINTLINE("totalDist: " << e->totalDist);

	/*
	long prev;
	long diff;
	float diffDist;
	long total;
	float totalDist;
	*/
}