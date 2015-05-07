#include <sstream>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <chrono>
#include <SerialStream.h>
using namespace LibSerial ;

// baud should be set to  117len47.1, which is 17 = 0x12
	
#define ACTION_DELAY 10000
#define SERIAL_DELAY 500

#define GRIP_ID 1
#define LIFT_ID 2

#define TOP_POS 20
#define MIDDLE_POS 490
#define BOTTOM_POS 980

#define OPEN_POS 230
#define CLOSED_POS 530


SerialStream serial;

void writeToSerial(uint8_t b[], int len) {
	for(int i = 0; i < len; i++) {
		serial << b[i];
	}
	//serial << std::endl;
}


// adds checksum and length bytes to the ASCII byte packet
// input needs to be of length 4, with the last 2 places empty
void addChecksumAndLength(uint8_t b[], int len) {
  // adding length
  b[3] = (uint8_t) (len - 4);

  // finding sum
  int counter = 0;
  for (int i=2; i<(len-1); i++)
  {
    int tmp = (int)b[i];
    if (tmp < 0) 
      tmp = tmp + 256 ;
    counter = counter + tmp;
  }

  // inverting bits
  counter = ~counter;
  // int2byte 
  counter = counter & 255;

  // adding checkSum
  b[len-1] = (uint8_t) counter;
//  return b;
}


void sendCmd(int id, int cmd) {
	uint8_t b[] = {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

	b[2] = (uint8_t) id;
	b[4] = (uint8_t) cmd;

	addChecksumAndLength(b, 6);

//	serial.write(b);
	writeToSerial(b, 6);
}



// Writes 0<val<255 to register "regNo" in servo "id" 
void setReg1(int id, int regNo, int val)
{
  uint8_t b[] = {0xFF, 0xFF, 0, 0, 3, 0, 0, 0}; 

  b[2] = (uint8_t) id;
  b[5] = (uint8_t) regNo;
  b[6] = (uint8_t) val;

  addChecksumAndLength(b, 8); 
  writeToSerial(b, 8);
}


// Writes 0<val<1023 to register "regNoLSB/regNoLSB+1" in servo "id"
void setReg2(int id, int regNoLSB, int val)
{
  uint8_t b[] = {0xFF, 0xFF, 0, 0, 3, 0, 0, 0, 0}; 
  b[2] = (uint8_t) id;
  b[5] = (uint8_t) regNoLSB;
  b[6] = (uint8_t) ( val & 255 );
  b[7] = (uint8_t) ( (val >> 8) & 255 );

  addChecksumAndLength(b, 9); 


std::cout << "Byte sent: {"
	<< (int) b[0] << ", " << (int) b[1] << ", " << (int) b[2]  << ", " << (int) b[3] << ", " 	
	<< (int) b[4] << ", " << (int) b[5] << ", " << (int) b[6] << ", " << (int) b[7] << ", " << (int) b[8] << "]" << std::endl;
 writeToSerial(b, 9);
}


// Read from register, status packet printout is handled by serialEvent()
void regRead(int id, int firstRegAdress, int noOfBytesToRead)
{
 // println(" "); // console newline before serialEvent() printout

  uint8_t b[] = {0xFF, 0xFF, 0, 0, 2, 0X2B, 0X01, 0}; 

  b[2] = (uint8_t) id;
  b[5] = (uint8_t) firstRegAdress;
  b[6] = (uint8_t) noOfBytesToRead;

  addChecksumAndLength(b, 8); 

std::cout << "Byte sent: {"
  << (int) b[0] << ", " << (int) b[1] << ", " << (int) b[2]  << ", " << (int) b[3] << ", "  
  << (int) b[4] << ", " << (int) b[5] << ", " << (int) b[6] << ", " << (int) b[7] << "]" << std::endl;


  writeToSerial(b, 8);
}


void setSpeed(int id, int speed) {
	if(speed >= 0 && speed < 1024) {
		setReg2(id, 32, speed);
	}
	usleep(ACTION_DELAY);
}

void setMaxTorque(int id, int torque) {
	if(torque >= 100 && torque < 1024) {
		setReg2(id, 14, torque);
	}
	usleep(ACTION_DELAY);
}

int led = 0;
void toggleLed(int id) {
	if(led == 1) led = 0;
	else led = 1;

	setReg1(id, 25, led);
	usleep(ACTION_DELAY);
}


void setPosition(int id, int angle) {
	std::cout << "SetPosition(" << id << "): " << angle << std::endl;
	if(angle >= 0 && angle < 1024) {
		setReg2(id, 30, angle);
	}
  	//usleep(ACTION_DELAY*100);
  	usleep(100);
}


void close_grip() {
  setPosition(GRIP_ID, CLOSED_POS);
  usleep(100000);
}

void open_grip() {
	setPosition(GRIP_ID, OPEN_POS);
	usleep(100000);
}


void lift_up() {
  setPosition(LIFT_ID, TOP_POS);
  usleep(100000);  
}

void lift_middle() {
  setPosition(LIFT_ID, MIDDLE_POS);
  usleep(100000);  
}

void lift_down() {
  setPosition(LIFT_ID, BOTTOM_POS);
  usleep(100000);  
}


void print(std::string s) {
	std::cout << s << std::endl;
}

// ====== MOVED OVER ======

/*
int getPosition() {
	std::cout << "GetPosition: ";

	regRead(id, 36, 2); //(int id, int firstRegAdress, int noOfBytesToRead)
	usleep(10000);
	readByte();

	std::cout << "[";
	for(int i = 0; i < 7; i++) {
		std::cout << (int) return_byte[i] << ", ";
	}
	std::cout << (int) return_byte[7] << "]" << std::endl;


	int val = 0;
	if(return_byte[3] == 3) {
		val =  (int) return_byte[5];
	} else if(return_byte[3] == 4) {
	    val += return_byte[6] << 8;
    	val += return_byte[5];
	}

	std::cout << "Position is: " << val << std::endl;
	return val;
}
*/

void flush() {
	std::cout << "Flushing serial: [";
    while(serial.rdbuf()->in_avail()) {
        uint8_t b = serial.get();
        std::cout << (int) b;
      // (void) b;
    }
    std::cout << "]" << std::endl;
}


int main(int argc, char *argv[]) {
   // serial = new SerialStream;
  std::cout << "Opening serial" << std::endl;
  serial.Open("/dev/ttyUSB0");
  //serial.Open("/dev/ttyACM0");

  std::cout << "Configurating serial" << std::endl;
  serial.SetBaudRate( SerialStreamBuf::BAUD_38400 );
  serial.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 );
  serial.SetNumOfStopBits( 1 ); //SerialStreamBuf::DEFAULT_NO_OF_STOP_BITS );
  serial.SetParity( SerialStreamBuf::PARITY_NONE );
  serial.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_NONE ); //FLOW_CONTROL_HARD
  std::cout << "Configuration complete" << std::endl;

  //setSpeed(atoi(argv[1]), 145);

  if(argc > 2) {
    setPosition(atoi(argv[1]), atoi(argv[2]));
  }

}


// 90* = 1024/4 = 2561