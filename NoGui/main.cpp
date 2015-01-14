/*
 * File:   main.cpp
 * Author: Eivind
 *
 * Created on 26. september 2014, 14:15
 *
 * TODO: 
 *      - change com-protocol from (x, y, r) -> (x,y) OR (r)
 *      - com should perhaps return more verbose error-messages to clients
 */

#include "motorcom.h"
#include "poscontrol.h"
#include "printing.h"
#include "sound.h"
#include <string>
#include <cstring>
#include <thread>
#include <mutex>
#include <atomic>
#include <zmq.hpp>

bool checkArguments(int argc, char *argv[]);
void drive();
void testDrive();
int getArguments(std::string input, int *pos);
bool sendToPos(int num_args, int *args);
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

int ACCELERATION = 3;
bool sound_enabled = false;

/* Waits for input on socket, mainly position. 
 * TODO: define an extensive communication protocol
 */
void readLoop() {
	TIMESTAMP("[COM] starting");
	// Prepare context and socket
	zmq::context_t context (1);
	zmq::socket_t socket (context, ZMQ_REP);
	socket.bind ("tcp://*:5555");

	while (true) {
		zmq::message_t request;
		// Wait for next request from client
		socket.recv (&request);
		std::string recv_str = std::string(static_cast<char*>(request.data()), request.size());
		TIMESTAMP("[COM] received" << recv_str);

        // Request for position
        if(recv_str.compare("getpos") == 0) {
            //return position
            //check if position can be read.
            //tries 4 times in case of mutex
            bool got_position = false;
            std::string pos_str;
            for(int i = 0; i < 4; i++) {
                std::string pos_str = p->getCurrentPos();
                if(pos_str.length() > 2) {
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
                usleep(1000);
            }

            if(!got_position) {
                TIMESTAMP("[COM] reply=unavailable");
                zmq::message_t reply(11);
                memcpy ((void *) reply.data (), "unavailable", 11);
                usleep(100);
                socket.send (reply);
            }
        } 

        //Not request, should be either a position or rotation
        else {
    	   	//check arguments
    		int pos[3];
            int args = getArguments(recv_str, pos);
            zmq::message_t reply(2);

            if(args <= 1 || args > 3) {
                TIMESTAMP("[COM] invalid number arguments(" << args << "): " << recv_str);
                
                //recv_str is invalid, return negative to client
                TIMESTAMP("[COM] reply=no");
                memcpy ((void *) reply.data (), "no", 2);
            } 
            else {
                TIMESTAMP("[COM] got position: " << args);

                bool success = sendToPos(args, pos);
                if(success) {
                    //return OK to client
                    TIMESTAMP("[COM] reply=ok");
                    memcpy ((void *) reply.data (), "ok", 2);
                } 
                else {
                    //return negative to client
                    TIMESTAMP("[COM] reply=no");
                    memcpy ((void *) reply.data (), "no", 2);
                }
            } 
            //sleep(1);
            usleep(100);
            TIMESTAMP("[COM] sending reply: " << std::string(static_cast<char*>(reply.data()), reply.size()));
            socket.send (reply);
        }
	}
}	


//attempts twice to lock mutex, write values, and unlock mutex
bool sendToPos(int num_args, int *args) {
    if(read_mutex.try_lock()) {
        if(num_args == 2) {
            p->enqueue(args[0], 0, 0, args[1], ROTATION);
        } else if(num_args == 3) {
            p->enqueue(args[0], args[1], args[2], 0, POSITION);
        }
        read_mutex.unlock();
        return true;
    } 
    else {
        usleep(1000);
        if(read_mutex.try_lock()) {
            if(num_args == 2) {
               p->enqueue(args[0], 0, 0, args[1], ROTATION);
            } else if(num_args == 3) {
                p->enqueue(args[0], args[1], args[2], 0, POSITION);
            }
            read_mutex.unlock();
            return true;
        }
    }
    PRINTLINE("MAIN: try_lock read_mutex unsuccessful");
    return false;
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


//could be written simpler/more efficient
int getArguments(std::string input, int *pos) {
	int i = 0;
    std::istringstream f(input);
    std::string s;
    while(getline(f, s, ',')) {
        pos[i] = atoi(s.c_str());
        i++;
        if(i > 2) break; //hack!! probably incorrect
    }
    //i=1 invalid
    //i=2 rotation+id
    //i=3 position+id
    return i;
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
	PRINTLINE("[SETUP] Reading arguments:   ");
	m->serialSimDisable(); //just because
    if(argc < 2) {
    	PRINTLINE("    No arguments - expecting serial at: /dev/ttyUSB0");
    	m->setSerialPort("ttyUSB0");
    } else {
    	PRINT("Arguments: ");
    	for(int i = 0; i < argc; i++) {
    		PRINT(argv[i] << " ");
    	}
    	PRINTLINE("");

    	for(int i = 1; i < argc; i++) {
			if(strcmp(argv[i], "sim") == 0) {
				PRINTLINE("     Simulating serial.");	
				m->serialSimEnable();
			} else if(strcmp(argv[i], "sound") == 0) {
                PRINTLINE("     Sound enabled.");
                sound_enabled = true;
            }
			else if(strcmp(argv[i], "ttyACM0") == 0) {
				PRINTLINE("     Opening serial on: /dev/" << argv[i]);
				m->setSerialPort(argv[1]);
			}
			else if(strcmp(argv[i], "ttyS0") == 0) {
				PRINTLINE("     Opening serial on: /dev/" << argv[i]);
				m->setSerialPort(argv[1]);
			}
			else if(strcmp(argv[i], "ttyACM1") == 0) {
				PRINTLINE("     Opening serial on: /dev/" << argv[i]);
				m->setSerialPort(argv[1]);
			}
			else if(strcmp(argv[i], "ttyUSB1") == 0) {
				PRINTLINE("     Opening serial on: /dev/" << argv[i]);
				m->setSerialPort(argv[1]);
			} else {
				PRINTLINE("     Invalid argument: " << argv[i]);
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
    usleep(10000);
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

    int acc2 = m->getAcceleration();
    PRINTLINE("[SETUP] Acceleration is: " << acc2 << ", setting new acceleration" << ACCELERATION);
    usleep(1000);
    m->setAcceleration(ACCELERATION);
    usleep(1000);
    acc2 = m->getAcceleration();
    PRINTLINE("[SETUP] acceleration is now: " << acc2);
    usleep(2000);

    PRINTLINE("[SETUP] done, looping and checking for input");
    
/*
    //should be done in com-loop?!
    while(true) {
        if(new_pos_ready) {
            //attempt to lock mutex, read values, unlock mutex, set pos_ready to false
            if(read_mutex.try_lock()) {
//                p->setGoalPos(input_pos.x, input_pos.y, input_pos.rot);
                read_mutex.unlock();
                new_pos_ready = false; 
            } else {
                usleep(1000);
                if(read_mutex.try_lock()) {
  //                  p->setGoalPos(input_pos.x, input_pos.y, input_pos.rot);
                    read_mutex.unlock();
                    new_pos_ready = false; 
                } else {
                    PRINTLINE("MAIN: try_lock read_mutex unsuccessful");
                }
            }
        }
        usleep(500);
    }*/

    PRINTLINE("[SETUP] Exiting");
    if(read_thread.joinable()) {
        read_thread.join();
    }
    if(pos_thread.joinable()) {
        pos_thread.join();
    }

    return 0;
}