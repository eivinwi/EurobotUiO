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
 
#include "main.h"


// ZMQ server waiting for input from AI.
void readLoop() {
    LOG(INFO) << "[COM] starting";
    com_running = true;
    // Prepare context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind ("tcp://*:5900");
    //socket.bind ("ipc://ai.ipc");

    while (true) {
        zmq::message_t request;
        // Wait for next request from client
        socket.recv (&request);
        std::string recv_str = std::string(static_cast<char*>(request.data()), request.size());
        std::string reply_str;
        int args[4];
        int num_args = getArguments(recv_str, args);

        LOG(INFO) << "[COM] input str(" << num_args << "): " << recv_str;
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
                case SET_REVERSE: 
                    LOG(INFO) << "[COM]  Received SET_REVERSE(id=" << args[1] << ", x=" << args[2] << ", y=" << args[3] << ")";
                    enqPosition(num_args, args, REVERSE);
                    reply_str = "ok";
                    break;             
                case SET_FORWARD: 
                    LOG(INFO) << "[COM]  Received SET_FORWARD(id=" << args[1] << ", x=" << args[2] << ", y=" << args[3] << ")";
                    enqPosition(num_args, args, FORWARD);
                    reply_str = "ok";
                    break;
                case SET_ROTATION: 
                    LOG(INFO) << "[COM]  Received SET_ROTATION(id=" << args[1] << ", r=" << args[2] << ")";
                    enqRotation(num_args, args);
                    reply_str = "ok";
                    break;
                case LIFT: 
                    LOG(INFO) << "[COM]  Received LIFT";
                    enqAction(num_args, args);
                    reply_str = "ok";
                    break;
                case SHUTTER: 
                    LOG(INFO) << "[COM]  Received SHUTTER";
                    //TODO
                    reply_str = "ok";
                    break;
                case STRAIGHT:
                    LOG(INFO) << "[COM]  Received STRAIGHT";
                    enqStraight(num_args, args);
                    reply_str = "ok";
                    break;
                case HALT: 
                    LOG(INFO) << "[COM] Recieved HALT";
                    p->halt();
                    reply_str = "ok";
                    break;
                case RESET: 
                    LOG(INFO) << "[COM]  Received RESET";
                    resetRobot(num_args, args);
                    reply_str = "ok";
                    break;
                case SOUND:
                    playSound(num_args, args);
                    reply_str = "ok";
                    break;
                default:
                    LOG(WARNING) << "[COM] invalid command: " << args[0];
                    reply_str = "no";
                    break;
            }                
        } 

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


