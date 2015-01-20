#include "liftcom.h"

LiftCom::LiftCom() {
    strcpy(serial_port, "ttyUSB1");
}


LiftCom::~LiftCom() {
}


void LiftCom::setSerialPort(const char *s) {
    LOG(DEBUG) << "[LIFT] serial_port set to: " << s;
	strcpy(serial_port, s);
}


void LiftCom::startSerial() {        
	if(strcmp(serial_port, "") == 0) {
        LOG(WARNING) << "[LIFT] 	Error, empty serial_port. Setting to /dev/ttyUSB1";
        strcpy(serial_port, "ttyUSB1");
    }     
    LOG(INFO) << "[LIFT] 	Starting serial at: " << serial_port;
	port = new Serial(serial_port);
}


void LiftCom::goTo(int p) {
	if(p == TOP) {
		goToTop();
	} else if(p == MIDDLE) {
		goToMiddle();
	} else if(p == BOTTOM) {
		goToBottom();
	}
}


void LiftCom::goToTop() {
	LOG(INFO) << "[LIFT] goToTop, previous is: " << current_pos;
	writeToSerial(ARD_TOP);
	current_pos = TOP;
}


void LiftCom::goToMiddle() {
	LOG(INFO) << "[LIFT] goToMiddle, previous is: " << current_pos;
	writeToSerial(ARD_MIDDLE);
	current_pos = MIDDLE;
}


void LiftCom::goToBottom() {
	LOG(INFO) << "[LIFT] goToBottom, previous is: " << current_pos;
	writeToSerial(ARD_BOTTOM);
	current_pos = BOTTOM;
}


uint8_t LiftCom::getPosition() {
	/*writeToSerial(GET);
	usleep(50);
	uint8_t pos = readFromSerial();
	LOG(INFO) << "Lift is at: " << pos;
	return pos;*/
	return current_pos;
}


bool LiftCom::test() {
	if(port->available()) {
		return true;
	} else {
		usleep(1000);
		if(port->available()) {
			return true;
		}
	}
	return false;
}


//used to simulate lift while testing
void LiftCom::setCurrentPos(int p) {
	current_pos = p;
}


/*** Private Functions ***/


void LiftCom::writeToSerial(uint8_t a) {
	port->write(a);
}


void LiftCom::writeToSerial(int a) {
	port->write(a);
}


uint8_t LiftCom::readFromSerial() {
	return port->read();
}
