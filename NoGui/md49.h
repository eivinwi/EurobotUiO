/*  
 * File: md49.h
 * Author: Eivind Wikheim
 *
 * Defines the commands used to communicate with the MD49 motor controller via serial.
 *
 * Documentation: http://www.robot-electronics.co.uk/htm/md49tech.htm
 */

#ifndef MD49_H
#define	MD49_H

#define CLEAR 0x00
							//Recv Send				
#define GET_SPEEDL 0x21		//	2	1
#define GET_SPEEDR 0x22		//	2 	1
#define GET_ENCODERL 0x23	//	2	4
#define GET_ENCODERR 0x24	//	2	4
#define GET_ENCODERS 0x25	//	2	8
#define GET_VOLT 0x26		//	2	1
#define GET_CURRENTL 0x27	//	2	1
#define GET_CURRENTR 0x28	//	2	1
#define GET_VERSION 0x29	//	2	1
#define GET_ACCELERATION 0x2A// 2	1
#define GET_MODE 0x2B		//	2	1
#define GET_VI 0x2C			//	2	3
#define GET_ERROR 0x2D		//	2	1

#define SET_SPEEDL 0x31		// 	3	0	
#define SET_SPEEDR 0x32 	//	3	0
#define SET_ACCELERATION 0x33//	3	0
#define SET_MODE 0x34 		//	3	0

#define RESET_ENCODERS 0x35	//	2	0	
#define DISABLE_REGULATOR 0x36//2	0
#define ENABLE_REGULATOR 0x37 //2	0	
#define DISABLE_TIMEOUT 0x38  //2	0
#define ENABLE_TIMEOUT 0x39  // 2	0

/* 
Speeds are calculated assuming mode=0
This means:
	0=full reverse
	128= stop
	255=full forward
*/
/*
#define SPEED_MAX_NEG 0
#define SPEED_MED_NEG 64
#define SPEED_SLOW_NEG 96
#define SPEED_STOP 128
#define SPEED_SLOW_POS 160
#define SPEED_MED_POS 192
#define SPEED_MAX_POS 255
*/
#define SPEED_MAX_NEG 255
#define SPEED_MED_NEG 192
#define SPEED_SLOW_NEG 160
#define SPEED_STOP 128
#define SPEED_SLOW_POS 196
#define SPEED_MED_POS 64
#define SPEED_MAX_POS 0
//*/

#define POS_DIR (-1) //-1

#endif /* MD49_H */