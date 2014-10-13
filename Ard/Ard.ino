/* DFRduino pins
d4: motor2 direction 
d5: motor2 pwm
d6: motor1 pwm
d7: motor1 direction

motor1=left
motor2=right
*/
#define LEFT 0
#define RIGHT 1
#define VOLT_MIN 800
#define VOLT_MAX 1600

int RIGHT_DIR = 4;
int RIGHT_PWM = 5;
int LEFT_PWM = 6;
int LEFT_DIR = 7;

int LEFT_A = 2;
int LEFT_B = 8;
int RIGHT_A = 3;
int RIGHT_B = 9;

int led = 13;

int present_speed_left;
int present_dir_left;
int present_speed_right;
int present_dir_right;


char serial_data[10];

void setup() {
	Serial.begin(9600); //(115200);
	pinMode(led, OUTPUT);
	pinMode(RIGHT_DIR, OUTPUT);
	pinMode(RIGHT_PWM, OUTPUT);
	pinMode(LEFT_PWM, OUTPUT);
	pinMode(LEFT_DIR, OUTPUT);

	pinMode(LEFT_A, INPUT);
	pinMode(LEFT_B, INPUT);
	pinMode(RIGHT_A, INPUT);
	pinMode(RIGHT_B, INPUT);

	present_speed_left = 0;
	present_dir_left = 0;
	present_speed_right = 0;
	present_dir_right = 0;
	stop();
}

void loop() {
	readInstructions();

	/*writeSpeed();
	if(Serial.available() > 0) {
		readInstructions();
	}*/

//	setSpeed(0, 100, 0, 100);
//	delay(3000);
	setSpeed(0, 0, 0, 0);
	delay(3000);
//	setSpeed(1, 100, 1, 100);
//	delay(6000);
}

void stop() {
	setSpeed(0, 0, 0, 0);
}

void setSpeed(int dir_left, int pwm_left, int dir_right, int pwm_right) {
	setSpeedLeft(dir_left, pwm_left);
	setSpeedRight(dir_right, pwm_right);
}

void setSpeedLeft(int dir_left, int pwm_left) {
	present_dir_left = dir_left;
	present_speed_left = pwm_left; 
	digitalWrite(LEFT_DIR, dir_left);
	analogWrite(LEFT_PWM, pwm_left);
}

void setSpeedRight(int dir_right, int pwm_right) {
	present_dir_right = dir_right;
	present_speed_right = pwm_right; 
	digitalWrite(RIGHT_DIR, dir_right);
	analogWrite(RIGHT_PWM, pwm_right);
}




/* INPUT: 
 * change one:
 * |motor|dir|pwm2|pwm1|pwm0| = 5byte. Motor is 0=l 1=r
 * 
 * change both:
 * |Ldir|Rdir|Lpwm2|Lpwm1|Lpwm0|Rpwm2|Rpwm1|Rpwm0| = 8byte
*/



void readInstructions() {
	if(Serial.available() >= 8) { //change both speeds
		Serial.readBytes(serial_data, 8);
		uint8_t dir_l = serial_data[0];
		uint8_t dir_r = serial.data[1];
		char left_pwm[3];
		char right_pwm[3];
		for(int i = 0; i < 3; i++) {
			left_pwm[i] = serial_data[i+2];
			right_pwm[i] = serial_data[i+5];
		}
		int pwm_l = atoi(left_pwm);
		int pwm_r = atoi(right_pwm);
		setSpeed(dir_l, pwm_l, dir_r, pwm_r);
	} 

	else if(Serial.available() >= 5) {//change one speed
		Serial.readBytes(serial_data, 4);
		uint8_t motor = serial_data[0];
		uint8_t dir = serial_data[1];
		char pwmc[3];
		for(int i = 0; i < 3; i++){
		    pwmc[i] = serial_data[i+2];
		}
		int pwm = atoi(pwmc);
		if(motor == LEFT) {
			setSpeedLeft(dir, pwm);
		} else {
			setSpeedRight(dir, pwm);
		}
	}
}

void writeSpeed() {
	//Serial.write("s");
	Serial.print(present_dir_left);
	Serial.print(present_speed_left);
	Serial.print(","); 
	Serial.print(present_dir_right);
	Serial.print(present_speed_right);
	Serial.print('\n');
}

/* DEBUG */
void blinkLed() {
	digitalWrite(led, HIGH);
	delay(1000);
	digitalWrite(led, LOW);
	delay(1000);
}