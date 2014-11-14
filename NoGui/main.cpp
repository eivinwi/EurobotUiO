/*
 * File:   main.cpp
 * Author: Eivind
 *
 * Created on 26. september 2014, 14:15
 *
*
 */

#include "motorcom.h"
#include "poscontrol.h"
#include "printing.h"
#include <string>
#include <cstring>
#include <thread>
#include <mutex>
#include <atomic>
#include <zmq.hpp>

bool checkArguments(int argc, char *argv[]);
void drive();
void testDrive();
bool getArguments(std::string input, int *pos);
void readLoop();

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


/* Waits for input on socket, mainly position. 
 * TODO: define an extensive communication protocol
 */
void readLoop() {
	PRINTLINE("READ_THREAD: starting");
	// Prepare context and socket
	zmq::context_t context (1);
//    zmq::context_t *context = (zmq::context_t *) arg;

	zmq::socket_t socket (context, ZMQ_REP);
	socket.bind ("tcp://*:5555");

	while (true) {
		zmq::message_t request;
		// Wait for next request from client
		socket.recv (&request);
		std::string recv_str = std::string(static_cast<char*>(request.data()), request.size());
		TIMESTAMP("[COM] received" << recv_str);

        if(recv_str.compare("getpos") == 0) {
            //return position
            //check if position can be read.
            //tries 4 times in case of mutex
            bool got_position = false;
            std::string pos_str;
            for(int i = 0; i < 4; i++) {
                std::string pos_str = p->getCurrentPos();
                if(pos_str.length() > 0) {
                    got_position = true;
                    TIMESTAMP("[COM] Got position: " << pos_str);
                    zmq::message_t reply(pos_str.length());
                    memcpy ((void *) reply.data (), pos_str.c_str(), pos_str.length());
                    std::string sending = std::string(static_cast<char*>(reply.data()), reply.size());
                    TIMESTAMP("[COM] Returning position (" << pos_str.length() << "): " << sending);
                    socket.send(reply);
                    TIMESTAMP("[COM] Done sending");
                    break;
                }
                usleep(100);
            }

            if(!got_position) {
                zmq::message_t reply(11);
                memcpy ((void *) reply.data (), "unavailable", 11);
                usleep(100);
                socket.send (reply);
            }
        } else {
    		//check arguments
    		int pos[3];
    		bool isPosition = getArguments(recv_str, pos);
    		zmq::message_t reply(2);
    		if(isPosition) {
                TIMESTAMP("[COM] got position");
    			//set position in struct
    			//if(pthread_mutex_trylock(&read_pos_mutex) != 0) {
    			if(!read_mutex.try_lock()) {
    				TIMESTAMP("[COM] mutex locked, wait and retry");
    				usleep(1000);
    				if(!read_mutex.try_lock()) {//mutex) {
    					TIMESTAMP("[COM] error, mutex still locked");
    				} else {
    					//PRINTLINE("READLOOP: mutex now open, setting position");
                        input_pos.x = pos[0];
                        input_pos.y = pos[1];
                        input_pos.rot = pos[2];
                        TIMESTAMP("[COM] mutex now open, setting position: [" << input_pos.x << "," << input_pos.y << "," << input_pos.rot << "]");
    					new_pos_ready = true;
    					read_mutex.unlock();
    				}
    			} else {
    				input_pos.x = pos[0];
    				input_pos.y = pos[1];
    				input_pos.rot = pos[2];
    				TIMESTAMP("[COM] setting position: [" << input_pos.x << "," << input_pos.y << "," << input_pos.rot << "]");
    				new_pos_ready = true;
    				read_mutex.unlock();
    			}
    			//return OK to client
    			memcpy ((void *) reply.data (), "ok", 2);
    		} 
            else {
    			//recv_str is invalid, return negative to client
    			memcpy ((void *) reply.data (), "no", 2);
    		}
            //sleep(1);
            usleep(100);
            socket.send (reply);
        }
	}
}	

