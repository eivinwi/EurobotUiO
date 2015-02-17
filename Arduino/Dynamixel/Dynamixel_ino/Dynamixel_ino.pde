// Simple AX12/18 examples - INF4500, M.H. 2014
// No guaranty for best / simplest code, use it if you want
// To use this sketch - paste all of this code in to an empty Processing editor window and save

import processing.serial.*;
import java.awt.event.KeyEvent;

Serial myPort;
int speed = 200;

int id = 1;

void setup() 
{
  // The "main program" in this example
  // window size - window not used
  size(640, 360);

  // 1Mbit transfer rate
  println(Serial.list());
  //myPort = new Serial(this, "/dev/ttyACM0", 1000000);
  myPort = new Serial(this, "/dev/ttyUSB0", 38400); // 115200);
  //myPort = new Serial(this, "/dev/ttyUSB0", 117647);


  // Some examples, just to see how it can be done

  // Configure a new ID to a servo on the bus:
  // (only one servo can be on the bus during this broadcast instruction)
  //setReg1(254, 3, id);

  // LED on/off: Set servo LED on/off: 
  toggleLed(id);

  //turn off feeback on commands
  //setReg1(id, 16, 0);

  // Simple move: Move one servo, angle(0-1023), speed(0-1023): 
  //setSpeed(id, speed); //
  //setSpeed(id, 10);




  // Ping: 
  // sendCmd(id, 1);

  // Reset: resets all registers to factory configuration, do so it if messed up:
  // For some unknown reason it may not reset everything to default values...
  // sendCmd(id, 2);  

  // Set max/min angle: limit the angular range (0-1024). 
  // Also use this to select "wheel mode" / "joint mode", maxAngle = minAngle = 0 gives "wheel mode"
  
//   setReg2(id, 6, 0x3FF); 
//   setReg2(id, 8, 0x3FF);

  // Read temp: read internal servo temperature in Celsius 
  // regRead(id, 43, 1);
  
  //readAngle();
  
  
  //ENGAGE
//  toggleLeds();
  //setFeedbackLevel(id, 1);
//  setSpeed(id, speed);
  
  while(myPort.available() > 0) {
    myPort.read(); 
  }
 // readAngle(0);
 continue_loop = true;
 
  goToAngle(id, 0);
  delay(1000);
  goToAngle(id, 100);
  delay(1000);
  goToAngle(id, 200);
  delay(1000);
  goToAngle(id, 300);
  delay(1000);
  goToAngle(id, 400);
  delay(1000);
  goToAngle(id, 500);
  
//  setSpeed(id, 0);

 // setReg2(id, 4, 34);
}




void toggleLed(int id) {
    setReg1(id, 25, 1);
}

//input 0, 1 or 2
void setFeedbackLevel(int id, int level) {
  setReg1(id, 16, 0x1);
  //setReg(id, 16, level);
}

void setSpeed(int id, int speed) {
  if(speed > 0 && speed <= 1023) {
     setReg2(id, 32, speed);
  }
}

//angle must be between 0-1023
void goToAngle(int id, int angle) {
  if(angle > 0 && angle < 1023) { 
  setReg2(id, 30, angle);
//  setReg2(id, 30, angle);
  }
}


boolean continue_loop;


void draw() 
{
  // infinite loop, not used in this example
}

