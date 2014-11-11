/*
 * File:   main.cpp
 * Author: Eivind
 *
 * Created on 26. september 2014, 14:15
 */
//#include "serial.h"

//TODO: own method for getdegrees/getpositions to the readloop
 //TODO: negative angle adjustments


#include "motorcom.h"
#include "communication.h"
#include "poscontrol.h"
#include "printing.h"
#include <string>
#include <cstring>
//#include <pthread.h>
#include <thread>
#include <mutex>
#include <atomic>

bool checkArguments(int argc, char *argv[]);
void drive();
void testDrive();
bool getArguments(std::string input, int *pos);


struct position {
	int x;
	int y;
	int rot;
};

struct position input_pos;

MotorCom *m;
PosControl *p;
std::mutex read_mutex;
std::atomic<bool> new_pos_ready(false);

void readLoop() {
	// Prepare our context and socket
	zmq::context_t context (1);
	zmq::socket_t socket (context, ZMQ_REP);
	socket.bind ("tcp://*:5555");
	while (true) {
		zmq::message_t request;
		// Wait for next request from client
		socket.recv (&request);
		std::string recv_str = std::string(static_cast<char*>(request.data()), request.size());

		PRINTLINE("READTHREAD: received" << recv_str);
		// Do some 'work'
		
		sleep(1);

		//check arguments
		int pos[3];
		bool isPosition = getArguments(recv_str, pos);
		
		zmq::message_t reply(2);
		if(isPosition) {
			//set position in struct

			//if(pthread_mutex_trylock(&read_pos_mutex) != 0) {
			if(false) {
				usleep(5000);
				if(true) {//mutex) {
					PRINTLINE("READLOOP: error, mutex still locked");
				} else {
					input_pos.x = pos[0];
					input_pos.y = pos[1];
					input_pos.rot = pos[2];
				}
			} else {
				input_pos.x = pos[0];
				input_pos.y = pos[1];
				input_pos.rot = pos[2];
			}

			//return OK to client
			memcpy ((void *) reply.data (), "ok", 2);
		} else {
			//recv_str is invalid, return negative to client
			memcpy ((void *) reply.data (), "no", 2);
		}
		socket.send (reply);
	}
	//return 0;
}	



int main(int argc, char *argv[]) {
    PRINTLINE("SETUP: creating MotorCom");
	m = new MotorCom;

    PRINTLINE("SETUP: starting serial");
    m->startSerial();

    PRINTLINE("SETUP: resetting encoders and flushing serial");
    m->resetEncoders();
    usleep(10000);
    m->flush();

    PRINTLINE("SETUP: checking cmdline-arguments");
    if(!checkArguments(argc, argv)) {
    	return -1;
    }

    PRINTLINE("SETUP: initializing PosControl");
    p = new PosControl(m);

    PRINTLINE("SETUP: initializing readLoop thread");
    input_pos.x = 0;
    input_pos.y = 0;
    input_pos.rot = 0;

    std::thread read_thread(readLoop);

    //PRINTLINE("SETUP: initializing writeLoop thread");
    //std::thread write_thread(writeLoop);


    PRINTLINE("SETUP: initializing controlLoop thread");
    std::thread pos_thread(&PosControl::controlLoop, p);


/*
    PRINTLINE("SETUP: done, looping and checking for input");
    while(true) {
    	if(new_pos_ready) {

    		//attempt to lock mutex, read values, unlock mutex, set pos_ready to false
    		if(read_mutex.try_lock()) {
    			//setGoalPos(input_pos.x, input_pos.y, input_pos.rot);
	    		read_mutex.unlock();
    			new_pos_ready = false; 
    		} else {
    			usleep(1000);
    			if(read_mutex.try_lock()) {
	    			//setGoalPos(input_pos.x, input_pos.y, input_pos.rot);
		    		read_mutex.unlock();
	    			new_pos_ready = false; 
    			} else {
    				PRINTLINE("MAIN: try_lock read_mutex unsuccessful");
    			}
    		}
    	}
    	usleep(1000);
    }*/

 //   testDrive();

    PRINTLINE("MAIN: Exiting");
    if(read_thread.joinable()) {
    	read_thread.join();
    }
/*    if(write_thread.joinable()) {
		write_thread.join();
    }*/
    if(pos_thread.joinable()) {
    	pos_thread.join();
    }

    return 0;
}


bool getArguments(std::string input, int *pos) {
	int i = 0;
    std::istringstream f(input);
    std::string s;    
    while(getline(f, s, ',')) {
        //std::cout << s << endl;
        pos[i] = atoi(s.c_str());
        
        i++;
        if(i >= 3) break;
    }

    if(i != 2) {
    	return false;
    } else {
        if(abs(pos[0]) > 300) return false;
        else if(abs(pos[1]) > 300) return false;
        else if(pos[2] < 0 || pos[2] > 360) return false;
    	else return true;
    }
}

/*
void drive() {
    bool done = false;
    while(!done) {
        done = p->controlLoop();
        usleep(4000);
    }
    PRINTLINE("Drive done");
    usleep(1000000);
}*/


void testDrive() {
    p->setGoalPos(50,0,0);
    //p->controlLoop();
    p->setGoalPos(50,0, 90);
    //p->controlLoop();
    p->setGoalPos(50,50, 90);
   // p->controlLoop();
    p->setGoalPos(50,50, 180);
   // p->controlLoop();
    p->setGoalPos(0,50, 180);
   // p->controlLoop();
    p->setGoalPos(0,50, 270);
   // p->controlLoop();
    p->setGoalPos(0,0, 270);
   // p->controlLoop();
    p->setGoalPos(0,0, 0);
   // p->controlLoop();	
}


/* return:
 *  	true - if good/no arguments
 *		false - if invalid argument exists
*/
bool checkArguments(int argc, char *argv[]) {
	PRINTLINE("READING Arguments");
	m->serialSimDisable(); //just because
    if(argc < 2) {
    	PRINTLINE("No arguments - expecting serial at: /dev/ttyACM0");
    	m->setSerialPort("ttyACM0");
    } else {
    	PRINT("Arguments: ");
    	for(int i = 0; i < argc; i++) {
    		PRINT(argv[i] << " ");
    	}
    	PRINTLINE("");


    	for(int i = 1; i < argc; i++) {
			if(strcmp(argv[i], "sim") == 0) {
				PRINTLINE("Simulating serial.");	
				m->serialSimEnable();
			}
			else if(strcmp(argv[i], "ttyUSB0") == 0) {
				PRINTLINE("Opening serial on: /dev/" << argv[i]);
				m->setSerialPort(argv[1]);
			}
			else if(strcmp(argv[i], "ttyS0") == 0) {
				PRINTLINE("Opening serial on: /dev/" << argv[i]);
				m->setSerialPort(argv[1]);
			}
			else if(strcmp(argv[i], "ttyACM1") == 0) {
				PRINTLINE("Opening serial on: /dev/" << argv[i]);
				m->setSerialPort(argv[1]);
			}
			else if(strcmp(argv[i], "ttyUSB1") == 0) {
				PRINTLINE("Opening serial on: /dev/" << argv[i]);
				m->setSerialPort(argv[1]);
			} else {
				PRINTLINE("Invalid argument: " << argv[i]);
				return false;
			}
		}
    }
    return true;
}