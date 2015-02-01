/*
 *  File: main.cpp
 *  Author: Eivind Wikheim
 *
 *  Main implements a ZMQ server for communication with AI, reads commandline-arguments, 
 *  creates necessary class objects, starts threads and performs testing.
 *
 *  Copyright (c) 2015 Eivind Wikheim <eivinwi@ifi.uio.no>. All Rights Reserved.
 *
 *  This file is part of EurobotUiO.
 *
 *  EurobotUiO is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EurobotUiO is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EurobotUiO.  If not, see <http://www.gnu.org/licenses/>.
 */


 // TODO: 
 //      - com should perhaps return more verbose error-messages to clients
 //     - noCom for program startup without read/write threads (probably not)
 //     - sound
 
#include "motorcom.h"
#include "liftcom.h"
#include "poscontrol.h"
#include "printing.h"
//include "sound.h"
#include "protocol.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <thread>
#include <mutex>
#include <zmq.hpp>
#include <iostream>
#include <iomanip>
INITIALIZE_EASYLOGGINGPP

#ifndef ELPP_THREAD_SAFE
#define ELPP_THREAD_SAFE
#endif

// Implements a ZMQ server that waits for input from clients. 
// Used for communication with AI.
void readLoop();

// Reads command-line arguments. Uses GNU C++ GetOpt standard.
// Returns true if no invalid arguments
bool checkArguments(int argc, char *argv[]);

// Used by the readLoop ZMQ-server thread.
// Splits input from Client on delimiter, fills pos 2d-array with arguments.
// Returns number of arguments
int getArguments(std::string input, int *pos);

// Sends rotation-change to PosControl to be added to command-queue
bool enqRotation(int num_args, int *args);

// Sends position-change to PosControl to be added to command-queue
bool enqPosition (int num_args, int *args);

// Sends action to PosControl to be added to command-queue
bool enqAction (int num_args, int *args);

// Test each part of the system after setup is completed, logs and print results.
void testSystem();

// Custom print to show ok/fail results from testSystem
void printResult(std::string text, bool success);

// Implemented trough easylogging++. 
// Catches SIGINT to avoid annoying CTRL-C being handled as a program crash.
// Prints additional crash information if legitimate program crash.
void crashHandler(int sig);


// Objects 
MotorCom *m;
LiftCom *l;
PosControl *p;

// locking objects while readLoop is writing to them.
std::mutex read_mutex;

// MD49 constants
const int ACCELERATION = 10;
const int MODE = 0;

// Default values, to be changed with command-line arguments
bool sim_enabled = false;
bool sound_enabled = false;
bool com_running = false;
bool debug_file_enabled = false;
bool testing_enabled = false;
bool log_to_file = true;
std::string motor_serial = "ttyUSB0";
std::string lift_serial = "ttyUSB1";