void keyPressed() 
{
  /*
  print("{");
  for(int i = 0; i < 5; i++) {
    print(current_pos[i]+", ");
  }
  println(current_pos[5] + ", std_delay},");

  if(key == 's') {
    continue_loop = false;
    startPosition(); 
  } 
  if(key == 'c') {
    continue_loop = false;
    crouchPosition(); 
  }
  if(key == 'd') {
    continue_loop = false;
    downPosition(); 
  }
  if(key == 'b') {
    continue_loop = true;
    walkLoop(walk_backward);   
  }
  if(key == 'f') {
    continue_loop = true;
    walkForward(-1);
  }
  if(key == 'l') {
    continue_loop = false;
    walkOnce(flip_stages); 
  }*/
  
  /*
  if (key == '0') {
    if(current_pos[0] < 0x3f9) current_pos[0]+=5;
    setPos(0, current_pos[0]);
    println("id" + 0 + ": " + current_pos[0]);
  }
  if (key == '1') {
    if(current_pos[1] < 0x3f9) current_pos[1]+=5;
    setPos(1, current_pos[1]);
    println("id" + 1 + ": " + current_pos[1]);
  }
  if (key == '2') {
    if(current_pos[2] < 0x3f9) current_pos[2]+=5;
    setPos(2, current_pos[2]);
    println("id" + 2 + ": " + current_pos[2]);
  }
  if (key == '3') {
    if(current_pos[3] < 0x3f9) current_pos[3]+=5;
    setPos(3, current_pos[3]);
    println("id" + 3 + ": " + current_pos[3]);
  }
  if (key == '4') {
    if(current_pos[4] < 0x3f9) current_pos[4]+=5;
    setPos(4, current_pos[4]);
    println("id" + 4 + ": " + current_pos[4]);
  }
  if (key == '5') {
    if(current_pos[5] < 0x3f9) current_pos[5]+=5;
    setPos(5, current_pos[5]);
    println("id" + 5 + ": " + current_pos[5]);
  }


  if (key == 'p') {
    if(current_pos[0] > 4) current_pos[0]-=5;
    setPos(0, current_pos[0]);
    println("id" + 0 + ": " + current_pos[0]);
  }
  if (key == 'q') {
    if(current_pos[1] > 4) current_pos[1]-=5;
    setPos(1, current_pos[1]);
    println("id" + 1 + ": " + current_pos[1]);
  }
  if (key == 'w') {
    if(current_pos[2] > 4) current_pos[2]-=5;
    setPos(2, current_pos[2]);
    println("id" + 2 + ": " + current_pos[2]);
  }
  if (key == 'e') {
    if(current_pos[3] > 4) current_pos[3]-=5;
    setPos(3, current_pos[3]);
    println("id" + 3 + ": " + current_pos[3]);
  }
  if (key == 'r') {
    if(current_pos[4] > 4) current_pos[4]-=5;
    setPos(4, current_pos[4]);
    println("id" + 4 + ": " + current_pos[4]);
  }
  if (key == 't') {
    if(current_pos[5] > 4) current_pos[5]-=5;
    setPos(5, current_pos[5]);
    println("id" + 5 + ": " + current_pos[5]);
  }
  
  if (key == CODED) {
    if(keyCode == KeyEvent.VK_F1) {
      continue_loop = false;
      walkForward(0);
    } 
    if(keyCode == KeyEvent.VK_F2) {
        continue_loop = false;
        walkForward(1);
    } 
    if(keyCode == KeyEvent.VK_F3) {
      continue_loop = false;  
      walkForward(2);
    } 
    if(keyCode == KeyEvent.VK_F4) {
      continue_loop = false;
      walkForward(3);
    } 
    if(keyCode == KeyEvent.VK_F5) {
      continue_loop = false;
      walkForward(4);
    } 
    if(keyCode == KeyEvent.VK_F6) {
      continue_loop = false;  
      walkForward(5);
    } 
    if(keyCode == KeyEvent.VK_F7) {
      continue_loop = false;  
      walkForward(6);
    } 
    if(keyCode == KeyEvent.VK_F8) {
      continue_loop = false;        
      walkForward(7);
    } 
    if(keyCode == KeyEvent.VK_F9) {
      continue_loop = false;
      walkForward(8);
    } 
    if(keyCode == KeyEvent.VK_F10) {
      continue_loop = false;
      walkForward(9);
    } 
    if(keyCode == KeyEvent.VK_F11) {
      continue_loop = false;  
      walkForward(10);
    } 
    if(keyCode == KeyEvent.VK_F12) {
      continue_loop = false;
      walkForward(11);
    } 
  }
*/
}

/*
void serialEvent(Serial p) 
{ 
  // Handles bytes back from servo (status packet) if you like it event based
  // Otherwise comment out this event based method and use "myPort.read()" from your other methods

    int in = p.read(); 
    print(" " + in); // DEC
    print(" " + hex(in, 2)); // HEX
  
} */

int readByte(Serial p) {
  while(p.available() == 0);
  
  println("DONE");
  return p.read(); 
}

int readAngle(int servoId) {
  regRead(servoId, 36, 2);
  println("Reading reg36 from servo" + servoId);
  int len = 0;
  for(int i = 0; i < 4; i++) {
     len = readByte(myPort);
     println("length: " + len + " " + hex(len, 2));    
  }
  int error = readByte(myPort);
  println("error : " + error + " " + hex(error, 2));

  if(len == 2) {
    int low = readByte(myPort);
    println("low   :" + low + " " + hex(low, 2));
    int high = readByte(myPort);
    println("High  : " + high + " " + hex(high, 2));
    int checkSum = readByte(myPort);
    println("check : " + checkSum + " " + hex(checkSum, 2));
  
    int result = low | (high << 8);
    
    println("Angle of servo" + servoId + " is: " + result);
    return result;
  }
  println("length: " + len);
  return -1;
}


