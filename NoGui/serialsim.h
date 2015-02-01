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
 * - Warning: poor code
 * -fix calculation of fake encoder values
 * 	  - readLong
 */


#ifndef SERIALSIM_H
#define	SERIALSIM_H

#include "md49.h"
#include "printing.h"
#include <iostream>
#include <stdint.h>
#include <time.h>

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

	// Simulates writing to serial. Uses arg to decide what to make ready for reads
	void write(char arg);
	void write(uint8_t arg);
	void write(int arg);

	// Simulates reading from serial. What to read needs to be be prepared after each write. 
	uint8_t readNoWait();
	uint8_t read();

	//TODO: incomplete
	long readLong();

	// Dummy method that dimulates clearing serial. 
	void printAll();

private:
	// Read methods that simulates reading from serial
	uint8_t readVi();
	uint8_t readEncL();
	uint8_t readEncR();
	uint8_t readEncs();	

	// Calculate appromiately correct encoder values based on time since speed was changed.
	void calculateEncL();
	void calculateEncR();

	int speedL, speedR;
	int acceleration;
	long encoderL, encoderR;
	int currentL, currentR;

	int volt, version, mode, error;
	int regulator, timeout;

	time_t timeL, timeR;

	int readCounter;
	bool synced;
	uint8_t prev_cmd;
	uint8_t encL[4];
	uint8_t encR[4];
};

#endif /* SERIALSIM_H */