/* Waits for input on socket, mainly position. */
void readLoop() {
    LOG(INFO) << "[COM] starting";
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
//        LOG(INFO) << "[COM] recv_str: " << recv_str;


        std::string reply_str;
        //check arguments
        int args[4];
        int num_args = getArguments(recv_str, args);
        //zmq::message_t reply(20);

        if(num_args < 1 || num_args > 4) {
            LOG(WARNING) << "[COM] invalid number arguments(" << num_args << "): " << recv_str;
            reply_str = "no";
        } 
        else {
            switch(args[0]) {
                case REQUEST: 
                    LOG(INFO) << "[COM] Recieved REQUEST";
                    if(args[1] == 1) {
                        int id = p->getCurrentId();
                        reply_str = std::to_string(id);
                        LOG(INFO) << "[COM] REQUEST was for ID, returning: " << reply_str;
                    } else if(args[1] == 2) {
                        reply_str = p->getCurrentPos();
                        LOG(INFO) << "[COM] REQUEST was for POS, returning(length=" << reply_str.length() << "): " << reply_str;
                    } else if(args[1] == 4) {
                        //LIFT
                        reply_str = std::to_string(p->getLiftPos());
                        LOG(INFO) << "[COM] REQUEST was for LIFT, returning: " << reply_str;
                    }                    
                    break;
                case RESET_ALL: 
                    LOG(INFO) << "[COM]  Received RESET_ALL";
                    p->reset();
                    reply_str = "ok";
                    break;
                case SET_ROTATION: 
                    LOG(INFO) << "[COM]  Received SET_ROTATION(id=" << args[1] << ", r=" << args[2] << ")";
                    enqRotation(num_args, args);
                    reply_str = "ok";
                    break;
                case SET_POSITION: 
                    LOG(INFO) << "[COM]  Received SET_POSITION(id=" << args[1] << ", x=" << args[2] << ", y=" << args[3] << ")";
                    enqPosition(num_args, args);
                    reply_str = "ok";
                    break;
                case LIFT: 
                    LOG(INFO) << "[COM]  Received LIFT";
                    enqAction(num_args, args);
                    reply_str = "ok";
                    break;
                case GRAB: 
                    LOG(INFO) << "[COM]  Received GRAB";
                    //TODO
                    reply_str = "ok";
                    break;
                case SHUTTER: 
                    LOG(INFO) << "[COM]  Received SHUTTER";
                    //TODO
                    reply_str = "ok";
                    break;
                default:
                    LOG(WARNING) << "[COM] invalid command: " << args[0];
                    reply_str = "no";
                    break;
            }                
        } 
        //sleep(1);

        //create reply message
        zmq::message_t reply(reply_str.length());
        memcpy ((void *) reply.data (), reply_str.c_str(), reply_str.length());

        usleep(20); //CHECK: necessary?
        LOG(INFO) << "[COM] sending reply: " << std::string(static_cast<char*>(reply.data()), reply.size()) << ".";
        socket.send (reply);
        usleep(100);
    }
    com_running = false;
}


//attempts twice to lock mutex, write values, and unlock mutex
//returns: true if successfully sent rotation to PosControl,
//         false if not
bool enqRotation(int num_args, int *args) {
    if(num_args != 3) {
        LOG(WARNING) << "[COM] Rotation: wrong number of arguments: " << num_args << "!=3";
    }
    else if(args[0] != SET_ROTATION) {
        LOG(WARNING) << "[COM] Rotation: incorrect arguments: " << args[1];  //should never happen      
    }
    else {
        if(read_mutex.try_lock()) {
            p->enqueue(args[1], 0, 0, args[2], 0, ROTATION);
            read_mutex.unlock();
            return true;
        } 
        else {
            usleep(1000);
            if(read_mutex.try_lock()) {
                p->enqueue(args[1], 0, 0, args[2], 0, ROTATION);
                read_mutex.unlock();
                return true;
            }
        }
        LOG(WARNING) << "[COM] try_lock read_mutex unsuccessful";
    }
    return false;
}


//see enqRotation
bool enqPosition(int num_args, int *args) {
    if(num_args != 4) {
        LOG(WARNING) << "[COM] Position: wrong number of arguments: " << num_args << "!=4";
    }
    else if(args[0] != SET_POSITION) {
        LOG(WARNING) << "[COM] Position: invalid argument: " << args[0];  //should never happen      
    }
    else {
        if(read_mutex.try_lock()) {
            p->enqueue(args[1], args[2], args[3], 0, 0, POSITION);
            read_mutex.unlock();
            return true;
        } 
        else {
            usleep(1000);
            if(read_mutex.try_lock()) {
                p->enqueue(args[1], args[2], args[3], 0, 0, POSITION);
                read_mutex.unlock();
                return true;
            }
        }
        LOG(WARNING) << "[COM] try_lock read_mutex unsuccessful";
    }
    return false;
}