// *************************************************************************************************
// *************************************************************************************************
// ******* Dynamixel bareBone methods **************************************************************

// ======================= Writes 0<val<255 to register "regNo" in servo "id" ======================

void setReg1(int id, int regNo, int val)
{
  byte b[] = { 
    (byte)0xFF, (byte)0xFF, (byte)0, (byte)0, (byte)3, (byte)0, (byte)0, (byte)0
  }; 

  b[2] = (byte)id;
  b[5] = (byte)regNo;
  b[6] = (byte)val;

  b = addChecksumAndLength(b); 
  //printOutPacketToConsole(b); (not included) 
  myPort.write(b);
}

// ======================= Writes 0<val<1023 to register "regNoLSB/regNoLSB+1" in servo "id" =======

void setReg2(int id, int regNoLSB, int val)
{
  byte b[] = { 
    (byte)0xFF, (byte)0xFF, (byte)0, (byte)0, (byte)3, (byte)0, (byte)0, (byte)0, (byte)0
  }; 

  b[2] = (byte)id;
  b[5] = (byte)regNoLSB;
  b[6] = (byte)( val & 255 );
  b[7] = (byte)( (val >> 8) & 255 );

  b = addChecksumAndLength(b); 
  //printOutPacketToConsole(b); (not included) 
  
  
  print("Byte sent: {");
  print(b[0]);
  print(", ");
  print(b[1]);
  print(", ");
  print(b[2]);
  print(", ");
  print((int) b[3]);
  print(", ");
  print(b[4]);
  print(", ");
  print(b[5]);
  print(", ");
  print(b[6]);
  print(", ");
  print(b[7]);
  print(", ");
  print(b[8]);
  println("]");
  myPort.write(b);
}
//   setReg2(id, 30, angle);


// ======================= read from register, status packet printout is handled by serialEvent() ==

void regRead(int id, int firstRegAdress, int noOfBytesToRead)
{
  println(" "); // console newline before serialEvent() printout

  byte b[] = { 
    (byte)0xFF, (byte)0xFF, (byte)0, (byte)0, (byte)2, (byte)0X2B, (byte)0X01, (byte)0
  }; 

  b[2] = (byte)id;
  b[5] = (byte)firstRegAdress;
  b[6] = (byte)noOfBytesToRead;

  b = addChecksumAndLength(b); 
  //printOutPacketToConsole(b); (not included) 
  myPort.write(b);
}

// ======================= Sends a 1 byte command to servo id ======================================

void sendCmd(int id, int cmd)
{
  byte b[] = { 
    (byte)0xFF, (byte)0xFF, (byte)0, (byte)0, (byte)0, (byte)0
  }; 

  b[2] = (byte)id;
  b[4] = (byte)cmd;

  b = addChecksumAndLength(b); 
  //printOutPacketToConsole(b); (not included) 
  myPort.write(b);
}

// ======================= adds checksum and length bytes to the ASCII byte packet =================

byte[] addChecksumAndLength(byte[] b)
{
  // adding length
  b[3] = (byte)(b.length - 4);

  // finding sum
  int teller = 0;
  for (int i=2; i<(b.length-1); i++)
  {
    int tmp = (int)b[i];
    if (tmp < 0) 
      tmp = tmp + 256;
    teller = teller + tmp;
  }

  // inverting bits
  teller = ~teller;
  // int2byte 
  teller = teller & 255;

  // adding checkSum
  b[b.length-1] = (byte)teller;

  return b;
}

// ======================= just plain Java - pauses current thread =================================

void pause(int ms)
{
  try 
  {
    Thread.currentThread().sleep(ms);
  }
  catch(Exception ie) 
  {
    // whatever you like to complain about
  }
}


/*void readAngle() {
  //regRead(id, 36, 1);
  while(myPort.available() <= 0) {
    println("SA: " + myPort.available());
    delay(10);
  }
 //lese inn
  for(int i = 0; i < 5; i++) {
    println(i + ": "+ myPort.read());
  }
  int param = myPort.read();
  println("PARAM: " + param);
  println("check: " + myPort.read());
  regRead(id, 37, 1);
}*/


