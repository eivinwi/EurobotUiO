#include "robotsim.h"


							//Recv Send				
#define GET_SPEED1 0x21		//	2	1
#define GET_SPEED2 0x22		//	2 	1
#define GET_ENCODER1 0x23	//	2	4
#define GET_ENCODER2 0x24	//	2	4
#define GET_ENCODERS 0x25	//	2	8
#define GET_VOLTS 0x26		//	2	1
#define GET_CURRENT1 0x27	//	2	1
#define GET_CURRENT2 0x28	//	2	1
#define GET_VERSION 0x29	//	2	1
#define GET_ACCELERATION 0x2A// 2	1
#define GET_MODE 0x2B		//	2	1
#define GET_VI 0x2C			//	2	3
#define GET_ERROR 0x2D		//	2	1

#define SET_SPEED1 0x31		// 	3	0	
#define SET_SPEED2 0x32 	//	3	0
#define SET_ACCELERATION 0x33//	3	0
#define SET_MODE 0x34 		//	3	0

#define RESET_ENCODERS 0x35	//	2	0	
#define DISABLE_REGULATOR 0x36//2	0
#define ENABLE_REGULATOR 0x37 //2	0	
#define DISABLE_TIMEOUT 0x38  //2	0
#define ENABLE_TIMEOUT 0x39  // 2	0

RobotSim::RobotSim() {
	port = new Serial;
	speed1 = 255;
	speed2 = 255;
	acceleration = 5;
	encoder1 = 0;
	encoder2 = 0;
	volt = 24;
	current1 = 2;
	current2 = 3;
	version = 2;
	mode = 0;
	error = 0;
	regulator = 1;
	timeout = 0;
	time1 = 0;
	time2 = 0;
}

RobotSim::~RobotSim() {
}

//uint8_t speed1 = 0x8F;
//uint8_t speed2 = 0xFF;
//uint8_t acceleration = 0x35; //5

void RobotSim::readSerial() {
	if(port->available()) {
		uint8_t first_byte = port->readNoWait();

		if(first_byte == 0x00) { //sync_byte
			uint8_t sec_byte = port->readNoWait();

			switch(sec_byte){		  
				//2 bytes
		        case GET_SPEED1:
		        	port->write(speed1);
		        	break;
				case GET_SPEED2:
					port->write(speed2);
		        	break;
				case GET_ENCODER1:
					writeEnc(time1);
					break;
				case GET_ENCODER2:
					writeEnc(time2);
		        	break;
				case GET_ENCODERS:
					writeEnc(time1);
					writeEnc(time2);
		        	break;
				case GET_VOLTS:
					port->write(volt);
		        	break;
				case GET_CURRENT1:
					port->write(current1);
		        	break;
				case GET_CURRENT2:
					port->write(current2);
		        	break;
				case GET_VERSION:
					port->write(version);
		        	break;
				case GET_ACCELERATION:
					port->write(acceleration);
		        	break;
				case GET_MODE:
					port->write(mode);
		        	break;
				case GET_VI:
					port->write(volt);
					port->write(current1);
					port->write(current2);
		        	break;
				case GET_ERROR:
					port->write(error);
		        	break;

		        //3 bytes
				case SET_SPEED1:
					speed1 = port->readNoWait();
					time(&time1);		//store time speed was set, for use in encoder calculations
		        	break;
				case SET_SPEED2:
					speed2 = port->readNoWait();
					time(&time2);		//store time speed was set, for use in encoder calculations
		        	break;
				case SET_ACCELERATION:
					acceleration = port->readNoWait();
		        	break;
				case SET_MODE:
					mode = speed1 = port->readNoWait();
		        	break;

		        //2 bytes
				case RESET_ENCODERS:
					encoder1 = 0;
					encoder2 = 0;
		        	break;
				case DISABLE_REGULATOR:
					regulator = 0;
		        	break;
				case ENABLE_REGULATOR:
					regulator = 1;
		        	break;
				case DISABLE_TIMEOUT:
					timeout = 0;
		        	break;
				case ENABLE_TIMEOUT:
					timeout = 1;
		        	break;
			    default:
			    	std::cout << "Invalid serial-cmd in SIM: " << sec_byte << '\n';
			    	break;
			}
		}
	}
}


//RPM = 122
#define RPM 122
#define RPS 2
void RobotSim::writeEnc(time_t encTime) {
	time_t now;
	time(&now);
	double timePassed = difftime(encTime, now);
	double rot = (timePassed)/RPS;
	double encoderDiff = rot*980;
	long enc = (long) encoderDiff;

	port->write((int) ((enc >> 24) & 0xff));
	port->write((int) ((enc >> 16) & 0xff));
	port->write((int) ((enc >> 8) & 0xff));
	port->write((int) (enc & 0xff));
}
/*
 * Encoder counts: 980 per output shaft turn
 * Wheel diameter: 120mm
 * Wheel circumference: 377mm
 * Distance per count: 0.385mm
 */