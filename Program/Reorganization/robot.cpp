#include "robot.h"


Robot::Robot(MotorCom *m) {
	mcom = m;
}



void Robot::turnCw(float degrees);
void Robot::turnCcw(float degrees);
void Robot::driveForward(float dist);
void Robot::driveBackward(float dist);

//cos in degree? probably correct
//TODO: <1
float Robot::currentX(auto t) {
	if(abs(velocity_l - velocity_r) < 1) {
		//driving straight
		return 	
	}

	float x_t = 
		x_0 + 
		((W_b * (velocity_l + velocity_r)) / (2 * (velocity_l - velocity_r))) 
		* ((sin_d(velocity_r - velocity_l) * t) / (W_b + angle_0)) - sin_d(angle_0);
	return x_t;
}


float Robot::currentY(auto t) {
	float y_t = 
		y_0 + 
		((W_b * (velocity_r + velocity_l)) / (2 * (velocity_r - velocity_l))) 
		* ((cos_d(velocity_r - velocity_l) * t) / (W_b + angle_0)) - cos_d(angle_0);
	return y_t;
}




float Robot::currentY(auto t);
float Robot::currentAngle(auto t);