/*
void writeLoop() {
    PRINTLINE("WRITE_THREAD: starting");
    // Prepare our context and socket
    bool got_position = false;
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);  */
   // socket.bind ("tcp://*:5556");
 /*   while (true) {
        zmq::message_t request;
        // Wait for next request from client
//        PRINTLINE
        socket.recv (&request);
        std::string recv_str = std::string(static_cast<char*>(request.data()), request.size());
        PRINTLINE("WRITETHREAD: recieved: " << recv_str);

        //check if position can be read.
        //tries 4 times in case of mutex
        got_position = false;
        std::string pos_str;
        for(int i = 0; i < 4; i++) {
            std::string pos_str = p->getCurrentPos();
            if(pos_str.length() > 0) {
                got_position = true;
                break;
            }
            usleep(1000);
        }

        sleep(1);
        if(got_position) {
            zmq::message_t reply(pos_str.length());
            memcpy ((void *) reply.data (), pos_str.c_str(), pos_str.length());
            socket.send(reply);
        } else {
            zmq::message_t reply(11);
            memcpy ((void *) reply.data (), "unavailable", 11);
            socket.send (reply);
        }
    }
}*/



bool getArguments(std::string input, int *pos) {
	int i = 0;
    std::istringstream f(input);
    std::string s;
    while(getline(f, s, ',')) {
        pos[i] = atoi(s.c_str());
        i++;
        if(i > 2) break;
    }
    if(i != 3) {
    	return false;
    } else {
        if(abs(pos[0]) > 300) return false;
        else if(abs(pos[1]) > 300) return false;
        else if(pos[2] < 0 || pos[2] > 360) return false;
    	else {
    		return true;
    	}
    }
}


//Warning: not updated to the new poscontrol-setup (loops continuosly)
void testDrive() {
	/*
    p->setGoalPos(50,0,0);
    p->setGoalPos(50,0, 90);
    p->setGoalPos(50,50, 90);
    p->setGoalPos(50,50, 180);
    p->setGoalPos(0,50, 180);
    p->setGoalPos(0,50, 270);
    p->setGoalPos(0,0, 270);
    p->setGoalPos(0,0, 0);
    */
}


/* Checks cmd-line arguments 
 * return:
 *  	true - if good/no arguments
 *		false - if invalid argument exists
 *
 * TODO: -noCom for program startup without read/write threads
 * TODO: -sound
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


int main(int argc, char *argv[]) {
    PRINTLINE("[SETUP] creating MotorCom");
    m = new MotorCom;

    PRINTLINE("[SETUP] checking cmdline-arguments");
    if(!checkArguments(argc, argv)) {
        return -1;
    }

    PRINTLINE("[SETUP] starting serial");
    m->startSerial();

    PRINTLINE("[SETUP] resetting encoders and flushing serial");
    m->resetEncoders();
    usleep(5000);
    m->flush();


    PRINTLINE("[SETUP] initializing PosControl");
    p = new PosControl(m);

    PRINTLINE("[SETUP] initializing readLoop thread");
    input_pos.x = 0;
    input_pos.y = 0;
    input_pos.rot = 0;


    std::thread read_thread(readLoop);
    usleep(5000);

 //   PRINTLINE("[SETUP] initializing writeLoop thread");
 //  std::thread write_thread(writeLoop);


    PRINTLINE("[SETUP] initializing controlLoop thread");
    std::thread pos_thread(&PosControl::controlLoop, p);
    usleep(5000);


    PRINTLINE("[SETUP] done, looping and checking for input");
    while(true) {
        if(new_pos_ready) {

            //attempt to lock mutex, read values, unlock mutex, set pos_ready to false
            if(read_mutex.try_lock()) {
                p->setGoalPos(input_pos.x, input_pos.y, input_pos.rot);
                //PRINTLINE();
                read_mutex.unlock();
                new_pos_ready = false; 
            } else {
                usleep(1000);
                if(read_mutex.try_lock()) {
                    p->setGoalPos(input_pos.x, input_pos.y, input_pos.rot);
                    read_mutex.unlock();
                    new_pos_ready = false; 
                } else {
                    PRINTLINE("MAIN: try_lock read_mutex unsuccessful");
                }
            }
        }
        usleep(500);
    }

 //   testDrive();

    PRINTLINE("[SETUP] Exiting");
    if(read_thread.joinable()) {
        read_thread.join();
    }
    if(pos_thread.joinable()) {
        pos_thread.join();
    }

    return 0;
}