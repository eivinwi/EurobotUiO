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
    
  /*  char port[1024];
    size_t size = sizeof(port);
    try {*/
     //   socket.bind("tcp://*:*");
 /*   } catch (zmq::error_t&e ){
        std::cerr << "couldn't bind to socket: " << e.what();
        //return e.num();
    }
    socket.getsockopt( ZMQ_LAST_ENDPOINT, &port, &size );
    std::cout << "socket is bound at port " << port << std::endl;
*/
    std::stringstream ss;
    ss << "tcp://*:" << ai_port;
   // PRINTLINE("Connecting to AI on: <" << ss.str().c_str() << ">");
    //socket.bind (ss.str().c_str());
       //socket.bind ("ipc://ai.ipc");
    socket.bind("tcp://*:5555");

    while (true) {
        zmq::message_t request;
        // Wait for next request from client
        socket.recv (&request);
        std::string recv_str = std::string(static_cast<char*>(request.data()), request.size());
        std::string reply_str;
        
        std::vector<int> args = extractInts(recv_str);
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
                        //int id = posctrl->getCurrentId();
                        int id = 0;
                        reply_str = std::to_string(id);
                        LOG(DEBUG) << "[COM] REQUEST ID, ret(" << reply_str.length() << "): " << reply_str;
                    } 
                    else if(args[1] == 2) {
                        reply_str = posctrl->getState();
                        LOG(DEBUG) << "[COM] REQUEST POS, ret(" << reply_str.length() << "): " << reply_str;
                    } 
                    else if(args[1] == 4) {
                        //LIFT
                        reply_str = dcom->getGripperPosition();
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
  /*              case SHUTTER: 
                    LOG(DEBUG) << "[COM]  Received SHUTTER";
                    //TODO
                    reply_str = "ok";
                    break;
    */            case 6:
                    LOG(INFO) << "[COM] Received GOAL";
                    enqueue(num_args, args);
                    reply_str = "ok";
                    break;
                case STRAIGHT:
                    LOG(DEBUG) << "[COM]  Received STRAIGHT";
                    enqueue(num_args, args);
                    reply_str = "ok";
                    break;
                case HALT: 
                    LOG(DEBUG) << "[COM] Recieved HALT";
                    posctrl->stopAll((int) args[1]);
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


void posClient() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.connect(pos_ip_port.c_str());
    PRINTLINE("Pos connected on: <" << pos_ip_port.c_str() << ">");

    while(true) {
        std::stringstream ss;
        ss << "1," << posctrl->getPosStrDivided();
        std::string s = ss.str();
        //std::string s = "3";
        LOG(INFO) << "[COM2] sending to POS: <" << s << ">";
        zmq::message_t req( s.length() );
        memcpy((void*) req.data(), s.c_str(), s.length());
        socket.send(req);

        zmq::message_t reply;
        socket.recv( &reply );

        std::string reply_str = std::string(static_cast<char*>(reply.data()), reply.size());

        std::vector<float> rpl = extractFloats(reply_str);
        std::stringstream stream;
        for( float f : rpl) {
            stream << "    |    " << f;
        }
        LOG(INFO) << "[COM2] Reply from POS: len=" << reply.size() << ": " << stream.str();
        usleep(2000000);
    }
}


bool enqueue(int num_args, std::vector<int> args) {
    std::vector<int> arr(num_args);
    for(int i = 0; i < num_args; i++) {
        arr[i] = args[i];
    }

    if(read_mutex.try_lock()) {
        posctrl->enqueue(arr);
        read_mutex.unlock();
        return true;
    } 
    else {
        usleep(1000);
        if(read_mutex.try_lock()) {
            posctrl->enqueue(arr);
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
            posctrl->reset(args[1], args[2], args[3]);
            read_mutex.unlock();
            return true;
        } 
        else {
            usleep(1000);
            if(read_mutex.try_lock()) {
                posctrl->reset(args[1], args[2], args[3]);
                read_mutex.unlock();
                return true;
            }
        }
        LOG(WARNING) << "[COM] try_lock read_mutex unsuccessful";
    }
    return false;
}


std::vector<int> extractInts(std::string input) {
    std::vector<int> args;
    std::istringstream f(input);
    std::string s;
    while(getline(f, s, ',')) {
        args.push_back(atoi(s.c_str()));
    }
    return args;    
}


std::vector<float> extractFloats(std::string input) {
    std::vector<float> args;
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
        ("ai", po::value<std::string>(), "AI ZMQ port as server (ex: 5900)")
        ("pos", po::value<int>(), "POS ZMQ port as client (ex: 5555)")
        ("pos_ip_port", po::value<std::string>(), "POS ZMQ ip:port (ex: tcp://193.157.206.209:5432")
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
        ai_port = vm["ai"].as<std::string>();
        PRINTLINE("[CFG] AI zmq-port is: " << ai_port);
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

    if(vm.count("pos_ip_port")) {
        std::string pos_ip_port = vm["pos_ip_port"].as<std::string>();
        PRINTLINE("[CFG] POS zmq address is: " << pos_ip_port);
    }
    if(vm.count("config")) {
        config_file = vm["config"].as<std::string>();
        PRINTLINE("[CFG] config file is: " << config_file);
    }
    return 0;
}


