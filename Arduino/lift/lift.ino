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
#define OPEN 3
#define CLOSE 4

#define GET 5

//?
#define DELAY 300

#define DIR 8
#define STEP 9

//lift states
#define BOTTOM_STATE 0
#define MIDDLE_STATE 1
#define TOP_STATE 2

//gripper states
#define OPEN_STATE 0
#define CLOSED_STATE 1

#define SUCCESS 5

int lift_state; //0, 1 or 2
int gripper_state;//0 or 1

int full_steps = 1200;
int middle_steps = 600;

void setup() {
	Serial.begin(38400);
	pinMode(DIR, OUTPUT);
	pinMode(STEP, OUTPUT);
	digitalWrite(DIR, DOWN);
	lift_state = BOTTOM_STATE;
	gripper_state = OPEN_STATE;
}


void loop() {
	if(Serial.available()) { 
    	int inByte = Serial.read();
    	if(inByte == UP) {
    		goTop();
    		Serial.print(SUCCESS);
    	} else if(inByte == DOWN) {
    		goBottom();
    		Serial.print(SUCCESS);
    	} else if(inByte == MIDDLE) {
    		goMiddle();
    		Serial.print(SUCCESS);
    	} else if(inByte == OPEN) {
    		openGripper();
    		Serial.print(SUCCESS);    		
    	} else if(inByte == CLOSE){
    		closeGripper();
    		Serial.print(SUCCESS);
    	} else if(inByte == GET) {
    		Serial.write(lift_state);
    	}
    	/*if(lift_state == TOP_STATE) {
    		goBottom();
    	} else if(lift_state == BOTTOM_STATE) {
    		goTop();
    	}*/
  	}
  	//testRoutine();
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
	if(lift_state == BOTTOM_STATE) {
		setDir(UP);
		for(int i = 0; i < full_steps; i++) {
			step();
		}
	} else if(lift_state == MIDDLE_STATE) {
		setDir(UP);
		for(int i = 0; i < middle_steps; i++) {
			step();
		}
	}
	lift_state = TOP_STATE;
	delay(500);
}


void goBottom() {
	if(lift_state == TOP_STATE) {
		setDir(DOWN);
		for(int i = 0; i < full_steps; i++) {
			step();
		}
	} else if(lift_state == MIDDLE_STATE) {
		setDir(DOWN);
		for(int i = 0; i < middle_steps; i++) {
			step();
		}
	}	
	lift_state = BOTTOM_STATE;
	delay(500);
}


void goMiddle() {
	if(lift_state == BOTTOM_STATE) {
		setDir(UP);
		for(int i = 0; i < middle_steps; i++) {
			step();
		}
	} else if(lift_state == TOP_STATE) {
		setDir(DOWN);		
		for(int i = 0; i < middle_steps; i++) {
			step();
		}
	}
	lift_state = MIDDLE_STATE;
	delay(500);
}


void step() {
	digitalWrite(STEP, HIGH);
	digitalWrite(STEP, LOW);
	delayMicroseconds(DELAY);
}


void setDir(int d) {
	digitalWrite(DIR, d);
}

void openGripper() {
	if(gripper_state == CLOSED_STATE) {
		//open it
		delay(2000);
	} else {
		//already open
		delay(1000);
	}
}

void closeGripper() {
	if(gripper_state == OPEN_STATE) {
		//close it
		delay(2000);
	} else {
		//already closed
		delay(1000);
	}
}