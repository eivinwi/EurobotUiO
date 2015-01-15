#include "poscontrol.h"
/*TODO: 	
 	- use IMU for angle untill beacon system is workinh
 	- should continually check angle instead of once!! (impossible with only encoders)
	- pooling wait has poor performance
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
	int type;
};


PosControl::PosControl(MotorCom *s, bool test) {
	testing = test;
	com = s;
	goalPos = new GoalPosition;
	curPos = new Position;
	exactPos = new Position;
	resetPosition();
}


PosControl::~PosControl() {
}


void PosControl::resetPosition() {
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
}


void PosControl::enqueue(int id, int x, int y, float rot, int type) {
	qPos qp = {id, x, y, rot, type};
	PRINTLINE("[POS] enqueueing qPos {" << id << "," << x << "," << y << "," << rot << "," << type << "}");

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
	PRINTLINE("[POS] setGoalPos(" << x << "," << y << ")");
	goalPos->setPosition(x, y);
	goToPosition();
}


//runs in its own thread, initialized in main.cpp
void PosControl::controlLoop() {
	while(true) {
		qPos qp = dequeue();

		//create goalPos from qp
		goalPos = new GoalPosition(qp.id, qp.x, qp.y, qp.rot);
		if(!testing) {
			PRINTLINE("[POS] dequeued qPos {" << qp.id << "," << qp.x << "," << qp.y << "," << qp.rot << "," << qp.type << "}");	

			if(qp.type == ROTATION) {
				goToRotation();
			} else if(qp.type == POSITION) {
				goToPosition();
			}
		} 
		else {
			if(qp.type == ROTATION) {
				curPos->setAngle(goalPos->getRotation());
				completeCurrent();
			} else if(qp.type == POSITION) {
				curPos->set(goalPos->getX(), goalPos->getY(), curPos->getRotation());
				completeCurrent();
			}
		}
	}
}


void PosControl::goToRotation() {
	PRINTLINE("[POS] goToRotation " << curPos->getRotation() << "->" << goalPos->getRotation());

	resetEncoders(); //here??	 
	float distR = 0.0;
//	bool rotated = false;
	if(closeEnoughX() && closeEnoughY() && !closeEnoughAngle()) {
		
		do {

			distR = distanceAngle();
			rotate(distR);
			updateRotation();
			usleep(1000);
		} while(!closeEnoughAngle());
		completeCurrent();
		fullStop();		
		PRINTLINE("[POS] 	rotation finished: " << curPos->getRotation() << "=" << goalPos->getRotation());		
	} 
	else {
		fullStop();
		completeCurrent();
		PRINTLINE("[POS] 	already at specified rotation: " << curPos->getRotation() << "=" << goalPos->getRotation());
	}
}


//CHECK: delays or not?
void PosControl::goToPosition() {
	PRINTLINE("[POS] goToPosition (" << curPos->getX() << "," << curPos->getY() << ") -> (" << goalPos->getX() << "," << goalPos->getY() << ")");

	resetEncoders(); //here??
	float distX = distanceX(); 
	float distY = distanceY();
	float distR = 0.0;
	float angle = 0.0;
	float dist = 0.0;

	if(!inGoalPosition()) {

		//get angle we need to rotate to before driving
		angle = atan2(distY, distX) *(180/M_PI);
		if(angle < 0) angle = angle+360;
		PRINTLINE("[POS] CALCULATED ANGLE=" << angle);
		goalPos->setAngle(angle);
				
		//calculate straigth distance
		dist = updateDist(angle, distX, distY);
		
		do {
			distX = distanceX(); 
			distY = distanceY();
			distR = distanceAngle();
			dist = updateDist(angle, distX, distY);

			//PRINTLINE("CURRENT: " << curPos->getX() << " | " << curPos->getY() << " | " << curPos->getRotation());
			printCurrent();

			if(!closeEnoughAngle()) {
				//PRINTLINE("[LOOP] ROTATION: " << distR);
				rotate(distR);
				updateRotation();
			} else {
				curPos->setAngle(goalPos->getRotation()); 
				//PRINTLINE("[LOOP] DRIVE: " << distX << "," << distY);
				drive(dist);
				updatePosition();
			} 
			usleep(2000);
		} while(!inGoal());
	}

	fullStop();
	PRINTLINE("[POS] IN GOAL!  (" << curPos->getX() << " , " << goalPos->getX() << ") ~= (" << curPos->getY() << " , " << goalPos->getY() << ")");
 
	completeCurrent();
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
void PosControl::rotate(float distR) {
	DBP("[POS] turning: ");
	if(distR == 0) {
		DBPL("Error, turn is 0.");
	} else if(distR > 0) {
		DBPL("Positive dir (" << distR << ")");
		if(distR > SLOWDOWN_DISTANCE_ROT) {
			setSpeed(SPEED_MED_POS, SPEED_MED_NEG);
		} else {
			DBPL("SLOWDOWN ROT");
			setSpeed(SPEED_SLOW_POS, SPEED_SLOW_NEG);
		}
	} else {
		DBPL("Negative dir (" << distR << ")");
		if(distR < -SLOWDOWN_DISTANCE_ROT) {
			setSpeed(SPEED_MED_NEG, SPEED_MED_POS);
		} else {
			setSpeed(SPEED_SLOW_NEG, SPEED_SLOW_POS);
		}
	}
}


void PosControl::drive(float dist) {	
	float rotation = goalPos->getRotation();
	if(closeEnoughAngle()) {
		//PRINTLINE("[POS] driving with rotation:" << rotation << " dist:" << dist);	
		//reset encoders??
		//unneccsary test?
		if(!inGoal()) {

			if(dist < 0) {
				if(dist > SLOWDOWN_DISTANCE) {
					//PRINTLINE("FULLSPEED REVERSE");
					setSpeed(SPEED_MED_NEG, SPEED_MED_NEG);
				} else {
					//PRINTLINE("SLOW REVERSE");
					setSpeed(SPEED_SLOW_NEG, SPEED_SLOW_NEG);
				}
			}
			else {
				if(dist < -SLOWDOWN_DISTANCE) {
					//PRINTLINE("FULLSPEED FORWARD");
					setSpeed(SPEED_MED_POS, SPEED_MED_POS);
				} else {
					//PRINTLINE("SLOW FORWARD");					
					setSpeed(SPEED_SLOW_POS, SPEED_SLOW_POS);
				}
			}
		} else {
			PRINTLINE("[POS] ERROR; already in goal");
		}
	} 
	else {
		PRINTLINE("[POS] ERROR; cur->angle is" << rotation << " should be " << goalPos->getRotation() << ". Attemting to fix by turning");
		PRINTLINE("[POS] Scratch that last part, fixing is not yet implemented.");	
	}
}


void PosControl::completeCurrent() {
	PRINTLINE("[POS] action " << goalPos->getId() << " completed.");	
	if(completed_actions[goalPos->getId()]) {
		PRINTLINE("[POS] action " << goalPos->getId() << " was already finished.");
	}
	completed_actions[goalPos->getId()] = true;
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


/*** PRIVATE FUNCTIONS: ***/


