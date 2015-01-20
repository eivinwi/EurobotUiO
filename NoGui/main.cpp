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
#include "liftcom.h"
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
INITIALIZE_EASYLOGGINGPP


bool checkArguments(int argc, char *argv[]);
void drive();
void testDrive();
int getArguments(std::string input, int *pos);
bool enqRotation(int num_args, int *args);
bool enqPosition (int num_args, int *args);
bool enqAction (int num_args, int *args);
void readLoop();
void testSystem();
void printResult(std::string text, bool success);
void crashHandler(int sig);


MotorCom *m;
LiftCom *l;
PosControl *p;
std::mutex read_mutex;

int ACCELERATION = 1;
int MODE = 0;
bool sound_enabled = false;
bool com_running = false;
bool debug_printing = false;
bool testing = false;


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
        LOG(INFO) << "[COM] recv_str: " << recv_str;


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


/* Checks cmd-line arguments 
 * return:
 *      true - if good/no arguments
 *      false - if invalid argument exists
 *
*/
bool checkArguments(int argc, char *argv[]) {
    LOG(INFO) << "[SETUP] Reading arguments:   ";
    m->serialSimDisable(); //just because
    if(argc < 2) {
        LOG(INFO) << "[SETUP]    No arguments - expecting serial at: /dev/ttyUSB0";
        m->setSerialPort("ttyUSB0");
    } else {
        std::stringstream str;
        for(int i = 0; i < argc; i++) {
            str << argv[i] << " ";
        }
        LOG(INFO) << "[SETUP] Arguments: " << str.str();

        for(int i = 1; i < argc; i++) {
            if(strcmp(argv[i], "sim") == 0) {
                LOG(INFO) << "[SETUP]     Simulating serial.";
                m->serialSimEnable();
            } 
            else if(strcmp(argv[i], "testing") == 0) {
                LOG(INFO) << "[SETUP]     Testing enabled.";
                testing = true;
            }
            else if(strcmp(argv[i], "sound") == 0) {
                LOG(INFO) << "[SETUP]     Sound enabled.";
                sound_enabled = true;
            }
            else if(strcmp(argv[i], "debug") == 0) {
                LOG(INFO) << "[SETUP]     Enabling debug-logging";
                debug_printing = true;
            }
            else if(strcmp(argv[i], "ttyACM0") == 0) {
                LOG(INFO) << "[SETUP]     Opening serial on: /dev/" << argv[i];
                m->setSerialPort(argv[1]);
            }
            else if(strcmp(argv[i], "ttyS0") == 0) {
                LOG(INFO) << "[SETUP]     Opening serial on: /dev/" << argv[i];
                m->setSerialPort(argv[1]);
            }
            else if(strcmp(argv[i], "ttyACM1") == 0) {
                LOG(INFO) << "[SETUP]     Opening serial on: /dev/" << argv[i];
                m->setSerialPort(argv[1]);
            }
            else if(strcmp(argv[i], "ttyUSB1") == 0) {
                LOG(INFO) << "[SETUP]     Opening serial on: /dev/" << argv[i];
                m->setSerialPort(argv[1]);
            } 
            else {
                LOG(INFO) << "[SETUP]     Invalid argument: " << argv[i];
                return false;
            }
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

    if(debug_printing) {
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
    LOG(INFO) << "[SETUP] Reconfiguring loggers";
    configureLogger();
    LOG(INFO) << "[SETUP] Attaching crashHandler";
    el::Helpers::setCrashHandler(crashHandler);

    LOG(INFO) << "[SETUP] creating MotorCom";
    m = new MotorCom;

    LOG(INFO) << "[SETUP] creating LiftCom";
    l = new LiftCom();

    LOG(INFO) << "[SETUP] checking cmdline-arguments";
    if(!checkArguments(argc, argv)) {
        return -1;
    }

    LOG(INFO) << "[SETUP] starting and flushing serials";
    m->startSerial();
    l->startSerial();
    usleep(100000);
    m->flush();

    LOG(INFO) << "[SETUP] resetting encoders";
    m->resetEncoders();
    usleep(5000);


    LOG(INFO) << "[SETUP] initializing PosControl";
    p = new PosControl(m, l, testing);

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

    LOG(INFO) << "[SETUP] Testing completed, waiting for client input";
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
    LOG(INFO) << "[SETUP] Complete, testing components:\n";

    //test LOGging
    printResult("[TEST] Logging: ", true); //pointless, if LOGging isnt active nothing will be written

    if(m->test()) {
        printResult("[TEST] MotorCom active", true);
        if(m->isSimulating()) printResult("[TEST] Serial open", true);
        else           printResult("[TEST] Serial open (sim)", true);
    } 

    if(l->test()) {
        printResult("[TEST] MotorCom active", true);
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
        el::Helpers::logCrashReason(sig, true);
    }
    bool stackTraceIfAvailable = false;
    const el::Level& level = el::Level::Fatal;
    const char* logger = "default";
    el::Helpers::logCrashReason(sig, stackTraceIfAvailable, level, logger);
    el::Helpers::crashAbort(sig);
}