void subscriptionLoop() {
    zmq::context_t context (1);
    LOG(INFO) << "[COM2] starting";
    zmq::socket_t subscriber (context, ZMQ_SUB);
    //subscriber.connect("ipc://pos.ipc");
    subscriber.connect("tcp://localhost:5556");

    subscriber.setsockopt(ZMQ_SUBSCRIBE, NULL, 0); //, filter, strlen (filter));

    int args[3]; // (x, y, r)    
    std::string str;
    while(1) {
        zmq::message_t update;
        subscriber.recv(&update);
        std::istringstream iss(static_cast<char*>(update.data()));
        iss >> str;
        
        int num_args = getArguments(str, args);
        if(num_args != 3) {
            LOG(WARNING) << "[COM2] Invalid position-string!!" << str.c_str();
        } else {
            PRINTLINE( "YEAH" );
            LOG(INFO) << "[COM2] recv: [" << args[0] << ", " << args[1] << ", " << args[2] << "]";
        }
        usleep(100);
    }
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
bool enqPosition(int num_args, int *args, int dir) {
    if(num_args != 4) {
        LOG(WARNING) << "[COM] Position: wrong number of arguments: " << num_args << "!=4";
    }
    else if(args[0] != SET_FORWARD && args[0] != SET_REVERSE) {
        LOG(WARNING) << "[COM] Position: invalid argument: " << args[0];  //should never happen      
    }
    else {
        if(read_mutex.try_lock()) {
            p->enqueue(args[1], args[2], args[3], 0, 0, dir);
            read_mutex.unlock();
            return true;
        } 
        else {
            usleep(1000);
            if(read_mutex.try_lock()) {
                p->enqueue(args[1], args[2], args[3], 0, 0, dir);
                read_mutex.unlock();
                return true;
            }
        }
        LOG(WARNING) << "[COM] try_lock read_mutex unsuccessful";
    }
    return false;
}


//see enqRotation
bool enqStraight(int num_args, int *args) {
    if(num_args != 3) {
        LOG(WARNING) << "[COM] Straight: wrong number of arguments: " << num_args << "!=4";
    }
    else {
        if(read_mutex.try_lock()) {
            p->enqueue(args[1], args[2], 0, 0, 0, STRAIGHT);
            read_mutex.unlock();
            return true;
        } 
        else {
            usleep(1000);
            if(read_mutex.try_lock()) {
                p->enqueue(args[1], args[2], 0, 0, 0, STRAIGHT);
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


bool resetRobot(int num_args, int *args) {
    if (num_args != 3) {
        LOG(WARNING) << "[COM] Action: wrong number of arguments: " << num_args << "!=3";        
    }
    else {
        if(read_mutex.try_lock()) {
            p->reset(args[1], args[2], args[3]);
            read_mutex.unlock();
            return true;
        } 
        else {
            usleep(1000);
            if(read_mutex.try_lock()) {
                p->reset(args[1], args[2], args[3]);
                read_mutex.unlock();
                return true;
            }
        }
        LOG(WARNING) << "[COM] try_lock read_mutex unsuccessful";
    }
    return false;
}


//TODO
void playSound(int num_args, int *args) {
    if(num_args > 1) {

    }
}


int getArguments(std::string input, int *pos) {
    int i = 0;
    std::istringstream f(input);
    std::string s;
    while(getline(f, s, ',')) {
        pos[i] = atoi(s.c_str());
        i++;
//        if(i > 4) break; //hack!! probably incorrect
    }
    return i;
}


bool checkArguments(int argc, char *argv[]) {
    LOG(INFO) << "[SETUP] Reading arguments:   ";
    opterr = 0;
    int opt;

    while( (opt = getopt(argc, argv, "hstflnm:d:")) != -1 ) {
        switch(opt) {
            case 'h':
                //print help
                PRINTLINE("---------- CMD-LINE OPTIONS ----------");
                PRINTLINE("-s: enable simulation of motor-serial");
                PRINTLINE("-l: enable simulation of lift-serial");
                PRINTLINE("-t: enable test-mode");
                PRINTLINE("-f: enable debug-file");
                PRINTLINE("-n: disable logging to file");
                PRINTLINE("-m <port>: set motor serial port (ex: ttyUSB0)");
                PRINTLINE("-d <port>: set dynamixel (lift) serial port (ex: ttyUSB1)");
                PRINTLINE("--------------------------------------");
                break;
            case 's':
                PRINTLINE("[SETUP]     Simulating motor-serial <<");
                sim_motors = true;
                break;
            case 'l':
                PRINTLINE("[SETUP]     Simulating lift-serial <<");
                sim_lift = true;                
                break;
            case 't':
                PRINTLINE("[SETUP]     Testmode enabled. <<");   
                testing_enabled = true;
                break;
            case 'f':
                PRINTLINE("[SETUP]     Debug file enabled. <<");
                debug_file_enabled = true;
                break;
            case 'n':
                PRINTLINE("[SETUP]     Log to file disabled. <<");
                log_to_file = false;
                break;
            case 'm':
                motor_serial = optarg;
                PRINTLINE("[SETUP]    motor arg=" << optarg);

                break;
            case 'd':
                dyna_serial = optarg;
                PRINTLINE("[SETUP]    dyna arg=" << optarg);
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


void testSystem() {
    usleep(20000);
    m->flush();
    LOG(INFO) << "[SETUP] Complete, testing components:\n";

    //test LOGging
    printResult("[TEST] Logging: ", true); //pointless, if Logging isnt active nothing will be written

    if(m->isSimulating()) {
        printResult("[TEST] MotorCom: serialsim", true);
    } else {
        printResult("[TEST] MotorCom: serial open", m->test());
    }

    printResult("[TEST] DynaCom: lift online", d->testLift());
    printResult("[TEST] DynaCom: gripper online", d->testGripper());

    printResult("[TEST] PosControl active", p->test()); //poscontrol test
    printResult("[TEST] Read_thread running", com_running);
    printResult("[TEST] Pos_thread running", p->running());

    uint8_t voltage = m->getVoltage();
    printResult("[TEST] Voltage = " + std::to_string((int)voltage) +"v", (voltage > 20 && voltage < 25));
    uint8_t error = m->getError();
    printResult("[TEST] MD49_Error = " + std::to_string((int) error), (error == 0));
    int acc = m->getAcceleration();
    printResult("[TEST] Acceleration = " + std::to_string(acc), (acc == ACCELERATION));  
    int mode = m->getMode();
    printResult("[TEST] Mode = " + std::to_string(mode), (mode == MODE));
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


void configureLogger() {
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    //el::Loggers::addFlag( el::LoggingFlag::DisableApplicationAbortOnFatalLog );
    defaultConf.setGlobally( el::ConfigurationType::Format, "%datetime{%H:%m:%s,%g} %level %msg" );
    defaultConf.set(el::Level::Global, el::ConfigurationType::Filename, "/home/eivinwi/EurobotUiO/Logs/std.log");
    defaultConf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "TRUE");

    if(log_to_file) {
        defaultConf.setGlobally( el::ConfigurationType::ToFile, "TRUE"); 
    } else {
        defaultConf.setGlobally( el::ConfigurationType::ToFile, "FALSE"); 
    }

    if(debug_file_enabled) {
        defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "TRUE");
    }
    else {
        defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "FALSE");
    }    

    defaultConf.set(el::Level::Debug, el::ConfigurationType::Filename, "/home/eivinwi/EurobotUiO/Logs/debug.log");
    defaultConf.set(el::Level::Debug, el::ConfigurationType::ToStandardOutput, "FALSE");

    defaultConf.set(el::Level::Trace, el::ConfigurationType::Format, "%datetime{%m:%s:%g} %msg");
    defaultConf.set(el::Level::Trace, el::ConfigurationType::Filename, "/home/eivinwi/EurobotUiO/Logs/position.log");
    defaultConf.set(el::Level::Trace, el::ConfigurationType::ToStandardOutput, "FALSE");

    el::Loggers::reconfigureAllLoggers(defaultConf);
}


int main(int argc, char *argv[]) {  
    PRINTLINE("[SETUP] checking cmdline-arguments");
    if(!checkArguments(argc, argv)) {
        PRINTLINE("ERROR: Invalid arguments");
        return -1;
    }

    PRINTLINE("[SETUP] Configuring loggers");
    configureLogger();

    LOG(INFO) << "[SETUP] Attaching crashHandler";
    el::Helpers::setCrashHandler(crashHandler);

    LOG(INFO) << "[SETUP] initializing MotorCom";
    m = new MotorCom(motor_serial, sim_motors);
    usleep(10000);

    LOG(INFO) << "[SETUP] initializing DynaCom";
    d = new DynaCom(dyna_serial, sim_lift);
    usleep(10000);

    LOG(INFO) << "[SETUP] starting motor serial";
    m->startSerial();
    usleep(100000); // extra delay for safety, motor controller sometimes need more time
    LOG(INFO) << "[SETUP] flushing motor serial";
    m->flush();

    LOG(INFO) << "[SETUP] resetting motor encoders";
    m->resetEncoders();
    usleep(10000);

    LOG(INFO) << "[SETUP] starting dyna-serial";
    d->startSerial();
    usleep(10000);

    LOG(INFO) << "[SETUP] initializing PosControl";
    p = new PosControl(m, d, testing_enabled);
    usleep(5000);

    LOG(INFO) << "[SETUP] initializing controlLoop thread";
    std::thread pos_thread(&PosControl::controlLoop, p);
    usleep(5000);

    LOG(INFO) << "[SETUP] initializing readLoop thread";
    std::thread read_thread(readLoop);
    usleep(5000);

  //  LOG(INFO) << "[SETUP] initializing subscription thread";
  //  std::thread write_thread(subscriptionLoop);


    int acc = m->getAcceleration(); 
    if(acc != ACCELERATION) {
        LOG(INFO) << "[SETUP] Acceleration is: " << acc << ", setting new acceleration: " << ACCELERATION;
        usleep(1000);
        m->setAcceleration(ACCELERATION);
    }
    
    usleep(1000);
    int mode = m->getMode();
    if(mode != 0) {
        LOG(INFO) << "[SETUP] Mode is: " << m << ", setting new mode: " << MODE;
        usleep(1000);
        m->setMode(MODE);
    }

   // s = new Sound();

    testSystem();

    LOG(INFO) << "\n[SETUP] System tests completed, waiting for client input...\n";
 
    if(read_thread.joinable()) {
        read_thread.join();
    }
    if(pos_thread.joinable()) {
        pos_thread.join();
    }
    return 0;
}