bool enqAction(int num_args, int *args) {
    if (num_args != 3) {
        LOG(WARNING) << "[COM] Action: wrong number of arguments: " << num_args << "!=3";        
    }
    else if(args[0] != 4 && args[0] != 5 && args[0] != 6) {
        LOG(WARNING) << "[COM] Action: invalid argument: " << args[0];  //should never happen      
    }
    else {
        if(read_mutex.try_lock()) {
            p->enqueue(args[1], 0, 0, 0, args[2], args[0]); //args[0] = ACTIONTYPE
            read_mutex.unlock();
            return true;
        } 
        else {
            usleep(1000);
            if(read_mutex.try_lock()) {
                p->enqueue(args[1], 0, 0, 0, args[2], args[0]);
                read_mutex.unlock();
                return true;
            }
        }
        LOG(WARNING) << "[COM] try_lock read_mutex unsuccessful";
    }
    return false;
}


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


bool checkArguments(int argc, char *argv[]) {
    LOG(INFO) << "[SETUP] Reading arguments:   ";
    //m->serialSimDisable(); //just because

    opterr = 0;
    char *cval = nullptr;
    int opt;
    while( (opt = getopt(argc, argv, "hstdnm:l:")) != -1 ) {
        switch(opt) {
            case 'h':
                //print help
                PRINTLINE("---------- CMD-LINE OPTIONS ----------");
                PRINTLINE("-s: enable simulation of serial");
                PRINTLINE("-t: enable test-mode");
                PRINTLINE("-d: enable debug-file");
                PRINTLINE("-n: disable logging to file");
                PRINTLINE("-m <port>: set motor serial port (ex: ttyUSB0)");
                PRINTLINE("-l <port>: set lift serial port (ex: ttyUSB1)");
                PRINTLINE("--------------------------------------");
                break;
            case 's':
                PRINTLINE("[SETUP]     SerialSim enabled. <<");
                sim_enabled = true;
                break;
            case 't':
                PRINTLINE("[SETUP]     Testmode enabled. <<");   
                testing_enabled = true;
                break;
            case 'd':
                PRINTLINE("[SETUP]     Debug file enabled. <<");
                debug_file_enabled = true;
                break;
            case 'n':
                PRINTLINE("[SETUP]     Log to file disabled. <<");
                log_to_file = false;
                break;
            case 'm':
                //motor_serial = std::to_string(optarg);
                cval = optarg;
                PRINTLINE("[SETUP]    motor arg=" << cval);

                break;
            case 'l':
                //lift_serial = std::to_string(optarg);
                lift_serial = optarg;
                PRINTLINE("[SETUP]    lift arg=" << optarg);
                break;
            case '?':
                if(optopt == 'm') {
                    PRINTLINE("Option -m requires an argument.");
                } else if(optopt == 'l') {
                    PRINTLINE("Option -l requires an argument.");                    
                } else {
                    PRINTLINE("Unknown cmd-option. (-h for help).");
                }
                break;                
            default:
                PRINTLINE("Invalid cmd-option. (-h for help).");
                break;
        }
    }

    return true;
}


void configureLogger() {
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    //el::Loggers::addFlag( el::LoggingFlag::DisableApplicationAbortOnFatalLog );

    defaultConf.setGlobally( el::ConfigurationType::Format, "%datetime{%H:%m:%s,%g} %level %msg" );
    defaultConf.set(el::Level::Global, 
        el::ConfigurationType::Filename, "/home/eivinwi/EurobotUiO/NoGui/newlogs/std.log"
    );
    defaultConf.set(el::Level::Global,
        el::ConfigurationType::ToStandardOutput, "TRUE"
    );


    if(log_to_file) {
        defaultConf.setGlobally( el::ConfigurationType::ToFile, "TRUE"); 
    } else {
        defaultConf.setGlobally( el::ConfigurationType::ToFile, "FALSE"); 
    }


    if(debug_file_enabled) {
        defaultConf.set(el::Level::Debug, 
            el::ConfigurationType::Enabled, "TRUE"
        );
    }
    else {
        defaultConf.set(el::Level::Debug, 
            el::ConfigurationType::Enabled, "FALSE"
        );
    }    

    defaultConf.set(el::Level::Debug, 
        el::ConfigurationType::Filename, "/home/eivinwi/EurobotUiO/NoGui/newlogs/debug.log"
    );
    defaultConf.set(el::Level::Debug, 
        el::ConfigurationType::ToStandardOutput, "FALSE"
    );
    defaultConf.set(el::Level::Trace, 
        el::ConfigurationType::Format, "%datetime{%m:%s:%g} %msg"
    );
    defaultConf.set(el::Level::Trace, 
        el::ConfigurationType::Filename, "/home/eivinwi/EurobotUiO/NoGui/newlogs/position.log"
    );
    defaultConf.set(el::Level::Trace, 
        el::ConfigurationType::ToStandardOutput, "FALSE"
    );
    el::Loggers::reconfigureAllLoggers(defaultConf);
}


