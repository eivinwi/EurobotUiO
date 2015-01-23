#include "liftcom.h"

LiftCom::LiftCom(std::string serial) {
    serial_port = serial;
    lift_pos = BOTTOM;
}


LiftCom::~LiftCom() {
}


void LiftCom::startSerial() {        
    if(serial_port == "") { //should never happen
        LOG(WARNING) << "[LIFT] 	Error, empty serial_port. Setting to /dev/ttyUSB1";
        serial_port = "ttyUSB1";
    }     
    LOG(INFO) << "[LIFT] 	 Starting serial at: " << serial_port;
	port = new Serial(serial_port);
}


void LiftCom::goTo(int p) {
	switch(p) {
		case TOP: 
			goToTop();
			break;
		case MIDDLE:
			goToMiddle();
			break;
		case BOTTOM:
			goToBottom();
			break;
		case OPEN:
			openGrabber();
			break;
		case CLOSE:
			closeGrabber();
			break;
		case GET:
			getPosition();
			break;
		default:
			LOG(INFO) << "[LIFT] invalid goto-command.";
			break;

	}
}


void LiftCom::goToTop() {
	LOG(INFO) << "[LIFT] goToTop, previous is: " << lift_pos;
	writeToSerial(ARD_TOP);
	lift_pos = TOP;
}


void LiftCom::goToMiddle() {
	LOG(INFO) << "[LIFT] goToMiddle, previous is: " << lift_pos;
	writeToSerial(ARD_MIDDLE);
	lift_pos = MIDDLE;
}


void LiftCom::goToBottom() {
	LOG(INFO) << "[LIFT] goToBottom, previous is: " << lift_pos;
	writeToSerial(ARD_BOTTOM);
	lift_pos = BOTTOM;
}


void LiftCom::openGrabber() {
	LOG(INFO) << "[LIFT] openGrabber, previous is: " << grabber_pos;
	writeToSerial(OPEN);
	grabber_pos = OPEN;
}


void LiftCom::closeGrabber() {
	LOG(INFO) << "[LIFT] closeGrabber, previous is: " << grabber_pos;	
	writeToSerial(OPEN);
	grabber_pos = CLOSE;
}


uint8_t LiftCom::getPosition() {
	/*writeToSerial(GET);
	usleep(50);
	uint8_t pos = readFromSerial();
	LOG(INFO) << "Lift is at: " << pos;
	return pos;*/
	return lift_pos;
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
	lift_pos = p;
}


bool LiftCom::waitForResponse() {
	auto t_start = std::chrono::high_resolution_clock::now();
    auto t_end = std::chrono::high_resolution_clock::now();    

	do {
		if(port->available()) {
			uint8_t resp = readFromSerial();
			LOG(INFO) << "[LIFT resp after: "  << (std::chrono::duration<double, std::milli>(t_end-t_start).count());

			if(resp == SUCCESS) {
				return true;
			} 	
			else {
				return false;
			}
		}
    } while ((std::chrono::duration<double, std::milli>(t_end-t_start).count()) < 10); //time needs testing
    return false;
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
