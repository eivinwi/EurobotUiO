/*
* TODO: 
* 	- Button on the bottom, automatically reset to safe bottom-position
* 	- This is at ttyUSB1
* 	-
*/

//?
#define DOWN 1
#define UP 0
#define MIDDLE 2

#define GET 3

//?
#define DELAY 300

#define DIR 8
#define STEP 9

#define BOTTOM_STATE 0
#define MIDDLE_STATE 1
#define TOP_STATE 2

int state; //0, 1 or 2

int full_steps = 1200;
int middle_steps = 600;

void setup() {
	Serial.begin(34800);
	pinMode(DIR, OUTPUT);
	pinMode(STEP, OUTPUT);
	digitalWrite(DIR, DOWN);
	state = BOTTOM_STATE;
}


void loop() {
	if(Serial.available()) { 
    	int inByte = Serial.read();
    	/*if(inByte == UP) {
    		goTop();
    	} else if(inByte == DOWN) {
    		goBottom();
    	} else if(inByte == MIDDLE) {
    		goMiddle();
    	} else if(inByte == GET) {
    		Serial.write(state);
    	}*/
    	if(state == TOP_STATE) {
    		goBottom();
    	} else if(state == BOTTOM_STATE) {
    		goTop();
    	}
  	}
  	//testRoutine();
  	delay(1000);
} 


void testRoutine() {
	goMiddle();
	delay(1000);
	goTop();
	delay(1000);
	goMiddle();
	delay(1000);
	goBottom();
	delay(3000);	
}


void goTop() {
	if(state == BOTTOM_STATE) {
		setDir(UP);
		for(int i = 0; i < full_steps; i++) {
			step();
		}
	} else if(state == MIDDLE_STATE) {
		setDir(UP);
		for(int i = 0; i < middle_steps; i++) {
			step();
		}
	}
	state = TOP_STATE;
}


void goBottom() {
	if(state == TOP_STATE) {
		setDir(DOWN);
		for(int i = 0; i < full_steps; i++) {
			step();
		}
	} else if(state == MIDDLE_STATE) {
		setDir(DOWN);
		for(int i = 0; i < middle_steps; i++) {
			step();
		}
	}	
	state = BOTTOM_STATE;
}


void goMiddle() {
	if(state == BOTTOM_STATE) {
		setDir(UP);
		for(int i = 0; i < middle_steps; i++) {
			step();
		}
	} else if(state == TOP_STATE) {
		setDir(DOWN);		
		for(int i = 0; i < middle_steps; i++) {
			step();
		}
	}
	state = MIDDLE_STATE;
}


void step() {
	digitalWrite(STEP, HIGH);
	digitalWrite(STEP, LOW);
	delayMicroseconds(DELAY);
}


void setDir(int d) {
	digitalWrite(DIR, d);
}