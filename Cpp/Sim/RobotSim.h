#ifndef ROBOTSIM_H
#define	ROBOTSIM_H

#include "Serial.h"
#include <stdint.h>
#include <time.h>


class RobotSim {
public:
	RobotSim();
	~RobotSim();
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

#endif /* ROBOTSIM_H */