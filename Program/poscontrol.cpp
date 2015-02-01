#include "poscontrol.h"
/*TODO: 	
 	- use IMU for angle untill beacon system is workinh
 	- should continually check angle instead of once!! (impossible with only encoders)

	- Overshoot protection
*/

struct encoder {
	long prev;
	long diff;
	double diffDist;
	long total;
	double totalDist;
} leftEncoder, rightEncoder;


struct Cmd {
	int id;
	int x;
	int y;
	double rot;
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


void PosControl::enqueue(int id, int x, int y, double rot, int arg, int type) {
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
	goalPos->setAngle(rot);
	goToRotation();
}


//CHECK: should check rotation? (probably not)
void PosControl::setGoalPosition(int x, int y) {
	LOG(DEBUG) << "[POS] setGoalPos(" << x << "," << y << ")";
	goalPos->setPosition(x, y);
	goToPosition();
}


void PosControl::controlLoop() {
	pos_running = true;
	while(true) {
		Cmd cmd = dequeue();
		working = true;

		goalPos = new GoalPosition(cmd.id, cmd.x, cmd.y, cmd.rot);
		LOG(INFO) << "[POS] [" << std::this_thread::get_id() << "]dequeued Cmd {" << cmd.id << "," << cmd.x << "," << cmd.y << "," << cmd.rot << "," << cmd.type << "}";
		
		if(!testing) {
			if(cmd.type == ROTATION) {
				goToRotation();
			} else if(cmd.type == POSITION) {
				goToPosition();
			} else if(cmd.type == LIFT) {
				goToLift(cmd.argument);	
			}
		} 
		else {
			if(cmd.type == ROTATION) {
				curPos->setAngle(goalPos->getAngle());
			} else if(cmd.type == POSITION) {
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
	LOG(DEBUG) << "[POS] goToRotation " << curPos->getAngle() << "->" << goalPos->getAngle();
	double distR = 0.0;
	if(!closeEnoughAngle()) {
		do {

			distR = distanceAngle();
			rotate(distR);
			updateRotation();
			logTrace();
			usleep(1000);
		} while(!closeEnoughAngle());
		fullStop();		
		LOG(DEBUG) << "[POS] rotation finished: " << curPos->getAngle() << "=" << goalPos->getAngle();
		completeCurrent();
	} 
	else {
		fullStop();
		LOG(DEBUG) << "[POS] already at specified rotation(" << goalPos->getId() << "): " << curPos->getAngle() << "=" << goalPos->getAngle();
		completeCurrent();
	}
}


//CHECK: delays or not?
void PosControl::goToPosition() {
	LOG(DEBUG) << "[POS] goToPosition (" << curPos->getX() << "," << curPos->getY() << ") -> (" << goalPos->getX() << "," << goalPos->getY() << ")";

	double distX = distanceX(); 
	double distY = distanceY();
	double distR = 0.0;
	double angle = 0.0;
	double dist = 0.0;

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
			usleep(3000);
		} while(!inGoal());
	}

	fullStop();
	LOG(INFO) << "[POS] IN GOAL!  (" << curPos->getX() << " , " <<  curPos->getY() << ") ~= (" << goalPos->getX() << " , " << goalPos->getY() << ")";
	completeCurrent();
}


void PosControl::goToLift(int arg) {
	lcom->goTo(arg);
	bool success = lcom->waitForResponse();
	if(success) {
		LOG(INFO) << "[LIFT] Successful movement";
	} else {
		LOG(WARNING) << "[LIFT] UNSUCCESSFULL movement";
	}
	completeCurrent();
}


double PosControl::updateDist(double angle, double distX, double distY) {
	if(sin_d(angle) == 0) {
		return distX;
	} else {
		return distY/sin_d(angle);
	}
}


//TODO: get input from IMU
//need exact rotation to do small angle adjustments
void PosControl::rotate(double distR) {
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
void PosControl::drive(double dist) {	
	LOG(DEBUG) << "DISTR: " << dist;
	double rotation = goalPos->getAngle();
	if(closeEnoughAngle()) {
		LOG(DEBUG) << "[POS] driving with rotation:" << rotation << " dist:" << dist;

		//CHECK: unneccsary test?
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
	
	double angle = goalPos->getAngle();

	int ediff = encoderDifference();
	if(ediff > 50) {
		LOG(WARNING) << "[POS] Warning, large encoder difference(drive): " << ediff;
		return;
	}

	LOG(INFO) << "L: " << leftEncoder.diffDist << "  R: " << rightEncoder.diffDist;
	LOG(INFO) << "Lt: " << leftEncoder.total << " Rt: " << rightEncoder.total;
	double avg_dist = (abs(leftEncoder.diffDist) + abs(rightEncoder.diffDist)) / 2;
	
	double x_distance = cos_d(angle)*avg_dist; //45 = +
	double y_distance = sin_d(angle)*avg_dist; //45 = +


	curPos->updateX( x_distance );
	curPos->updateY( y_distance );
}


void PosControl::updateRotation() {
	updateLeftEncoder();
	updateRightEncoder();

	LOG(INFO) << "L: " << leftEncoder.diffDist << "  R: " << rightEncoder.diffDist;
	LOG(INFO) << "Lt: " << leftEncoder.total << " Rt: " << rightEncoder.total;

	curPos->updateAngle(leftEncoder.diff, rightEncoder.diff);
}


void PosControl::updateEncoder(long e, struct encoder *enc) {
	long diff = e - enc->prev;
	long absDiff = abs(diff);
	double distance = diff*ENCODER_CONSTANT;

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
double PosControl::distanceX() {
	return (goalPos->getX() - curPos->getX());
}


/* >0 : goal is in positive x-direction
 * =0 : in exact x-coordinate
 * <0 : goal is in negative x-direction
 */
double PosControl::distanceY() {
	return (goalPos->getY() - curPos->getY());	
}


/* >0 : goal is in positive direction
 * =0 : at exact angle
 * <0 : goal angle in negative direction
 */
double PosControl::distanceAngle() {
	double dist = curPos->distanceRot(goalPos->getAngle());
	return dist;
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


double PosControl::sin_d(double angle) {
	return sin(angle*M_PI/180);
}


double PosControl::cos_d(double angle) {
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
	double diffDist;
	long total;
	double totalDist;
	*/
}