int main(int argc, char *argv[]) {  
    PRINTLINE("[SETUP] checking cmdline-arguments");
    if(!checkArguments(argc, argv)) {
        return -1;
    }
    PRINTLINE("[SETUP] Configuring loggers");
    configureLogger();
    LOG(INFO) << "[SETUP] Attaching crashHandler";
    el::Helpers::setCrashHandler(crashHandler);


    LOG(INFO) << "[SETUP] creating MotorCom";
    m = new MotorCom(motor_serial, sim_enabled);

    LOG(INFO) << "[SETUP] creating LiftCom";
    l = new LiftCom(lift_serial);


    LOG(INFO) << "[SETUP] starting and flushing serials";
    m->startSerial();
    l->startSerial();
    usleep(100000);
    m->flush();

    LOG(INFO) << "[SETUP] resetting encoders";
    m->resetEncoders();
    usleep(5000);


    LOG(INFO) << "[SETUP] initializing PosControl";
    p = new PosControl(m, l, testing_enabled);

    LOG(INFO) << "[SETUP] initializing readLoop thread";
    std::thread read_thread(readLoop);
    usleep(5000);

 //   LOG(INFO) << "[SETUP] initializing writeLoop thread");
 //  std::thread write_thread(writeLoop);


    LOG(INFO) << "[SETUP] initializing controlLoop thread";
    std::thread pos_thread(&PosControl::controlLoop, p);
    usleep(5000);

    //m->testSerial();

    int acc2 = m->getAcceleration(); 
    if(acc2 != ACCELERATION) {
        LOG(INFO) << "[SETUP] Acceleration is: " << acc2 << ", setting new acceleration: " << ACCELERATION;
        usleep(1000);
        m->setAcceleration(ACCELERATION);
    }
    
    usleep(1000);
    int mode = m->getAcceleration();
    if(mode != 0) {
        LOG(INFO) << "[SETUP] Mode is: " << m << ", setting new mode: " << MODE;
        usleep(1000);
        m->setMode(MODE);
    }

    testSystem();

    LOG(INFO) << "[SETUP] testing_enabled completed, waiting for client input";
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
    LOG(INFO) << "[SETUP] Complete, testing_enabled components:\n";

    //test LOGging
    printResult("[TEST] Logging: ", true); //pointless, if LOGging isnt active nothing will be written

    if(m->test()) {
        printResult("[TEST] MotorCom active", true);
        if(m->isSimulating()) printResult("[TEST]     sSerial open", true);
        else           printResult("[TEST]      Serial open (sim)", true);
    } 

    if(l->test()) {
        printResult("[TEST] LiftCom active", true);
        printResult("[TEST]     Serial open", true);
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
        LOG(INFO) << std::left << std::setw(30) << text
             << std::right << std::setw(30) << "\033[0;32m[ok]\033[0m";
    } else {
        LOG(ERROR) << std::left << std::setw(30) << text
             << std::right << std::setw(30) << "\033[0;31m[fail]\033[0m";
    }
}


void crashHandler(int sig) {
    if(sig == SIGINT) {
        LOG(ERROR) << "Program interrupted by user";
    } else {
        LOG(ERROR) << "Unintended program crash!";
        bool stackTraceIfAvailable = false;
        const el::Level& level = el::Level::Fatal;
        const char* logger = "default";
        el::Helpers::logCrashReason(sig, stackTraceIfAvailable, level, logger);
//        el::Helpers::logCrashReason(sig, true);
    }
    el::Helpers::crashAbort(sig);
}