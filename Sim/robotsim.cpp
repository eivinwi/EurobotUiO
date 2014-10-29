#include "robotsim.h"

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