/*
 * Angle calculation:
 * omkrets = 2*pi*r = 2*pi*(330/2)=pi*330 = 1036
 * mm per grad = 1036/360 = 2.88
 * enc per mm = 980/337 = 2.6
 * enc per grad = 2.88*2.6 = 7.5
 */
 // CHECK: works with negative??
 //TODO: DOES NOT WORK!!!
void PosControl::updatePosition() {
	DBPL("[POS] updatePos ");
	
	updateLeftEncoder();
	updateRightEncoder();
	
	DBPL("[POS] encoders updated");
	
	float angle = goalPos->getRotation();
//	angle = 0.0;

	int ediff = encoderDifference();
	if(ediff > 50) {
		PRINTLINE("[POS] Warning, large encoder difference(drive): " << ediff);
		return;
	}

//	PRINTLINE("L: " << leftEncoder.diffDist << "  R: " << rightEncoder.diffDist);
	float avg_dist = (abs(leftEncoder.diffDist) + abs(rightEncoder.diffDist)) / 2;
	
	float x_distance = cos_d(angle)*avg_dist; //45 = +
	float y_distance = sin_d(angle)*avg_dist; //45 = +


	curPos->updateX( x_distance );
	curPos->updateY( y_distance );
}


void PosControl::updateRotation() {
	updateLeftEncoder();
	updateRightEncoder();

	DBP("[POS] updateRot ");
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
		DBPL("Error: unreasonable encoder value.");
	}


	enc->prev = e;
	enc->diff = diff;
	enc->diffDist = distance;
	enc->total += diff;
	enc->totalDist += distance;

	DBPL("enc: " << e << " diff: " << diff << " distance: " << distance);
	DBPL("total: " << enc->total << " totalDist: " << enc->totalDist);
}


void PosControl::updateLeftEncoder() {
	DBPL("[POS]updating left encoder");
	com->flush(); //unnecessary?
	long enc = com->getEncL();
	updateEncoder(enc, &leftEncoder);
}


void PosControl::updateRightEncoder() {
	DBPL("[POS]updating right encoder");
	com->flush(); //unnecessary?
	long enc = com->getEncR();
	updateEncoder(enc, &rightEncoder);
}


void PosControl::resetEncoders() {
	leftEncoder.prev = 0;
	leftEncoder.diff = 0;
	leftEncoder.diffDist = 0;
	rightEncoder.prev = 0;
	rightEncoder.diff = 0;
	rightEncoder.diffDist = 0;	
	com->resetEncoders();
}


void PosControl::fullStop() {
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
	float dist = curPos->distanceRot(goalPos->getRotation());
	return dist;
}


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
	PRINT("[POS] Current: ");
	curPos->print();
}


void PosControl::printGoal() {
	PRINT("[POS] Goal: ");
	//#ifdef DEBUG
	goalPos->print();
	//#endif
}


void PosControl::printDist() {
	PRINTLINE("[POS] dist: " << distanceX() << "," << distanceY() << "," << distanceAngle());
}


void PosControl::setSpeed(int l, int r) {
	if(curSpeedLeft != l) {
		com->setSpeedL(l);
		curSpeedLeft = l;
	} 
	if(curSpeedRight != r) {
		com->setSpeedR(r);
		curSpeedRight = r;
	}
}


float PosControl::sin_d(float angle) {
	return sin(angle*M_PI/180);
}


float PosControl::cos_d(float angle) {
	return cos(angle*M_PI/180);
}