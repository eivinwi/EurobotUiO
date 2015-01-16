/*
 * File:   main.cpp
 * Author: Eivind
 *
 * Created on 26. september 2014, 14:15
 *
 * TODO: 
 *      - com should perhaps return more verbose error-messages to clients
 *      - noCom for program startup without read/write threads (probably not)
 *      - sound
 */

#include "motorcom.h"
#include "poscontrol.h"
#include "printing.h"
//include "sound.h"
#include "protocol.h"
#include <string>
#include <cstring>
#include <thread>
#include <mutex>
#include <zmq.hpp>
#include <iostream>


bool checkArguments(int argc, char *argv[]);
void drive();
void testDrive();
int getArguments(std::string input, int *pos);
bool enqRotation(int num_args, int *args);
bool enqPosition (int num_args, int *args);
void readLoop();
void testSystem();
void printResult(std::string text, bool success);

struct position {
	int x;
	int y;
	int rot;
};

struct position input_pos;

MotorCom *m;
PosControl *p;
std::mutex read_mutex;

int ACCELERATION = 3;
int MODE = 0;
bool sound_enabled = false;
bool com_running = false;

/* Waits for input on socket, mainly position. 
 */
void readLoop() {
	TIMESTAMP("[COM] starting");
    com_running = true;
	// Prepare context and socket
	zmq::context_t context (1);
	zmq::socket_t socket (context, ZMQ_REP);
	socket.bind ("tcp://*:5555");

	while (true) {
		zmq::message_t request;
		// Wait for next request from client
		socket.recv (&request);
		std::string recv_str = std::string(static_cast<char*>(request.data()), request.size());
		TIMESTAMP("[COM] recv_str: " << recv_str);

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
            std::string reply_str;
    	   	//check arguments
    		int args[4];
            int num_args = getArguments(recv_str, args);
            //zmq::message_t reply(20);

            if(num_args < 1 || num_args > 4) {
                TIMESTAMP("[COM] invalid number arguments(" << num_args << "): " << recv_str);
                
                //recv_str is invalid, return negative to client
                reply_str = "no";
            } 
            else {
                switch(args[0]) {
                    case REQUEST: 
                        //request for information
                        TIMESTAMP("[COM] Recieved REQUEST");
                        if(args[1] == 1) {
                            //return id
                            int id = p->getCurrentId();
                            reply_str = std::to_string(id);
                            TIMESTAMP("[COM] REQUEST was for ID, returning: " << reply_str);
                        } else if(args[1] == 2) {
                            //return pos
                            reply_str = p->getCurrentPos();
                            TIMESTAMP("[COM] REQUEST was for POS, returning(length=" << reply_str.length() << "): " << reply_str);
                        }
                        
                        break;
                    case RESET_ALL: 
                        //order to reset all
                        TIMESTAMP("[COM]  Received RESET_ALL");
                        p->reset();
                        reply_str = "ok";
                        break;
                    case SET_ROTATION: 
                        TIMESTAMP("[COM]  Received SET_ROTATION(id=" << args[1] << ", r=" << args[2] << ")");
                        enqRotation(num_args, args);
                        reply_str = "ok";
                        break;
                    case SET_POSITION: 
                        TIMESTAMP("[COM]  Received SET_POSITION(id=" << args[1] << ", x=" << args[2] << ", y=" << args[3] << ")");
                        enqPosition(num_args, args);
                        reply_str = "ok";
                        break;
                    case LIFT: 
                        TIMESTAMP("[COM]  Received LIFT");
                        //TODO
                        reply_str = "ok";
                        break;
                    case GRAB: 
                        TIMESTAMP("[COM]  Received GRAB");
                        //TODO
                        reply_str = "ok";
                        break;
                    case SHUTTER: 
                        TIMESTAMP("[COM]  Received SHUTTER");
                        //TODO
                        reply_str = "ok";
                        break;
                    default:
                        TIMESTAMP("[COM] invalid command: " << args[0]);
                        reply_str = "no";
                        break;
                }                
            } 
            //sleep(1);

            //create reply message
            zmq::message_t reply(reply_str.length());
            memcpy ((void *) reply.data (), reply_str.c_str(), reply_str.length());

            usleep(20);
            TIMESTAMP("[COM] sending reply: " << std::string(static_cast<char*>(reply.data()), reply.size()) << ".");
            socket.send (reply);
        }
	}
    com_running = false;
}	


//attempts twice to lock mutex, write values, and unlock mutex
//returns: true if successfully sent rotation to PosControl,
//         false if not
bool enqRotation(int num_args, int *args) {
    if(num_args != 3) {
        TIMESTAMP("[COM] Rotation: wrong number of arguments: " << num_args);
    }
    else if(args[0] != SET_ROTATION) {
        TIMESTAMP("[COM] Rotation: incorrect arguments: " << args[1]);  //should never happen      
    }
    else {
        if(read_mutex.try_lock()) {
            p->enqueue(args[1], 0, 0, args[2], ROTATION);
            read_mutex.unlock();
            return true;
        } 
        else {
            usleep(1000);
            if(read_mutex.try_lock()) {
                p->enqueue(args[1], 0, 0, args[2], ROTATION);
                read_mutex.unlock();
                return true;
            }
        }
        TIMESTAMP("[COM] try_lock read_mutex unsuccessful");
    }
    return false;
}

