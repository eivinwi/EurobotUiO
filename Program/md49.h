/*
 *  File: md49.h
 *  Author: Eivind Wikheim
 *
 *	md49 defines the commands used to communicate with the MD49 motor controller via serial,
 *	as well as general defines concerning position control.
 *	Documentation: http://www.robot-electronics.co.uk/htm/md49tech.htm
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

#ifndef MD49_H
#define	MD49_H

#define CLEAR 0x00
//					bytes:	//Recv Send				
/*#define GET_SPEEDR 0x21		//	2	1
#define GET_SPEEDL 0x22		//	2 	1
#define GET_ENCODERR 0x23	//	2	4
#define GET_ENCODERL 0x24	//	2	4
#define GET_CURRENTR 0x27	//	2	1
#define GET_CURRENTL 0x28	//	2	1
#define SET_SPEEDR 0x31		// 	3	0	
#define SET_SPEEDL 0x32 	//	3	0
*/
#define GET_SPEEDL 0x21		//	2	1
#define GET_SPEEDR 0x22		//	2 	1
#define GET_ENCODERL 0x23	//	2	4
#define GET_ENCODERR 0x24	//	2	4
#define GET_CURRENTL 0x27	//	2	1
#define GET_CURRENTR 0x28	//	2	1
#define SET_SPEEDL 0x31		// 	3	0	
#define SET_SPEEDR 0x32 	//	3	0

#define GET_ENCODERS 0x25	//	2	8
#define GET_VOLT 0x26		//	2	1
#define GET_VERSION 0x29	//	2	1
#define GET_ACCELERATION 0x2A// 2	1
#define GET_MODE 0x2B		//	2	1
#define GET_VI 0x2C			//	2	3
#define GET_ERROR 0x2D		//	2	1

#define SET_ACCELERATION 0x33//	3	0
#define SET_MODE 0x34 		//	3	0

#define RESET_ENCODERS 0x35	//	2	0	
#define DISABLE_REGULATOR 0x36//2	0
#define ENABLE_REGULATOR 0x37 //2	0	
#define DISABLE_TIMEOUT 0x38  //2	0
#define ENABLE_TIMEOUT 0x39  // 2	0

/* 
Speeds for mode=0. 
See MD49 documentation for explanation
*/
#define SPEED_MAX_NEG 225 //235, 255
#define SPEED_MED_NEG 192
#define SPEED_SLOW_NEG 160
#define SPEED_STOP 128
#define SPEED_SLOW_POS 96
#define SPEED_MED_POS 64
#define SPEED_MAX_POS 30 //20, 0


/*#define SPEED_MAX_POS 225 //235, 255
#define SPEED_MED_POS 192
#define SPEED_SLOW_POS 160
#define SPEED_STOP 128
#define SPEED_SLOW_NEG 96
#define SPEED_MED_NEG 64
#define SPEED_MAX_NEG 30 //20, 0
*/

#define POS_DIR (-1) //-1 CHECK: is this necessary?

#define SLOWDOWN_MAX_DIST 120
#define SLOWDOWN_MED_DIST 60
#define SLOWDOWN_DISTANCE_ROT 10

#define ROTATION_CLOSE_ENOUGH 1.0
#define POSITION_CLOSE_ENOUGH 2.0

//max increase from one encoder-reply to the next.
#define MAX_ENC_INCR 980 // = one rotation. 

//max difference between L and R encoder diffs.
#define MAX_ENC_DIFF 440 // == half rotation. Should possibly be smaller
#define TOO_LONG 20             //timeout in ms. Currently not in use (?)

// Game area resolution in mm:
#define XRES 30000
#define YRES 20000


/*
 * Encoder counts: 980 per output shaft turn
 * Wheel diameter: 120mm
 * Wheel circumference: 377mm
 * Distance per count: 0.385mm  
 */
//#define ENCODER_CONSTANT 0.40	//0.385
#define ENCODER_CONSTANT 0.327	//0.385
#define ENC_PER_DEGREE 8.05 //8.55 		//6.2 		//7.5

//Delays for serial communications
#define MAX_WAIT 2000
#define SERIAL_DELAY 500


#define TURNING 0
#define DRIVE_X 1
#define DRIVE_Y 2

#endif /* MD49_H */


 /*
 * Encoder counts: 980 per output shaft turn
 * Wheel diameter: 102mm
 * Wheel circumference: 320mm
 * Distance per count: 0.327mm
 */