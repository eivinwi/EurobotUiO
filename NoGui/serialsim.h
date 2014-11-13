/* 
 * File: serialsim.h
 * Author: Eivind Wikheim
 *
 * Simulates a serial-connection to the MD49 motor controller.
 * 
 * Relatively realistic encoder-values are calculated based on set motorspeed and
 * time since last encoder-request (or since program start).
 *
 * TODO: 
 * -fix calculation of fake encoder values
 * 	  - readLong
 */


#ifndef SERIALSIM_H
#define	SERIALSIM_H

#include "printing.h"
#include <iostream>
#include <stdint.h>
#include <time.h>
#include "md49.h"

/*
 * Encoder counts: 980 per output shaft turn
 * Wheel diameter: 120mm
 * Wheel circumference: 377mm
 * Distance per count: 0.385mm
 */
#define RPM 122
#define RPS 2


class SerialSim {
public:
	SerialSim();
	~SerialSim();
	void write(char arg);
	void write(uint8_t arg);
	void write(int arg);
	uint8_t readNoWait();
	uint8_t read();
	long readLong();
	void printAll();

private:
	uint8_t readVi();
	uint8_t readEncL();
	uint8_t readEncR();
	uint8_t readEncs();	
	void calculateEncL();
	void calculateEncR();


	int speedL;
	int speedR;
	int acceleration;
	long encoderL;
	long encoderR;
	int volt;
	int currentL;
	int currentR;
	int version;
	int mode;
	int error;

	int regulator;
	int timeout;

	time_t timeL;
	time_t timeR;


	int readCounter;

	bool synced;
	uint8_t prev_cmd;
	uint8_t encL[4];
	uint8_t encR[4];
};

#endif /* SERIALSIM_H */