void testSystem() {
    usleep(20000);
    mcom->flush();
    LOG(INFO) << "[SETUP] Complete, testing components:\n";

    printResult("[TEST] Logging: ", true); //pointless, if logging isnt active nothing will be written

    if(mcom->isSimulating()) {
        printResult("[TEST] MotorCom: serialsim", true);
    } else {
        printResult("[TEST] MotorCom: serial open", mcom->test());
    }

    printResult("[TEST] DynaCom: gripper online", dcom->testGripper());
    printResult("[TEST] PosControl active", posctrl->test()); 
    printResult("[TEST] Read_thread running", com_running);
    printResult("[TEST] Pos_thread running", posctrl->running());

    uint8_t voltage = mcom->getVoltage();
    printResult("[TEST] Voltage = " + std::to_string((int)voltage) +"v", (voltage > 20 && voltage < 25));
    uint8_t error = mcom->getError();
    printResult("[TEST] MD49_Error = " + std::to_string((int) error), (error == 0));
    int acc = mcom->getAcceleration();
    printResult("[TEST] Acceleration = " + std::to_string(acc), (acc == ACCELERATION));  
    int mode = mcom->getMode();
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


//TODO: disable logging if --nolog
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

bool exists_test(const std::string& name) {
        struct stat buffer;   
        return (stat (name.c_str(), &buffer) == 0); 
}

int main(int argc, char *argv[]) {  
    PRINTLINE("[SETUP] checking cmdline-arguments");
    //if(!checkArguments(argc, argv)) {
    //    PRINTLINE("ERROR: Invalid arguments");
    //    return -1;
    //}
    if(cmdArgs(argc, argv) != 0) {
        return 1;
    };

    PRINTLINE("[SETUP] Configuring loggers");
    configureLogger();

    LOG(INFO) << "[SETUP] Attaching crashHandler";
    el::Helpers::setCrashHandler(crashHandler);

    LOG(INFO) << "[SETUP] initializing MotorCom";

    bool usb0 = exists_test("/dev/ttyUSB0");
    bool usb1 = exists_test("/dev/ttyUSB1");
    bool usb2 = exists_test("/dev/ttyUSB2");
    bool usb3 = exists_test("/dev/ttyUSB3");
    LOG(WARNING) << "[SETUP] /dev/ttyUSB0: " << ((usb0)? " online" : "offline"); 
    LOG(WARNING) << "[SETUP] /dev/ttyUSB1: " << ((usb1)? " online" : "offline");
    LOG(WARNING) << "[SETUP] /dev/ttyUSB2: " << ((usb2)? " online" : "offline");
    LOG(WARNING) << "[SETUP] /dev/ttyUSB3: " << ((usb3)? " online" : "offline");

	if(usb0) {
		motor_serial = "/dev/ttyUSB0";
		usb0 = false;
	}	
	else if(usb1) {
		motor_serial = "/dev/ttyUSB1";
		usb1 = false;
	}
	else if(usb2) {
		motor_serial = "/dev/ttyUSB2";
		usb2 = false;
	}	
/*	else {
		LOG(WARNING) << "Too few ports available.";
		return 0;
	}
	if(usb0) {
		dyna_serial = "/dev/ttyUSB0";
		usb0 = false;
	}	
	else if(usb1) {
		dyna_serial = "/dev/ttyUSB1";
		usb1 = false;
	}
	else if(usb2) {
		dyna_serial = "/dev/ttyUSB2";
		usb2 = false;
	}	
	else {
		LOG(WARNING) << "Too few ports available.";
		return 0;
	}*/

	dyna_serial = "/dev/ttyS9";

    mcom = new MotorCom(motor_serial, sim_motors);
    usleep(10000);

    LOG(INFO) << "[SETUP] initializing DynaCom";
    dcom = new DynaCom(dyna_serial, sim_lift);
    usleep(10000);

    LOG(INFO) << "[SETUP] starting motor serial";
    mcom->startSerial();
    usleep(100000); // extra delay for safety, motor controller sometimes need more time
    LOG(INFO) << "[SETUP] flushing motor serial";
    mcom->flush();

    LOG(INFO) << "[SETUP] resetting motor encoders";
    mcom->resetEncoders();
    usleep(10000);
    mcom->enableReg(true);
    usleep(10000);
    mcom->enableTimeout(true);
    usleep(10000);


    LOG(INFO) << "[SETUP] starting dyna-serial";
    dcom->startSerial();
    usleep(10000);

    LOG(INFO) << "[SETUP] initializing PosControl";
    posctrl = new PosControl(mcom, dcom, testing_enabled, config_file);
    usleep(5000);

    LOG(INFO) << "[SETUP] initializing controlLoop thread";
    std::thread pos_thread(&PosControl::controlLoop, posctrl);
    usleep(5000);

    LOG(INFO) << "[SETUP] initializing aiServer thread";
    std::thread read_thread(aiServer);
    usleep(5000);

//    LOG(INFO) << "[SETUP] initializing POS thread";
//    std::thread write_thread(posClient);

    int acc = mcom->getAcceleration(); 
    if(acc != ACCELERATION) {
        LOG(INFO) << "[SETUP] Acceleration is: " << acc << ", setting new acceleration: " << ACCELERATION;
        usleep(1000);
        mcom->setAcceleration(ACCELERATION);
    }
    
    usleep(5000);
    int mode = mcom->getMode();
    if(mode != MODE) {
        LOG(INFO) << "[SETUP] Mode is: " << mode << ", setting new mode: " << MODE;
        usleep(1000);
        mcom->setMode(MODE);
    }

    testSystem();
    std::vector<int> initGrippers{5, 0, 0, 280, 280};
    dcom->performAction(initGrippers);
    dcom->closeShutters();

    LOG(INFO) << "\n[SETUP] System tests completed, waiting for client input...\n";
 
    if(read_thread.joinable()) {
        read_thread.join();
    }
    if(pos_thread.joinable()) {
        pos_thread.join();
    }
 //   if(write_thread.joinable()) {
 //       write_thread.join();
 //   }
    return 0;
}
