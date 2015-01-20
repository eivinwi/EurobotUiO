#include "poscontrol.h"
/*TODO: 	
 	- use IMU for angle untill beacon system is workinh
 	- should continually check angle instead of once!! (impossible with only encoders)
	- pooling wait has poor performance (?)

	- Overshoot protection
*/

struct encoder {
	long prev;
	long diff;
	float diffDist;
	long total;
	float totalDist;
} leftEncoder, rightEncoder;


struct qPos {
	int id;
	int x;
	int y;
	float rot;
	int argument;
	int type;
};


PosControl::PosControl(MotorCom *m, LiftCom *l, bool test) {
	mcom = m;
	lcom = l;
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
	itr = 0;

	std::fill(std::begin(completed_actions), std::end(completed_actions), false);
	while(!q.empty()) {
		q.pop();
	}
}


bool PosControl::test() {
	enqueue(0, 1, 2, 3, 4, NONE);
	qPos test = dequeue();
	return (test.id == 0 && test.x == 1 && test.y == 2 && test.rot == 3 && test.argument == 4 && test.type == NONE);
}


void PosControl::enqueue(int id, int x, int y, float rot, int arg, int type) {
	qPos qp = {id, x, y, rot, arg, type};
	std::lock_guard<std::mutex> lock(qMutex);	
	q.push(qp);
	notifier.notify_one();
}


qPos PosControl::dequeue() {
	std::unique_lock<std::mutex> lock(qMutex);
	while(q.empty()) {
		notifier.wait(lock); //alternative implementation try_unlock with crono ms-timeout
	}

	qPos qp = q.front();
	q.pop();
	return qp;
}


//CHECK: should check if position is in goal?
void PosControl::setGoalRotation(int rot) {
	if(rot == 360) rot = 0;
	goalPos->setAngle(rot);
	goToRotation();
}


//CHECK: should check rotation? (probably not)
// sets the new goal pos. 360* == 0*
void PosControl::setGoalPosition(int x, int y) {
	LOG(DEBUG) << "[POS] setGoalPos(" << x << "," << y << ")";
	goalPos->setPosition(x, y);
	goToPosition();
}


//runs in its own thread, initialized in main.cpp
void PosControl::controlLoop() {
	pos_running = true;
	while(true) {
		qPos qp = dequeue();
		working = true;

		//create goalPos from qp
		goalPos = new GoalPosition(qp.id, qp.x, qp.y, qp.rot);
		if(!testing) {
			LOG(DEBUG) << "[POS] dequeued qPos {" << qp.id << "," << qp.x << "," << qp.y << "," << qp.rot << "," << qp.type << "}";

			if(qp.type == ROTATION) {
				goToRotation();
			} else if(qp.type == POSITION) {
				goToPosition();
			} else if(qp.type == LIFT) {
				lcom->goTo(qp.argument);
				completeCurrent();
			}
		} 
		else {
			if(qp.type == ROTATION) {
				curPos->setAngle(goalPos->getAngle());
				completeCurrent();
			} else if(qp.type == POSITION) {
				curPos->set(goalPos->getX(), goalPos->getY(), curPos->getAngle());
				completeCurrent();
			} else if(qp.type == LIFT) {
				lcom->setCurrentPos(qp.argument);
				completeCurrent();
			}
		}
		logTrace();
	}
	pos_running = false;
}


void PosControl::goToRotation() {
	LOG(DEBUG) << "[POS] goToRotation " << curPos->getAngle() << "->" << goalPos->getAngle();
	resetEncoders(); //here??	 
	float distR = 0.0;
//	bool rotated = false;
//	if(closeEnoughX() && closeEnoughY() && !closeEnoughAngle()) {
	if(!closeEnoughAngle()) {
		do {

			distR = distanceAngle();
			rotate(distR);
			updateRotation();
			logTrace();
			usleep(1000);
		} while(!closeEnoughAngle());
		completeCurrent();
		fullStop();		
		LOG(DEBUG) << "[POS] rotation finished: " << curPos->getAngle() << "=" << goalPos->getAngle();
	} 
	else {
		fullStop();
		completeCurrent();
		LOG(DEBUG) << "[POS] already at specified rotation(" << goalPos->getId() << "): " << curPos->getAngle() << "=" << goalPos->getAngle();
		usleep(3000);
	}
}


//CHECK: delays or not?
void PosControl::goToPosition() {
	LOG(DEBUG) << "[POS] goToPosition (" << curPos->getX() << "," << curPos->getY() << ") -> (" << goalPos->getX() << "," << goalPos->getY() << ")";

	resetEncoders(); //CHECK: here??
	float distX = distanceX(); 
	float distY = distanceY();
	float distR = 0.0;
	float angle = 0.0;
	float dist = 0.0;

	if(!inGoalPosition()) {
		//get angle we need to rotate to before driving
		angle = atan2(distY, distX) *(180/M_PI);
		if(angle < 0) angle = angle+360;
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
			//printCurrent();
			LOG_EVERY_N(5, INFO) << "(" << curPos->getX() << ", " << curPos->getY() << ", " << curPos->getAngle() << ")";

			if(!closeEnoughAngle()) {				
				LOG(DEBUG) << "[LOOP] ROTATION: " << distR;
				rotate(distR);
				updateRotation();
			} else {
				curPos->setAngle(goalPos->getAngle()); 
				LOG(DEBUG) << "[LOOP] DRIVE: " << distX << "," << distY;
				drive(dist);
				updatePosition();
			} 
			logTrace();
			usleep(2000);
		} while(!inGoal());
	}

	fullStop();
	completeCurrent();
	LOG(INFO) << "[POS] IN GOAL!  (" << curPos->getX() << " , " <<  curPos->getY() << ") ~= (" << goalPos->getX() << " , " << goalPos->getY() << ")";
	usleep(3000);
}


