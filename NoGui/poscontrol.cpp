#include "poscontrol.h"
#include <iostream>
#include <string>
#include <sstream>
#include "rotation.h"
//using namespace std;




/*
	TODO:
		- fix encoder values on turn
		- negative encoder values

		- share objects in threads

*/


/* curPos - current estimated position based on dead reckoning with encoders. Is updated regularly to be Exactpos
 * ExactPos   - input position from SENS
 * GoalPos    - input from main or AI 
*/
struct goalPosition {
	float x;
	float y;
	float rot;
	time_t changed;
} goalPos;

struct exactPosition {
	float x;
	float y;
	float rot;
} exactPos;

struct currentPosition {
	float x;
	float y;
	Rotation *rotation;
	time_t changed;
} curPos;

struct encoder {
	long prev;
	long diff;
	float diffDist;
	long total;
	float totalDist;
} leftEncoder, rightEncoder;




PosControl::PosControl(MotorCom *s) {
	com = s;

	curPos.x = 0.0;
	curPos.y = 0.0;
	curPos.rotation = new Rotation;
	curPos.changed = time(0);

	goalPos.x = 0.0;
	goalPos.y = 0.0;
	goalPos.rot = 0.0;
	goalPos.changed = time(0); //initialize to avoid problems, might be wrong
	
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
}

PosControl::~PosControl() {
}

//TODO: fix multiple changes
//sets the new goal pos
// WARNING: currently only one arguments should be changed at the time
void PosControl::setGoalPos(int x, int y, int rot) {
	PRINT("POS: setGoalPos(" << x << "," << y << "," << rot << ")  : ");
	x *= 10;
	y *= 10;
	int change = 0;
	if(x != goalPos.x) change++;
	if(y != goalPos.y) change++;
	if(rot != goalPos.rot) change++;

	if(change == 0) {
		PRINTLINE("already at specified goalPos.");
	} else if(change > 1) {
		PRINTLINE("only one position can be changed at a time.");
	} else {
		if(x != goalPos.x) goalPos.x = x;
		if(y != goalPos.y) goalPos.y = y;
		if(rot != goalPos.rot) goalPos.rot = rot;
		goalPos.changed = time(0);
	}
}


void PosControl::resetPosition() {
	curPos.x = 0.0;
	curPos.y = 0.0;
	curPos.rotation->reset();
	curPos.changed = time(0);

	goalPos.x = 0.0;
	goalPos.y = 0.0;
	goalPos.rot = 0.0;
	goalPos.changed = time(0);

	com->resetEncoders();
}


//return false - no yet in goalPos
//return true - in goalPos
bool PosControl::controlLoop() {
	/*if(timeSinceGoal() > TOO_LONG) {
		PRINTLINE("Too long time since goal update, shutting down.");
		fullStop();
		exit(EXIT_SUCCESS);
	}*/

	int distR = rotationOffset();
	float distX = distanceX(); 
	float distY = distanceY();

	PRINTLINE("===== CONTROL-LOOP =====");
	printGoal();
	printCurrent();
	printDist();
	PRINTLINE("========================\n");

	// Checking if rotation is necessary.
	// If not: checking if position-change is necessary
	if(abs(distR) > ROTATION_CLOSE_ENOUGH) {
		changeRotation(distR);
		updatePosition(TURNING);
	}
	else if(abs(distX) > POSITION_CLOSE_ENOUGH) {
		driveX(distX);
		updatePosition(DRIVE_X);
	} 	
	else if(abs(distY) > POSITION_CLOSE_ENOUGH) {	
		driveY(distY);
		updatePosition(DRIVE_Y);
	}
	else {	
		PRINTLINE("**** GOAL REACHED ******");
		printGoal();
		printCurrent();
		printDist();
		PRINTLINE("************************\n\n\n");
		fullStop();
		return true;
	}
	PRINTLINE("\n");
	return false;
}

//TODO: fix negative
void PosControl::changeRotation(float distR) {
	PRINT("POS: turning: ");
	if(distR == 0) {
		PRINTLINE("Error, turn is 0.");
	} else if(distR > 0) {
		PRINTLINE("Positive dir (" << distR << ")");
		if(distR > SLOWDOWN_DISTANCE_ROT) {
			com->setSpeedL(SPEED_MED_POS);
			com->setSpeedR(SPEED_MED_NEG);
		} else {
			com->setSpeedL(SPEED_SLOW_POS);
			com->setSpeedR(SPEED_SLOW_NEG);
		}
	} else {
		PRINTLINE("Negative dir (" << distR << ")");
		if(distR < -SLOWDOWN_DISTANCE_ROT) {
			com->setSpeedL(SPEED_MED_NEG);
			com->setSpeedR(SPEED_MED_POS);
		} else {
			com->setSpeedL(SPEED_SLOW_NEG);
			com->setSpeedR(SPEED_SLOW_POS);
		}
	}
}

void PosControl::driveX(float distX) {
	float rotation = currentRotation();	
	if(goalPos.rot != 0 && goalPos.rot != 180) {
		PRINTLINE("POS: ERROR; goalX specified, but goalRot is " << goalPos.rot);
	} 
	else if(!closeEnoughAngle(rotation, 0) && !closeEnoughAngle(rotation, 180)) {
		PRINTLINE("POS: ERROR; cur->angle is" << currentRotation() << " should be 0/180. Attemting to fix by turning");
		changeRotation(rotationOffset());
	} 
	else {
		PRINTLINE("POS: correct rotation, can drive X.");
		if(distX > 0) {
			if(distX > SLOWDOWN_DISTANCE) {
				com->setSpeedBoth(SPEED_MED_POS);
			} else {
				com->setSpeedBoth(SPEED_SLOW_POS);
			}
		} else {
			if(distX < -SLOWDOWN_DISTANCE) {
				com->setSpeedBoth(SPEED_MED_NEG);
			} else {
				com->setSpeedBoth(SPEED_SLOW_NEG);
			}
		}
	}
}

