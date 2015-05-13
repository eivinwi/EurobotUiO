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
void aiServer() {
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
        
        std::vector<int> args = extractArguments(recv_str);
        int num_args = args.size();

        LOG(DEBUG) << "[COM] input str(" << num_args << "): " << recv_str;
        if(num_args < 1 || num_args > 5) {
            LOG(WARNING) << "[COM] invalid number arguments(" << num_args << "): " << recv_str;
            reply_str = "no";
        } 
        else {
            switch(args[0]) {
                case REQUEST: 
                    LOG(DEBUG) << "[COM] Recieved REQUEST";
                    if(args[1] == 1) {
                        int id = p->getCurrentId();
                        reply_str = std::to_string(id);
                        LOG(DEBUG) << "[COM] REQUEST ID, ret(" << reply_str.length() << "): " << reply_str;
                    } 
                    else if(args[1] == 2) {
                        reply_str = p->getCurrentPos();
                        LOG(DEBUG) << "[COM] REQUEST POS, ret(" << reply_str.length() << "): " << reply_str;
                    } 
                    else if(args[1] == 4) {
                        //LIFT
                        reply_str = d->getGripperPosition();
                        LOG(DEBUG) << "[COM] REQUEST GRIPPER POS, ret(" << reply_str.length() << "): " << reply_str;
                    }                    
                    break;
                case SET_REVERSE: 
                    LOG(DEBUG) << "[COM]  Received SET_REVERSE(id=" << args[1] << ", x=" << args[2] << ", y=" << args[3] << ")";
                    enqueue(num_args, args);
                    reply_str = "ok";
                    break;             
                case SET_FORWARD: 
                    LOG(DEBUG) << "[COM]  Received SET_FORWARD(id=" << args[1] << ", x=" << args[2] << ", y=" << args[3] << ")";
                    enqueue(num_args, args); //, FORWARD);
                    reply_str = "ok";
                    break;
                case SET_ROTATION: 
                    LOG(DEBUG) << "[COM]  Received SET_ROTATION(id=" << args[1] << ", r=" << args[2] << ")";
                    enqueue(num_args, args);
                    reply_str = "ok";
                    break;
                case LIFT: 
                    LOG(DEBUG) << "[COM]  Received LIFT";
                    enqueue(num_args, args);
                    reply_str = "ok";
                    break;
                case GRIPPER:
                    enqueue(num_args, args);
                    reply_str = "ok";
                    break;
                case SHUTTER: 
                    LOG(DEBUG) << "[COM]  Received SHUTTER";
                    //TODO
                    reply_str = "ok";
                    break;
                case STRAIGHT:
                    LOG(DEBUG) << "[COM]  Received STRAIGHT";
                    enqueue(num_args, args);
                    reply_str = "ok";
                    break;
                case HALT: 
                    LOG(DEBUG) << "[COM] Recieved HALT";
                    p->halt();
                    reply_str = "ok";
                    break;
                case RESET: 
                    LOG(DEBUG) << "[COM]  Received RESET";
                    resetRobot(num_args, args);
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
        LOG(DEBUG) << "[COM] reply: " << std::string(static_cast<char*>(reply.data()), reply.size()) << ".";
        socket.send (reply);
        usleep(100);
    }
    com_running = false;
}

/*
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
        
        int num_args = extractArguments(str, args);
        if(num_args != 3) {
            LOG(WARNING) << "[COM2] Invalid position-string!!" << str.c_str();
        } else {
            PRINTLINE( "YEAH" );
            LOG(INFO) << "[COM2] recv: [" << args[0] << ", " << args[1] << ", " << args[2] << "]";
        }
        usleep(100);
    }
}*/

void posClient() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.connect("tcp://193.157.227.196:5900");

    while(true) {
        std::string s = p->getState();

        zmq::message_t pos( s.length() );
        memcpy((void*) pos.data(), s.c_str(), s.length());
        
        LOG(INFO) << "[COM2] Sending  len=" << s.length() << " to POS";
        socket.send(pos);

        zmq::message_t reply;
        socket.recv( &reply );

        std::string reply_str = std::string(static_cast<char*>(reply.data()), reply.size());
        LOG(INFO) << "[COM2] Reply from POS: len=" << reply.size() << ": " << reply_str;

        usleep(500000); //500ms
    }
    // Error happend, should be handeled
}


bool enqueue(int num_args, std::vector<int> args) {
    std::vector<int> arr(num_args);
    for(int i = 0; i < num_args; i++) {
        arr[i] = args[i];
    }

    if(read_mutex.try_lock()) {
        p->enqueue(arr);
        read_mutex.unlock();
        return true;
    } 
    else {
        usleep(1000);
        if(read_mutex.try_lock()) {
            p->enqueue(arr);
            read_mutex.unlock();
            return true;
        }
    }
    return false;
}


