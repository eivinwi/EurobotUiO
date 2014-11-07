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

#define ROTATION_CLOSE_ENOUGH 1
#define POSITION_CLOSE_ENOUGH 1
#define TOO_LONG 20

bool PosControl::controlLoop() {
	if(timeSinceGoal() > TOO_LONG) {
		PRINTLINE("Too long time since goal update, shutting down.");
		stop();
		exit(EXIT_SUCCESS);
	}

	int distR = rotationOffset();
	float distX = distanceFromX(); 
	float distY = distanceFromY();

	PRINTLINE("===== CONTROL-LOOP =====");
	PRINTLINE("goalX: " << goalPos.x << " goalY: " << goalPos.y << " goalR: " << goalPos.rot);	
	PRINTLINE("curX: " << curPos.x << " curY: " << curPos.y << " curR: " << currentRotation());
	PRINTLINE("distX: " << distX << " distY: " << distY << "distR: " << distR);
	PRINTLINE("---\n");

	if(distR > ROTATION_CLOSE_ENOUGH) {
		PRINTLINE("POS: rotating");
		turn(distR);
		updatePosition(TURNING);
	} 

	else if(distX > POSITION_CLOSE_ENOUGH) {
		if(goalPos.rot != 0 && goalPos.rot != 180) {
			PRINTLINE("POS: ERROR; goalX specified, but goalRot is " << goalPos.rot);
			return true;
		} else if(currentRotation() != 0 && currentRotation() != 180) {
			PRINTLINE("POS: ERROR; cur->angle is" << currentRotation() << " should be 0/180. ");
			PRINTLINE("POS: attempting to fix by turning");
			turn(rotationOffset());
		} else {
			PRINTLINE("POS: correct rotation, can drive X.");
			if(distX > 0) {
				com->setSpeedBoth(SPEED_MED_POS);
			} else {
				com->setSpeedBoth(SPEED_MED_NEG);
			}
		}
		updatePosition(DRIVE_X);
	} 	
	else if(distY > POSITION_CLOSE_ENOUGH) {	
		if(goalPos.rot != 90 && goalPos.rot != 270) {
			PRINTLINE("POS: ERROR; goalY specified, but goalRot is " << goalPos.rot);
			return true;
		} else if(currentRotation() != 0 && currentRotation() != 180) {
			PRINTLINE("POS: ERROR; cur->angle is" << currentRotation() << " should be 90/270. ");
			PRINTLINE("POS: attempting to fix by turning");
			turn(rotationOffset());
		} else {
			PRINTLINE("POS: correct rotation, can drive Y.");

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
		return true;
	}
	PRINTLINE("\n");
	return false;
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

	PRINTLINE("POS: updatePos");
	if(action == TURNING) {
		//curX and curY should not really be updated
		// update angle
		curPos.rotation->updateAngle(leftEncoder.diff, rightEncoder.diff);
	}
	else if(action == DRIVE_X) {
		int ediff = encoderDifference();
		if(ediff > 50) {
			PRINTLINE("POS: Warning, large encoder difference(drivex): " << ediff);
		}

		curPos.x += leftEncoder.diffDist;
	}
	else if(action == DRIVE_Y) {
		int ediff = encoderDifference();
		if(ediff > 50) {
			PRINTLINE("POS: Warning, large encoder difference(drivey): " << ediff);
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
	return (goalPos.y - curPos.y);	
}

/* >0 : goal is in positive direction
 * =0 : at exact angle
 * <0 : goal angle in negative direction
 */
int PosControl::rotationOffset() {
	return curPos.rotation->distanceTo(goalPos.rot);
}


//TODO: change restraints
void PosControl::setGoalPos(int x, int y, int rot) {
	PRINTLINE("POS: goal changed: " << x << "," << y << "," << rot);
	if(x*10 > 1000) {
		//
	} else {
		goalPos.x = x*10;
		goalPos.changed = time(0);
		PRINTLINE("new goalX: " << goalPos.x);
	} 
	if(y*10 > 1000) {
		//
	} else {
		goalPos.y = y*10;
		goalPos.changed = time(0);
		PRINTLINE("new goalY: " << goalPos.y);
	} 
	if(goalPos.rot != rot && rot < 360) {
		goalPos.rot = rot;
		goalPos.changed = time(0);
		PRINTLINE("new goalR: " << goalPos.rot);
	} 
	PRINTLINE(" ");
}



// following updatePosition() is more complex, should be used if diagonal travel is wanted
/*
void PosControl::updatePosition() {
	updateLeftEncoder();
	updateRightEncoder();
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


double PosControl::timeSinceGoal() {
	return (time(0) - goalPos.changed);
}

//TODO: turning negative


float PosControl::currentRotation() {
	return curPos.rotation->getAngle();
}