void PosControl::driveY(float distY) {
	float rotation = currentRotation();
	if(goalPos.rot != 90 && goalPos.rot != 270) {
		PRINTLINE("POS: ERROR; goalY specified, but goalRot is " << goalPos.rot);
	} 
	else if(!closeEnoughAngle(rotation, 90) && !closeEnoughAngle(rotation,270)) {
		PRINTLINE("POS: ERROR; cur->angle is" << rotation << " should be 90/270. Attempting to fix by turning");
		changeRotation(rotationOffset());
	} 
	else {
		PRINTLINE("POS: correct rotation, can drive Y.");
		if(distY > 0) {
			if(distY > SLOWDOWN_DISTANCE) {
				com->setSpeedBoth(SPEED_MED_POS);
			} else {
				com->setSpeedBoth(SPEED_SLOW_POS);
			}
		} else {
			if(distY < -SLOWDOWN_DISTANCE) {
				com->setSpeedBoth(SPEED_MED_NEG);
			} else {
				com->setSpeedBoth(SPEED_SLOW_NEG);
			}
		}
	} 	
}

/*
Angle calculation:
omkrets = 2*pi*r = 2*pi*(330/2)=pi*330 = 1036
mm per grad = 1036/360 = 2.88
enc per mm = 980/337 = 2.6
enc per grad = 2.88*2.6 = 7.5
*/
void PosControl::updatePosition(int action) {
	updateLeftEncoder();
	updateRightEncoder();
//	int angle =	getRotation();

	PRINT("POS: updatePos ");
	if(action == TURNING) {
//		PRINTLINE("Turning");
		//curX and curY should not really be updated
		// update angle
		curPos.rotation->updateAngle(leftEncoder.diff, rightEncoder.diff);
	}
	else if(action == DRIVE_X) {
	//	PRINTLINE("DriveX");
		int ediff = encoderDifference();
		if(ediff > 50) {
			PRINTLINE("POS: Warning, large encoder difference(drivex): " << ediff);
		}

		curPos.x += leftEncoder.diffDist*POS_DIR;
	}
	else {  //if(action == DRIVE_Y) {
	//	PRINTLINE("DriveY");
		int ediff = encoderDifference();
		if(ediff > 50) {
			PRINTLINE("POS: Warning, large encoder difference(drivey): " << ediff);
		}

		curPos.y += leftEncoder.diffDist*POS_DIR;
	}

}


/*
 * Encoder counts: 980 per output shaft turn
 * Wheel diameter: 120mm
 * Wheel circumference: 377mm
 * Distance per count: 0.385mm
 */
void PosControl::updateEncoder(long e, struct encoder *enc) {
	long diff = e - enc->prev;
	long absDiff = abs(diff);
	float distance = diff*0.385;

	if(absDiff > REASONABLE_ENC_DIFF) {
		//TODO: reset encoders while taking care of values in a controlled manner
		PRINTLINE("Error: unreasonable encoder value.");
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
	DBPL("POS:updating left encoder");
	com->flush(); //unnecessary?
	long enc = com->getEncL();
	updateEncoder(enc, &leftEncoder);
}


void PosControl::updateRightEncoder() {
	DBPL("POS:updating right encoder");
	com->flush(); //unnecessary?
	long enc = com->getEncR();
	updateEncoder(enc, &rightEncoder);
}


void PosControl::fullStop() {
	com->setSpeedBoth(SPEED_STOP);
}



float PosControl::currentRotation() {
	return curPos.rotation->getAngle();
}


/* >0 : goal is in positive x-direction
 * =0 : in exact x-coordinate
 * <0 : goal is in negative x-direction
 */
float PosControl::distanceX() {
	return (goalPos.x - curPos.x);
}


/* >0 : goal is in positive x-direction
 * =0 : in exact x-coordinate
 * <0 : goal is in negative x-direction
 */
float PosControl::distanceY() {
	return (goalPos.y - curPos.y);	
}


/* >0 : goal is in positive direction
 * =0 : at exact angle
 * <0 : goal angle in negative direction
 */
float PosControl::rotationOffset() {
	float dist = curPos.rotation->distanceTo(goalPos.rot);
	return dist;
}


float PosControl::average(long a, long b) {
	return (a - b)/2; //TODO: negative values!!!
}


int PosControl::getRotation() {
	//TODO: everything
	return 0;
}


bool PosControl::closeEnoughEnc(long a, long b) {
	if((abs(a) - abs(b)) < 20) {
		return true;
	} else {
		return false;
	}
}


bool PosControl::closeEnoughAngle(int a, int b) {
	if((abs(a) - abs(b)) < ROTATION_CLOSE_ENOUGH) {
		return true;
	} else {
		return false;
	}
}


long PosControl::encoderDifference() {
	return (abs(leftEncoder.diff) - abs(rightEncoder.diff));
}


double PosControl::timeSinceGoal() {
	return (time(0) - goalPos.changed);
}


void PosControl::printCurrent() {
	PRINTLINE("cur : " << curPos.x << "," << curPos.y << "," << currentRotation());
}

void PosControl::printGoal() {
	PRINTLINE("goal: " << goalPos.x << "," << goalPos.y << "," << goalPos.rot);	
}

void PosControl::printDist() {
	PRINTLINE("dist: " << distanceX() << "," << distanceY() << "," << rotationOffset());
}