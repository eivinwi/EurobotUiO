#include "poscontrol.h"

//TODO: implement position-queue

struct encoder {
	long prev;
	long diff;
	float diffDist;
	long total;
	float totalDist;
} leftEncoder, rightEncoder;


PosControl::PosControl(MotorCom *s) {
	com = s;
	goalPos = new GoalPosition;
	curPos = new Position;
	exactPos = new Position;
	resetPosition();
}


PosControl::~PosControl() {
}


/* sets the new goal pos
 * WARNING: currently only one arguments should be changed at the time
*/
void PosControl::setGoalPos(int x, int y, int rot) {
	PRINTLINE("[POS] setGoalPos(" << x << "," << y << "," << rot << ")  : ");
	x *= 10;
	y *= 10;
	int change = 0;
	if(x != goalPos->getX()) change++;
	if(y != goalPos->getY()) change++;
	if(rot != goalPos->getRotation()) change++;

	if(change == 0) {
		PRINTLINE("already at specified goalPos->");
	} else if(change > 1) {
		PRINTLINE("only one position can be changed at a time.");
	} else {
		goalPos->set(x, y, rot);			
	}
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
}


//return false - no yet in goalPos
//return true - in goalPos
	/*if(goalPos.timeSinceUpdate()) > TOO_LONG) {
		DBPL("Too long time since goal update, shutting down.");
		fullStop();
		exit(EXIT_SUCCESS);
	}*/
void PosControl::controlLoop() {
	working = false;
	do {
		//TODO: check if too long since position update

		int distR = rotationOffset();
		float distX = distanceX(); 
		float distY = distanceY();

	/*	PRINTLINE("===== CONTROL-LOOP =====");
		printGoal();    //DBP
		printCurrent(); //is always printed
		printDist();	//DBP
		PRINTLINE("========================\n");*/

		// Checking if rotation is necessary.
		// If not: checking if position-change is necessary
		if(abs(distR) > ROTATION_CLOSE_ENOUGH) {
			working = true;
			changeRotation(distR);
			updatePosition(TURNING);
			curPos->updatePosString();
			//printCurrent();
		}
		else if(abs(distX) > POSITION_CLOSE_ENOUGH) {
			working = true;
			driveX(distX);
			updatePosition(DRIVE_X);
			curPos->updatePosString();
			//printCurrent();
		} 	
		else if(abs(distY) > POSITION_CLOSE_ENOUGH) {
			working = true;	
			driveY(distY);
			updatePosition(DRIVE_Y);
			curPos->updatePosString();
			//printCurrent();
		} else {
			if(working) {
				TIMESTAMP(" GOAL REACHED ******");
				printGoal();
				printCurrent();
				printDist();
				PRINTLINE("************************\n\n\n");
				curPos->set(goalPos->getX(), goalPos->getY(), goalPos->getRotation());

				curPos->updatePosString();
			}
			fullStop();
			working = false;
			usleep(500);
		}
		//curPos->updatePosString();
		usleep(2000);
	} while(true);

	PRINTLINE("*** EXITING CONTROL LOOP ***");
	usleep(1000000);
}


bool PosControl::inGoal() {
	if(abs(rotationOffset()) > ROTATION_CLOSE_ENOUGH) return false;
	if(abs(distanceX()) > ROTATION_CLOSE_ENOUGH) return false;
	if(abs(distanceY()) > ROTATION_CLOSE_ENOUGH) return false;
	return true;
}


