#include "dynacom.h"
#include "motorcom.h"
#include "liftcom.h"
#include "poscontrol.h"
#include "printing.h"
//#include "sound.h"
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
void aiServer();

// Implements a ZMQ subscriber that reads current real position from a publisher
void posClient();

// Reads command-line arguments. Uses GNU C++ GetOpt standard.
// Returns true if no invalid arguments
bool checkArguments(int argc, char *argv[]);

// Used by the aiServer ZMQ-server thread.
// Splits input from Client on delimiter, fills pos 2d-array with arguments.
// Returns number of arguments
int getArguments(std::string input, int *pos);

// Sends rotation-change to PosControl to be added to command-queue
bool enqRotation(int num_args, int *args);

// Sends position-change to PosControl to be added to command-queue. Dir is FORWARD or REVERSE
bool enqPosition(int num_args, int *args, int dir);

// Sends command to the robot to drive forward in current rotation.
bool enqStraight(int num_args, int *args);

// Sends action to PosControl to be added to command-queue
bool enqAction(int num_args, int *args);

// Reset robot to intial configuration, then set position to the one provided
bool resetRobot(int num_args, int *args);

//TODO
void playSound(int num_args, int *args);

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
//LiftCom *l;
PosControl *p;
DynaCom *d;
//Sound *s;

// locking objects while aiServer is writing to them.
std::mutex read_mutex;

// MD49 constants
const int ACCELERATION = 5;
const int MODE = 0;

// Default values, can be changed with command-line arguments
bool sim_motors = false;
bool sim_lift = false;

bool sound_enabled = false;
bool com_running = false;
bool debug_file_enabled = false;
bool testing_enabled = false;
bool log_to_file = true;
std::string motor_serial = "/dev/ttyUSB0";
std::string dyna_serial = "/dev/ttyUSB1";

float enc_per_degree = 0.0;