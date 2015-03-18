#include <Servo.h>

#define pos A5
#define ref 12

#define INWARD 45
#define OUTWARD 135
#define STOP 90

#define DELAY_FULL 330
#define DELAY_HALF (DELAY_FULL/2)

Servo grabber;


//int pwmPin = 11;

//at 45: 1000 = 3/4 rotation
//        600 = ~40% rotation

int LIM = 500;
short sample_array[500];
int counter = 0;

void setup() {
	Serial.begin(38400);
	pinMode(pos, INPUT);
	pinMode(ref, OUTPUT);
	digitalWrite(ref, HIGH);
//	pinMode(pwmPin, OUTPUT);	

	grabber.attach(11);
        delay(5000);
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
  /*  if(counter < LIM) {
        sample();	
        grab();
	delay(5000);
        sample();
	release();
	delay(5000);
    }
    else {
      for(int i = 0; i < LIM; i++) {
        //Serial.println(sample_array[i]);      
        delay(1);  
      }
      delay(1000000); 
    }*/
    
    sample();
    delay(40);
}


void grab() {
	grabber.write(INWARD);
        for(int i = 0; i < DELAY_FULL; i+=10) {
           delay(10); 
           sample();
        }

	stop();
	delay(2000);
}

void release() {
	grabber.write(OUTWARD);
        for(int i = 0; i < DELAY_FULL+20; i+=10) {
          sample(); 
          delay(10); 
        }
        
	stop();
	delay(2000);	
}

void stop() {
	grabber.write(STOP);
}


void sample() {
    if(counter < LIM) { 
      //sample_array[counter] = (short) analogRead(pos);
      Serial.println(analogRead(pos));
      counter++;
    }
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
