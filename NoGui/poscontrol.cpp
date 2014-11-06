#include "poscontrol.h"
#include <iostream>
#include <string>
#include <sstream>
//using namespace std;

/* curPos - current estimated position based on dead reckoning with encoders. Is updated regularly to be Exactpos
 * ExactPos   - input position from SENS
 * GoalPos    - input from main or AI 
*/
struct position {
	float x;
	float y;
	int rot;
	time_t changed;
} goalPos, curPos, exactPos;

struct encoder {
	long prev;
	long diff;
	float diffDist;
	long total;
	float totalDist;
} leftEncoder, rightEncoder;




PosControl::PosControl(MotorCom *s) {
	com = s;

	curPos.x = 0;
	curPos.y = 0;
	curPos.rot = 0;
	curPos.changed = time(0);

	goalPos.x = 0;
	goalPos.y = 0;
	goalPos.rot = 0;
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

#define ROTATION_CLOSE_ENOUGH 1
#define POSITION_CLOSE_ENOUGH 1
#define TOO_LONG 20

void PosControl::controlLoop() {
	if(timeSinceGoal() > TOO_LONG) {
		PRINTLINE("Too long time since goal update, shutting down.");
		stop();
		exit(EXIT_SUCCESS);
	}

	int distR = rotationOffset();
	float distX = distanceFromX(); 
	float distY = distanceFromY();

	PRINTLINE("===== CONTROL-LOOP =====");
	PRINTLINE("distR: " << distR << "distX: " << distX << "distY: " << distY);


	if(distR > ROTATION_CLOSE_ENOUGH) {
		PRINTLINE("POS: distR: " << distR << " (" << goalPos.rot << "-" << curPos.rot);
		turn(distR);
		updatePosition(TURNING);
	} 

	else if(distX > POSITION_CLOSE_ENOUGH) {
		PRINTLINE("POS: distX: " << distX << " (" << goalPos.x << "-" << curPos.x);
	
		if(goalPos.rot != 0 && goalPos.rot != 180) {
			PRINTLINE("POS: ERROR; goalX specified, but goalRot is " << goalPos.rot);
			return;
		} else if(curPos.rot != 0 && curPos.rot != 180) {
			PRINTLINE("POS: ERROR; curPos.rot is" << curPos.rot << " should be 0/180. ");
			PRINTLINE("POS: attempting to fix by turning");
			turn(rotationOffset());
		} else {
			PRINTLINE("POS: correct rotation, can drive.");

			if(distX > 0) {
				com->setSpeedBoth(SPEED_MED_POS);
			} else {
				com->setSpeedBoth(SPEED_MED_NEG);
			}
		}
		updatePosition(DRIVE_X);
	} 	
	else if(distY > POSITION_CLOSE_ENOUGH) {
		PRINTLINE("POS: distY: " << distY << " (" << goalPos.y << "-" << curPos.y);
	
		if(goalPos.rot != 90 && goalPos.rot != 270) {
			PRINTLINE("POS: ERROR; goalX specified, but goalRot is " << goalPos.rot);
			return;
		} else if(curPos.rot != 0 && curPos.rot != 180) {
			PRINTLINE("POS: ERROR; curPos.rot is" << curPos.rot << " should be 90/270. ");
			PRINTLINE("POS: attempting to fix by turning");
			turn(rotationOffset());
		} else {
			PRINTLINE("POS: correct rotation, can drive.");

			if(distY > 0) {
				com->setSpeedBoth(SPEED_MED_POS);
			} else {
				com->setSpeedBoth(SPEED_MED_NEG);
			}
		} 	
		updatePosition(DRIVE_Y);
	}
	else {	
		PRINTLINE("POS: GOAL REACHED");
		stop();
	}
}




void PosControl::updatePosition(int action) {
	updateLeft();
	updateRight();
	int angle =	getRotation();

	if(action == TURNING) {
		//curX and curY should not really be updated
		curPos.rot = angle;
	}
	else if(action == DRIVE_X) {
		int ediff = encoderDifference();
		if(ediff > 50) {
			PRINTLINE("POS: Warning, large encoder difference: " << ediff);
		}

		curPos.x += leftEncoder.diffDist;
	}
	else if(action == DRIVE_Y) {
		int ediff = encoderDifference();
		if(ediff > 50) {
			PRINTLINE("POS: Warning, large encoder difference: " << ediff);
		}

		curPos.y += leftEncoder.diffDist;
	}

}

void PosControl::stop() {
	com->setSpeedBoth(SPEED_STOP);
}


//positive turning directions is LEFT
void PosControl::turn(int distR) {
	PRINT("POS: turning: ");
	if(distR == 0) {
		PRINTLINE("Error, turn is 0.");
	} else if(distR > 0) {
		PRINTLINE("Positive dir (" << distR << ")");
		com->setSpeedL(SPEED_MED_POS);
		com->setSpeedR(SPEED_MED_NEG);
	} else {
		PRINTLINE("Negative dir (" << distR << ")");
		com->setSpeedL(SPEED_MED_POS);
		com->setSpeedR(SPEED_MED_NEG);
	}
}

/* >0 : goal is in positive x-direction
 * =0 : in exact x-coordinate
 * <0 : goal is in negative x-direction
 */
float PosControl::distanceFromX() {
	return (goalPos.x - curPos.x);
}


/* >0 : goal is in positive x-direction
 * =0 : in exact x-coordinate
 * <0 : goal is in negative x-direction
 */
float PosControl::distanceFromY() {
	return (goalPos.x - curPos.x);	
}

/* >0 : goal is in positive direction
 * =0 : at exact angle
 * <0 : goal angle in negative direction
 */
int PosControl::rotationOffset() {
	return (goalPos.rot - curPos.rot);
}

void PosControl::setGoalPos(int x, int y, int rot) {
	PRINT("POS: goal");
	if(goalPos.x != x*10) {
		goalPos.x = x*10;
		goalPos.changed = time(0);
		PRINTLINE("Pos.x = " << x*10);
	} 
	if(goalPos.y != y*10) {
		goalPos.y = y*10;
		goalPos.changed = time(0);
		PRINTLINE("Pos.y = " << y*10);
	} 
	if(goalPos.rot != rot) {
		goalPos.rot = rot;
		goalPos.changed = time(0);
		PRINTLINE("Pos.rot = " << rot);
	} 
}



// following updatePosition() is more complex, should be used if diagonal travel is wanted
/*
void PosControl::updatePosition() {
	updateLeft();
	updateRight();
	int angle =	getRotation();

	//left&right encoder movement is hypotenuse
	//calculate opp/adj:

	if(closeEnoughEnc(leftEncoder.diff, rightEncoder.diff) && closeEnoughAngle(angle, curPos.rot)) {
		//traveled straight
		float avg = average(leftEncoder.diff, rightEncoder.diff);

		float opposite = sin(angle)*avg;
		float adjacent = cos(angle)*avg;


		if(angle >= 0 && angle < 90) {
			curPos.x += adjacent; //x++
			curPos.y += opposite; //y++
		} else if(angle >= 90 && angle < 180) {
			curPos.x -= opposite; //x--
			curPos.y += adjacent; //y++
		} else if(angle >= 180 && angle < 270) {
			curPos.x -= adjacent; //x--
			curPos.y -= opposite; //y--
		} else if(angle <= 360) {
			curPos.x += opposite;//x++
			curPos.y -= adjacent;//y--
		} else {
			PRINTLINE("Error: invalid angle: " + angle);
			return;
		}
	
		//curPos.rot = angle;
		//?
	} 
	else {
		//turning
		curPos.rot = angle;
	}
}*/

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
	if((abs(a) - abs(b)) < 2) {
		return true;
	} else {
		return false;
	}
}

long PosControl::encoderDifference() {
	return (abs(leftEncoder.diff) - abs(rightEncoder.diff));
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

	PRINTLINE("enc: " << e << " diff: " << diff << " distance: " << distance);
	PRINTLINE("total: " << enc->total << " totalDist: " << enc->totalDist);
}



void PosControl::updateLeft() {
	com->flush(); //unnecessary?
	long enc = com->getEncL();
	updateEncoder(enc, &leftEncoder);
}

void PosControl::updateRight() {
	com->flush(); //unnecessary?
	long enc = com->getEncR();
	updateEncoder(enc, &rightEncoder);
}


double PosControl::timeSinceGoal() {
	return (time(0) - goalPos.changed);
}
