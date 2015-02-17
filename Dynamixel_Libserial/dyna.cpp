#include <iostream>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <SerialStream.h>
using namespace LibSerial ;

// baud should be set to  117len47.1, which is 17 = 0x12
	

SerialStream serial;

void writeToSerial(int8_t b[], int len) {
	for(int i = 0; i < len; i++) {
		serial << b[i];
	}
	//serial << std::endl;
}


// adds checksum and length bytes to the ASCII byte packet
// input needs to be of length 4, with the last 2 places empty
void addChecksumAndLength(int8_t b[], int len) {
  // adding length
  b[3] = (int8_t) (len - 4);

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
  b[len-1] = (int8_t) counter;
//  return b;
}


void sendCmd(int id, int cmd) {
	int8_t b[] = {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

	b[2] = (int8_t) id;
	b[4] = (int8_t) cmd;

	addChecksumAndLength(b, 6);

//	serial.write(b);
	writeToSerial(b, 6);
}


// Writes 0<val<255 to register "regNo" in servo "id" 
void setReg1(int id, int regNo, int val)
{
  int8_t b[] = {0xFF, 0xFF, 0, 0, 3, 0, 0, 0}; 

  b[2] = (int8_t) id;
  b[5] = (int8_t) regNo;
  b[6] = (int8_t) val;

  addChecksumAndLength(b, 8); 
  writeToSerial(b, 8);
}


// Writes 0<val<1023 to register "regNoLSB/regNoLSB+1" in servo "id"
void setReg2(int id, int regNoLSB, int val)
{
  int8_t b[] = {0xFF, 0xFF, 0, 0, 3, 0, 0, 0, 0}; 
  b[2] = (int8_t) id;
  b[5] = (int8_t) regNoLSB;
  b[6] = (int8_t) ( val & 255 );
  b[7] = (int8_t) ( (val >> 8) & 255 );

  addChecksumAndLength(b, 9); 


std::cout << "Byte sent: {"
	<< (int) b[0] << ", " << (int) b[1] << ", " << (int) b[2]  << ", " << (int) b[3] << ", " 	
	<< (int) b[4] << ", " << (int) b[5] << ", " << (int) b[6] << ", " << (int) b[7] << "," << (int) b[8] << "]" << std::endl;
 writeToSerial(b, 9);
}


// Read from register, status packet printout is handled by serialEvent()
void regRead(int id, int firstRegAdress, int noOfBytesToRead)
{
 // println(" "); // console newline before serialEvent() printout

  int8_t b[] = {0xFF, 0xFF, 0, 0, 2, 0X2B, 0X01, 0}; 

  b[2] = (int8_t) id;
  b[5] = (int8_t) firstRegAdress;
  b[6] = (int8_t) noOfBytesToRead;

  addChecksumAndLength(b, 8); 
  writeToSerial(b, 8);
}


void goToAngle(int id, int angle) {
	if(angle >= 0 && angle < 1024) {
		setReg2(id, 30, angle);
	}
}

void setSpeed(int id, int speed) {
	if(speed >= 0 && speed < 1024) {
		setReg2(id, 32, speed);
	}
}

int led = 0;
void toggleLed(int id) {
	if(led == 1) led = 0;
	else led = 1;

	setReg1(id, 25, led);
}

void broadCastId(int id) {
	setReg1(254, 3, id);
}


int id = 1;

int main(int argc, char *argv[]) {
   // serial = new SerialStream;
  std::cout << "Opening serial" << std::endl;
  serial.Open("/dev/ttyUSB0");


  std::cout << "Configurating serial" << std::endl;
  serial.SetBaudRate( SerialStreamBuf::BAUD_38400 );
  serial.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 );
  serial.SetNumOfStopBits( 1 ); //SerialStreamBuf::DEFAULT_NO_OF_STOP_BITS );
  serial.SetParity( SerialStreamBuf::PARITY_NONE );
  serial.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_NONE ); //FLOW_CONTROL_HARD
  std::cout << "Configuration compelete" << std::endl;

  usleep(100000);

//  std::cout << "Broadcasting ID" << std::endl;
//  broadCastId(id);

  std::cout << "Toggling LED" << std::endl;
  toggleLed(id);

  std::cout << "Testing goToAngle:" << std::endl;
  std::cout << "0" << std::endl;
  goToAngle(id, 0);
  usleep(1000000);
  std::cout << "100" << std::endl;
  goToAngle(id, 100);
  usleep(1000000);
  std::cout << "200" << std::endl;
  goToAngle(id, 200);
  usleep(100000);
  std::cout << "300" << std::endl;
  goToAngle(id, 300);
  usleep(1000000);
  std::cout << "400" << std::endl;
  goToAngle(id, 400);
  usleep(1000000);
  std::cout << "500" << std::endl;
  goToAngle(id, 500);
  return 0;
}