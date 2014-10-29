#ifndef SERIALSIM_H
#define	SERIALSIM_H

#include "serial.h"
#include <stdint.h>
#include <time.h>


							//Recv Send				
#define GET_SPEED1 0x21		//	2	1
#define GET_SPEED2 0x22		//	2 	1
#define GET_ENCODER1 0x23	//	2	4
#define GET_ENCODER2 0x24	//	2	4
#define GET_ENCODERS 0x25	//	2	8
#define GET_VOLTS 0x26		//	2	1
#define GET_CURRENT1 0x27	//	2	1
#define GET_CURRENT2 0x28	//	2	1
#define GET_VERSION 0x29	//	2	1
#define GET_ACCELERATION 0x2A// 2	1
#define GET_MODE 0x2B		//	2	1
#define GET_VI 0x2C			//	2	3
#define GET_ERROR 0x2D		//	2	1

#define SET_SPEED1 0x31		// 	3	0	
#define SET_SPEED2 0x32 	//	3	0
#define SET_ACCELERATION 0x33//	3	0
#define SET_MODE 0x34 		//	3	0

#define RESET_ENCODERS 0x35	//	2	0	
#define DISABLE_REGULATOR 0x36//2	0
#define ENABLE_REGULATOR 0x37 //2	0	
#define DISABLE_TIMEOUT 0x38  //2	0
#define ENABLE_TIMEOUT 0x39  // 2	0


class SerialSim {
public:
	SerialSim();
	~SerialSim();
	void readSerial();
	void writeEnc(time_t encTime);

private:
	Serial *port;


	int speed1;
	int speed2;
	int acceleration;
	long encoder1;
	long encoder2;
	int volt;
	int current1;
	int current2;
	int version;
	int mode;
	int error;

	int regulator;
	int timeout;

	time_t time1;
	time_t time2;
};

#endif /* SERIALSIM_H */