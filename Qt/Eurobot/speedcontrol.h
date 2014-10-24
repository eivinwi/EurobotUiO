#ifndef SPEEDCONTROL_H
#define	SPEEDCONTROL_H

#define LEFT 1
#define RIGHT 2
#define ENC_BUFF_SIZE 8


#include "serial.h"
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
    void setAcceleration(int acc);
    void setSpeedL(uint8_t speed);
    void setSpeedR(uint8_t speed);
    void setSpeedBoth(uint8_t speed);
    uint8_t getSpeedL();
    uint8_t getSpeedR();
    uint8_t getAcceleration();
    void resetEncoders();
    long getEnc1();
    long getEnc2();
    void getEncoders();
    uint8_t getVoltage();
    long getVi();
    uint8_t getVersion();
    uint8_t getError();
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

    const static uint8_t GET_SPEEDL = 0x21;
    const static uint8_t GET_SPEEDR = 0x22;
    const static uint8_t GET_ENC1 = 0x23;
    const static uint8_t GET_ENC2 = 0x24;
    const static uint8_t GET_ENCODERS = 0x25;
    const static uint8_t GET_VOLT = 0x26;
    const static uint8_t GET_CURRENTL = 0x27;
    const static uint8_t GET_CURRENTL = 0x28;
    const static uint8_t GET_VERSION = 0x29;
    const static uint8_t GET_ACCEL = 0x2A;
    //mode
    const static uint8_t GET_VI = 0x2C;

    const static uint8_t SET_SPEEDL = 0x31;
    const static uint8_t SET_SPEEDR = 0x32; //alternatively TURN
    const static uint8_t SET_ACCEL = 0x33;
    const static uint8_t SET_MODE = 0x34;

    const static uint8_t REG_ENABLE = 0x37;
    const static uint8_t REG_DISABLE = 0x36;

    const static uint8_t TIMEOUT_ENABLE = 0x39;
    const static uint8_t TIMEOUT_DISABLE = 0x38;
};

#endif /* SPEEDCONTROL_H */