//see enqRotation
bool enqPosition(int num_args, int *args) {
    if(num_args != 4) {
        TIMESTAMP("[COM] Position: wrong number of arguments: " << num_args);
    }
    else if(args[0] != SET_POSITION) {
        TIMESTAMP("[COM] Position: incorrect arguments: " << args[1]);  //should never happen      
    }
    else {
        if(read_mutex.try_lock()) {
            p->enqueue(args[1], args[2], args[3], 0, POSITION);
            read_mutex.unlock();
            return true;
        } 
        else {
            usleep(1000);
            if(read_mutex.try_lock()) {
                p->enqueue(args[1], args[2], args[3], 0, POSITION);
                read_mutex.unlock();
                return true;
            }
        }
        TIMESTAMP("[COM] try_lock read_mutex unsuccessful");
    }
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
//i=1 invalid    
//i=2 rotation+id
//i=3 position+id
int getArguments(std::string input, int *pos) {
    int i = 0;
    std::istringstream f(input);
    std::string s;
    while(getline(f, s, ',')) {
        pos[i] = atoi(s.c_str());
        i++;
        if(i > 4) break; //hack!! probably incorrect
    }
    return i;
}

bool testing = false;


/* Checks cmd-line arguments 
 * return:
 *      true - if good/no arguments
 *      false - if invalid argument exists
 *
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
            } 
            else if(strcmp(argv[i], "testing") == 0) {
                PRINTLINE("     Testing enabled.");
                testing = true;
            }
            else if(strcmp(argv[i], "sound") == 0) {
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

    PRINTLINE("[SETUP] starting and flushing serial");
    m->startSerial();
    usleep(100000);
    m->flush();

    PRINTLINE("[SETUP] resetting encoders");
    m->resetEncoders();
    usleep(5000);


    PRINTLINE("[SETUP] initializing PosControl");
    p = new PosControl(m, testing);



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

    //m->testSerial();

    int acc2 = m->getAcceleration(); 
    if(acc2 != ACCELERATION) {
        PRINTLINE("[SETUP] Acceleration is: " << acc2 << ", setting new acceleration: " << ACCELERATION);
        usleep(1000);
        m->setAcceleration(ACCELERATION);
    }
    
    usleep(1000);
    int mode = m->getAcceleration();
    if(mode != 0) {
        PRINTLINE("[SETUP] Mode is: " << m << ", setting new mode: " << MODE);
        usleep(1000);
        m->setMode(MODE);
    }

    testSystem();

    PRINTLINE("");
    TIMESTAMP("[SETUP] Testing completed, waiting for client input");
    if(read_thread.joinable()) {
        read_thread.join();
    }
    if(pos_thread.joinable()) {
        pos_thread.join();
    }
    return 0;
}

void testSystem() {
    usleep(20000);
    m->flush();
    PRINTLINE("[SETUP] Complete, testing components:\n");

    if(m->test()) {
        printResult("[TEST] MotorCom active", true);
        if(m->isSimulating()) printResult("[TEST] Serial open", true);
        else           printResult("[TEST] Serial open (sim)", true);
    } 
    printResult("[TEST] PosControl active", p->test()); //poscontrol test
    printResult("[TEST] Read_thread running", com_running);
    printResult("[TEST] Pos_thread running", p->running());

    uint8_t voltage = m->getVoltage();
    printResult("[TEST] Voltage = " + std::to_string((int)voltage) +"v", (voltage > 20 && voltage < 25));
    uint8_t error = m->getError();
    printResult("[TEST] MD49_Error = " + std::to_string((int) error), (error == 0));
    int acc = m->getAcceleration();
    printResult("[TEST] Acceleration = " + std::to_string((int) acc), (acc == ACCELERATION));
    

    int mode = m->getMode();
    printResult("[TEST] Mode = " + std::to_string((int) mode), (mode == MODE));
}

void printResult(std::string text, bool success) {
    if(success) {
        std::cout << std::left << std::setw(30) << text
             << std::right << std::setw(30) << "\033[0;32m[ok]\033[0m" << std::endl;
    } else {
        std::cout << std::left << std::setw(30) << text
             << std::right << std::setw(30) << "\033[0;31m[fail]\033[0m"<< std::endl;
    }
}

//#define PRINT_OK() std::cout << std::right << std::setw(40) << "\033[0;32m[ok]\033[0m" << std::endl;


//#define PRINT_FAIL() std::cout << std::right << std::setw(40) << "\033[0;31m[fail]\033[0m" << std::endl; 
