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
#ifndef ROBOT_H
#define	ROBOT_H

#include "md49.h"
#include "motorcom.h"
#include <math.h>
#include <chrono.h>

//These factors are the main causes of systematic odometry errors
#define W_r 102 //wheel radius in mm
#define W_b 282 //wheelbase in mm

#define ENCODER_CONSTANT 0.327	//0.385
#define ENC_PER_DEGREE 8.05 //8.55 		//6.2 		//7.5

//SHOULD BE TWEAKED:
//max increase from one encoder-reply to the next.
#define MAX_ENC_INCR 980 // = one rotation. 
//max difference between L and R encoder diffs.
#define MAX_ENC_DIFF 440 // == half rotation. Should possibly be smaller

#define ACCELERATION 5 //acceleration constant, should be measured

enum class State {IDLE, MOVING, WORKING}; //workig = idle + using actuators



class Robot {
public:
	Robot(MotorCom *m)

	//actions:
	void turnCw(float degrees);
	void turnCcw(float degrees);
	void driveForward(float dist);
	void driveBackward(float dist);


	float currentX(auto t);
	float currentY(auto t);
	float currentAngle(auto t);

private:	
	MotorCom *mcom;

	//set at start of action. Used as starting point when calculation new position
	float x_0;
	float y_0;
	float angle_0;
	auto t_0;


	float x_local;
	float y_local;
	float angle_local;
	float angle_compass;

	float x_abs;
	float y_abs;
	float angle_abs;
	auto timestamp_abs; //time when last timestamp was updated, to avoid using outdated information


	int velocity_l; //left wheel rotational speed
	int velocity_r; //right speed rotational speed


	State state = State::IDLE;
};

#endif /* ROBOT_H */