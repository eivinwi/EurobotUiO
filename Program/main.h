#include <boost/program_options.hpp>
#include <cstring>
#include <ctype.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <zmq.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dynacom.h"
#include "motorcom.h"
#include "poscontrol.h"
#include "printing.h"
#include "protocol.h"

INITIALIZE_EASYLOGGINGPP
#ifndef ELPP_THREAD_SAFE
#define ELPP_THREAD_SAFE
#endif

#define MAX_INT std::numeric_limits<int>::max()


// Implements a ZMQ server that waits for input from clients. 
// Used for communication with AI.
void aiServer();

// Implements a ZMQ subscriber that reads current real position from a publisher
void posClient();

// Reads command-line arguments. Uses GNU C++ GetOpt standard.
// Returns true if no invalid arguments
//bool checkArguments(int argc, char *argv[]);
int cmdArgs(int ac, char *av[]);

// Used by the aiServer ZMQ-server thread.
// Splits input from Client on delimiter, returns array with arguments
std::vector<int> extractInts(std::string input);
std::vector<float> extractFloats(std::string input);

// Adds current command to queue in PosControl
bool enqueue(int num_args, std::vector<int> args);

// Reset robot to intial configuration, then set position to the one provided
bool resetRobot(int num_args, std::vector<int> args);

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
PosControl *p;
DynaCom *d;

// locking objects while aiServer is writing to them.
std::mutex read_mutex;

// MD49 constants
const int ACCELERATION = 3;
const int MODE = 0;//2;

// Default values, can be changed with command-line arguments
bool sim_motors = false;
bool sim_lift = false;

bool sound_enabled = false;
bool com_running = false;
bool debug_file_enabled = false;
bool testing_enabled = false;
bool log_to_file = true;
bool logging = true;
std::string motor_serial = "/dev/ttyUSB0";
std::string dyna_serial = "/dev/ttyUSB1";
std::string config_file = "config.yaml";
std::string pos_ip_port = "tcp://localhost:5900";
//std::string pos_ip_port = "tcp://193.157.205.194:5900";

std::string ai_port = "5060";
int pos_port = 5900;


namespace po = boost::program_options;