bool resetRobot(int num_args, std::vector<int> args) {
    if (num_args < 4) {
        LOG(WARNING) << "[COM] Action: wrong number of arguments: " << num_args << "<4";        
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


std::vector<int> extractArguments(std::string input) {
    std::vector<int> args;
    std::istringstream f(input);
    std::string s;
    while(getline(f, s, ',')) {
        args.push_back(atoi(s.c_str()));
    }
    return args;    
}


int cmdArgs(int ac, char *av[]) {
    po::options_description desc("Valid options");

    desc.add_options() 
        ("help", "print help message")
        ("testing", "enable testing")
        ("sim", "simulate")
        ("debug", "enable debug logging")
        ("nolog", "disable all logging")
        ("mport", po::value<std::string>(),"MD49 serial port (ex: /dev/ttyUSB0)")
        ("dport", po::value<std::string>(), "Dynamixel serial port (ex: /dev/ttyUSB1")
        ("ai", po::value<int>(), "AI ZMQ port as server (ex: 5900)")
        ("pos", po::value<int>(), "POS ZMQ port as client (ex: 5555)")
        ("config", po::value<std::string>(), "Set YAML config file")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);


    if(vm.count("help")) {
        PRINTLINE(desc);
        return 1;
    }

    if(vm.count("testing")) {
        PRINTLINE("[CFG] Testing enabled")
        testing_enabled = true;
    }

    if(vm.count("sim")) {
        PRINTLINE("[CFG] Simulation enabled")
        sim_motors = true;
    }

    if(vm.count("debug")) {
        PRINTLINE("[CFG] Debug prints enabled")
        debug_file_enabled = true;
    }

    if(vm.count("nolog")) {
        PRINTLINE("[CFG] All logging disabled")
        logging = false;
    }

    if(vm.count("mport")) {
        motor_serial = vm["mport"].as<std::string>(); // << std::endl;
        PRINTLINE("[CFG] Motor serial port = " << motor_serial);
    }

    if(vm.count("dport")) {
        dyna_serial = vm["dport"].as<std::string>();// << std::endl;
        PRINTLINE("[CFG] Dynamixel serial port = " << dyna_serial);
    }

    if(vm.count("ai")) {
        int p = vm["ai"].as<int>();
        if(p > 0 && p < MAX_INT) {
            ai_port = p;
            PRINTLINE("[CFG] AI zmq-port is: " << ai_port);
        } else {
            PRINTLINE("[CFG] AI: Invalid port value: " << p);
        }
    }

    if(vm.count("pos")) {
        int p = vm["pos"].as<int>();
        if(p > 0 && p < MAX_INT) {
            pos_port = p;
            PRINTLINE("[CFG] POS zmq-port is: " << pos_port);
        } else {
            PRINTLINE("[CFG] Invalid zmq port value: " << ai_port);
        } 
    }

    if(vm.count("config")) {
        config_file = vm["config"].as<std::string>();
        PRINTLINE("[CFG] config file is: " << config_file);
    }

    return 0;
}


void testSystem() {
    usleep(20000);
    m->flush();
    LOG(INFO) << "[SETUP] Complete, testing components:\n";

    printResult("[TEST] Logging: ", true); //pointless, if logging isnt active nothing will be written

    if(m->isSimulating()) {
        printResult("[TEST] MotorCom: serialsim", true);
    } else {
        printResult("[TEST] MotorCom: serial open", m->test());
    }

    printResult("[TEST] DynaCom: gripper online", d->testGripper());
    printResult("[TEST] PosControl active", p->test()); 
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
    //if(!checkArguments(argc, argv)) {
    //    PRINTLINE("ERROR: Invalid arguments");
    //    return -1;
    //}
    cmdArgs(argc, argv);

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
    m->enableReg(true);
    usleep(10000);

    LOG(INFO) << "[SETUP] starting dyna-serial";
    d->startSerial();
    usleep(10000);

    LOG(INFO) << "[SETUP] initializing PosControl";
    p = new PosControl(m, d, testing_enabled, config_file);
    usleep(5000);

    LOG(INFO) << "[SETUP] initializing controlLoop thread";
    std::thread pos_thread(&PosControl::controlLoop, p);
    usleep(5000);

    LOG(INFO) << "[SETUP] initializing aiServer thread";
    std::thread read_thread(aiServer);
    usleep(5000);

 //   LOG(INFO) << "[SETUP] initializing POS thread";
 //   std::thread write_thread(posClient);

    int acc = m->getAcceleration(); 
    if(acc != ACCELERATION) {
        LOG(INFO) << "[SETUP] Acceleration is: " << acc << ", setting new acceleration: " << ACCELERATION;
        usleep(1000);
        m->setAcceleration(ACCELERATION);
    }
    
    usleep(5000);
    int mode = m->getMode();
    if(mode != 0) {
        LOG(INFO) << "[SETUP] Mode is: " << m << ", setting new mode: " << MODE;
        usleep(1000);
        m->setMode(MODE);
    }

    testSystem();
    std::vector<int> testAction{5, 0, 4};
    d->performAction(testAction);
    LOG(INFO) << "\n[SETUP] System tests completed, waiting for client input...\n";
 
    if(read_thread.joinable()) {
        read_thread.join();
    }
    if(pos_thread.joinable()) {
        pos_thread.join();
    }
    return 0;
}