float PosControl::updateDist(float angle, float distX, float distY) {
	if(sin_d(angle) == 0) {
		return distX;
	} else {
		return distY/sin_d(angle);
	}
}


//TODO: get input from IMU
//need exact rotation to do small angle adjustments
//CHECK: could make turning more accurate
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



//CHECK: stuff
void PosControl::drive(float dist) {	
	LOG(DEBUG) << "DISTR: " << dist;
	float rotation = goalPos->getAngle();
	if(closeEnoughAngle()) {
		LOG(DEBUG) << "[POS] driving with rotation:" << rotation << " dist:" << dist;

		//reset encoders??
		//unneccsary test?
		if(!inGoal()) {
			if(dist < 0) {
				if(dist < -SLOWDOWN_MAX_DIST) {
					setSpeed(SPEED_MAX_NEG, SPEED_MAX_NEG);
				} else if(dist < -SLOWDOWN_MED_DIST) {
					setSpeed(SPEED_MED_NEG, SPEED_MED_NEG);					
				} else {
					setSpeed(SPEED_SLOW_NEG, SPEED_SLOW_NEG);
				}
			}
			else {
				if(dist > SLOWDOWN_MAX_DIST) {
					setSpeed(SPEED_MAX_POS, SPEED_MAX_POS);
				} else if(dist > SLOWDOWN_MED_DIST) {
					setSpeed(SPEED_MED_POS, SPEED_MED_POS);
				} else {
					setSpeed(SPEED_SLOW_POS, SPEED_SLOW_POS);
				}
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
	working = false;
	if(completed_actions[goalPos->getId()]) {
		LOG(INFO) << "[POS] action " << goalPos->getId() << " was already finished.";
	} else {
		LOG(INFO) << "[POS] action " << goalPos->getId() << " completed.";	
		completed_actions[goalPos->getId()] = true;
	}
	usleep(2000);
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
	LOG(TRACE) << goalPos->getId() << "," << curPos->getX() << "," << curPos->getY() << "," << curPos->getAngle();
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
	
	float angle = goalPos->getAngle();

	int ediff = encoderDifference();
	if(ediff > 50) {
		LOG(WARNING) << "[POS] Warning, large encoder difference(drive): " << ediff;
		return;
	}

	LOG(DEBUG) << "L: " << leftEncoder.diffDist << "  R: " << rightEncoder.diffDist;
	float avg_dist = (abs(leftEncoder.diffDist) + abs(rightEncoder.diffDist)) / 2;
	
	float x_distance = cos_d(angle)*avg_dist; //45 = +
	float y_distance = sin_d(angle)*avg_dist; //45 = +


	curPos->updateX( x_distance );
	curPos->updateY( y_distance );
}


void PosControl::updateRotation() {
	updateLeftEncoder();
	updateRightEncoder();

	//curX and curY should not really be updated
	// update angle
	curPos->updateAngle(leftEncoder.diff, rightEncoder.diff);
}


void PosControl::updateEncoder(long e, struct encoder *enc) {
	long diff = e - enc->prev;
	long absDiff = abs(diff);
	float distance = diff*ENCODER_CONSTANT;

	if(absDiff > REASONABLE_ENC_DIFF) {
		//TODO: reset encoders while taking care of values in a controlled manner
		LOG(WARNING) << "Unreasonable encoder value.";
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
	rightEncoder.prev = 0;
	rightEncoder.diff = 0;
	rightEncoder.diffDist = 0;	
	mcom->resetEncoders();
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
	return dist;
}

//CHECK: unused?
float PosControl::average(long a, long b) {
	//TODO: negative values!!!
	return (a - b)/2; 
}


bool PosControl::closeEnoughEnc(long a, long b) {
	return ((abs(a) - abs(b)) < 20);
}


bool PosControl::closeEnoughX() {
	return (abs(distanceX()) < POSITION_CLOSE_ENOUGH);
}


bool PosControl::closeEnoughY() {
	return (abs(distanceY()) < POSITION_CLOSE_ENOUGH);
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


//CHECK: unused?
void PosControl::printDist() {
	PRINTLINE("[POS] dist: " << distanceX() << "," << distanceY() << "," << distanceAngle());
}


void PosControl::setSpeed(int l, int r) {
	if(curSpeedLeft != l) {
		mcom->setSpeedL(l);
		curSpeedLeft = l;
	} 
	if(curSpeedRight != r) {
		mcom->setSpeedR(r);
		curSpeedRight = r;
	}
}


float PosControl::sin_d(float angle) {
	return sin(angle*M_PI/180);
}


float PosControl::cos_d(float angle) {
	return cos(angle*M_PI/180);
}