void PosControl::changeRotation(float distR) {
	DBP("[POS] turning: ");
	if(distR == 0) {
		DBPL("Error, turn is 0.");
	} else if(distR > 0) {
		DBPL("Positive dir (" << distR << ")");
		if(distR > SLOWDOWN_DISTANCE_ROT) {
			com->setSpeedL(SPEED_MED_POS);
			com->setSpeedR(SPEED_MED_NEG);
		} else {
			DBPL("SLOWDOWN ROT");
			com->setSpeedL(SPEED_SLOW_POS);
			com->setSpeedR(SPEED_SLOW_NEG);
		}
	} else {
		DBPL("Negative dir (" << distR << ")");
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
	float rotation = curPos->getRotation();	
	if(goalPos->getRotation() != 0 && goalPos->getRotation() != 180) {
		DBPL("[POS] ERROR; goalX specified, but goalRot is " << goalPos->getRotation());
	} 
	else if(closeEnoughAngle(rotation, 0)) {
		DBPL("[POS] driveX rotation: " << rotation);
		if(distX > 0) {
			if(distX > SLOWDOWN_DISTANCE) {
				DBPL("FULLSPEED");
				com->setSpeedBoth(SPEED_MED_POS);
			} else {
				DBPL("SLOWDOWN X");
				com->setSpeedBoth(SPEED_SLOW_POS);
			}
		} else {
			if(distX < -SLOWDOWN_DISTANCE) {
				com->setSpeedBoth(SPEED_MED_NEG);
			} else {
				com->setSpeedBoth(SPEED_SLOW_NEG);
			}
		}
	} else if(closeEnoughAngle(rotation, 180)) {
		DBPL("[POS] driveX rotation: " << rotation);
		if(distX > 0) {
			if(distX > SLOWDOWN_DISTANCE) {
				DBPL("FULLSPEED");
				com->setSpeedBoth(SPEED_MED_NEG);
			} else {
				DBPL("SLOWDOWN X");
				com->setSpeedBoth(SPEED_SLOW_NEG);
			}
		} else {
			if(distX < -SLOWDOWN_DISTANCE) {
				com->setSpeedBoth(SPEED_MED_POS);
			} else {
				com->setSpeedBoth(SPEED_SLOW_POS);
			}
		}
	} 
	else {
		DBPL("[POS] ERROR; cur->angle is" << curPos->getRotation() << " should be 0/180. Attemting to fix by turning");
		changeRotation(rotationOffset());
	}
}


void PosControl::driveY(float distY) {
	float rotation = curPos->getRotation();
	if(goalPos->getRotation() != 90 && goalPos->getRotation() != 270) {
		DBPL("[POS] ERROR; goalY specified, but goalRot is " << goalPos->getRotation());
	} 
	else if(closeEnoughAngle(rotation, 90)) {
		DBPL("[POS] correct rotation, can drive Y.");
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
	else if(closeEnoughAngle(rotation,270)) {
		DBPL("[POS] correct rotation, can drive Y.");
		if(distY > 0) {
			if(distY > SLOWDOWN_DISTANCE) {
				com->setSpeedBoth(SPEED_MED_NEG);
			} else {
				com->setSpeedBoth(SPEED_SLOW_NEG);
			}
		} else {
			if(distY < -SLOWDOWN_DISTANCE) {
				com->setSpeedBoth(SPEED_MED_POS);
			} else {
				com->setSpeedBoth(SPEED_SLOW_POS);
			}
		}
	} 	
	else {
		DBPL("[POS] ERROR; cur->angle is" << rotation << " should be 90/270. Attempting to fix by turning");
		changeRotation(rotationOffset());		
	}
}


std::string PosControl::getCurrentPos() {
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
void PosControl::updatePosition(int action) {
	updateLeftEncoder();
	updateRightEncoder();

	DBP("[POS] updatePos ");
	if(action == TURNING) {
		DBPL("Turning");
		//curX and curY should not really be updated
		// update angle
		curPos->updateAngle(leftEncoder.diff, rightEncoder.diff);
	}
	else if(action == DRIVE_X) {
		DBPL("DriveX");
		int ediff = encoderDifference();
		if(ediff > 50) {
			DBPL("[POS] Warning, large encoder difference(drivex): " << ediff);
		}

		if(goalPos->getRotation() == 180) {
			curPos->decrX( leftEncoder.diffDist );
		} else {
			curPos->incrX( leftEncoder.diffDist );
		}

	}
	else {  //if(action == DRIVE_Y) {
		DBPL("DriveY");
		int ediff = encoderDifference();
		if(ediff > 50) {
			DBPL("[POS] Warning, large encoder difference(drivey): " << ediff);
		}

		if(goalPos->getRotation() == 270) {
			curPos->decrY( leftEncoder.diffDist );
		} else {
			curPos->incrY( leftEncoder.diffDist );
		}
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


void PosControl::fullStop() {
	com->setSpeedBoth(SPEED_STOP);
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
float PosControl::rotationOffset() {
	float dist = curPos->distanceRot(goalPos->getRotation());
	return dist;
}


float PosControl::average(long a, long b) {
	//TODO: negative values!!!
	return (a - b)/2; 
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


void PosControl::printCurrent() {
	PRINT("[POS] Current: ");
	curPos->print();
}


void PosControl::printGoal() {
	PRINT("[POS] Current: ");
	//#ifdef DEBUG
	goalPos->print();
	//#endif
}


void PosControl::printDist() {
	PRINTLINE("[POS] dist: " << distanceX() << "," << distanceY() << "," << rotationOffset());
}