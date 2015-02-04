#include <Servo.h>

#define INWARD 45
#define OUTWARD 135
#define STOP 90

#define DELAY_FULL 330
#define DELAY_HALF (DELAY_FULL/2)

Servo grabber;

//int pwmPin = 11;

//at 45: 1000 = 3/4 rotation
//        600 = ~40% rotation

void setup() {
	Serial.begin(38400);
//	pinMode(pwmPin, OUTPUT);	

	grabber.attach(11);
}

void loop() {
/*	analogWrite(pwmPin, 160);
	Serial.println("250");
	delay(2000);
	analogWrite(pwmPin, 140);
	Serial.println("100");
	delay(2000);*/

//	incrPwm();
	testRoutine();
}

void testRoutine() {
	grab();
	delay(3000);
	release();
	delay(8000);	
}


void grab() {
	grabber.write(INWARD);
	delay(DELAY_FULL);
	stop();
	delay(2000);
}

void release() {
	grabber.write(OUTWARD);
	delay(DELAY_FULL);
	stop();
	delay(2000);	
}

void stop() {
	grabber.write(STOP);
}

void incrPwm() {
	for(int i=0; i <= 180; i+=45){
	    Serial.print("Setting pwm = ");
	    Serial.println(i);
	   	grabber.write(i);
		delay(2000);
	}
}


// 1-200 forward??
// 200-255 backward??