#ifndef SPEEDCONTROL_H
#define	SPEEDCONTROL_H

#define LEFT 1
#define RIGHT 2
#define ENC_BUFF_SIZE 8


#include "Serial.h"
#include <stdint.h>

/* Structs */
typedef struct Motor {
	uint8_t id;
	uint8_t dir;
	uint8_t speed;	
} Motor;


class SpeedControl {
public:
	SpeedControl();
	~SpeedControl();
	void setAccel(int acc);
	void setSpeed1(uint8_t speed);
	void setSpeed2(uint8_t speed);
	void setSpeedBoth(uint8_t speed);
	uint8_t getSpeed1();
	uint8_t getSpeed2();
	void resetEncoders();
	long getEnc1();
	long getEnc2();
	void getEncoders();
	uint8_t getVoltage();
	uint8_t getVersion();
	void setMode(int mode);
	void enableReg(bool b);
	void enableTimeout(bool b);
	void flush();
	void sync();
	void action(uint8_t a); 

private:
	Serial *port;
	Motor *right; 
	Motor *left;

	char enc_buffer[ENC_BUFF_SIZE];

	long prev_enc1;
	long prev_enc2;

	const static uint8_t CLEAR = 0x00;
	const static uint8_t RESET_ENC = 0x35;

	const static uint8_t GET_SPEED1 = 0x21;
	const static uint8_t GET_SPEED2 = 0x22;
	const static uint8_t GET_ENC1 = 0x23;
	const static uint8_t GET_ENC2 = 0x24;
	const static uint8_t GET_ENCODERS = 0x25;
	const static uint8_t GET_ACCEL = 0x33;
	const static uint8_t GET_VOLT = 0x26;
	const static uint8_t GET_VERSION = 0x29;

	const static uint8_t SET_SPEED1 = 0x31;
	const static uint8_t SET_SPEED2 = 0x32; //alternatively TURN
	const static uint8_t SET_ACCEL = 0x2A;
	const static uint8_t SET_MODE = 0x24;

	const static uint8_t REG_ENABLE = 0x37;
	const static uint8_t REG_DISABLE = 0x36;

	const static uint8_t TIMEOUT_ENABLE = 0x39;
	const static uint8_t TIMEOUT_DISABLE = 0x38;
};

#endif /* SPEEDCONTROL_H */