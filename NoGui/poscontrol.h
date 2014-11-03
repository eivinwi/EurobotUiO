/* 
 * File: poscontrol.h
 * Author: Eivind Wikheim
 *
 * TODO:
 * - driver robot from its current position to goal position in a controlled manner.
 * - tune algorithms for accuracy
 *
 * INPUTS via zeromq in communication.h:
 * - from AI: goal pos, goal orientation
 * - from SENS: true position
 *
 *
 * The resolution of the positioning system is 3000x2000, where 1res = 1cm
 */

#ifndef POSCONTROL_H
#define POSCONTROL_H

#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include <cassert>


#define XRES 3000
#define YRES 2000

void *worker_routine(void *arg);

class Poscontrol {
public:
    Poscontrol();
    ~Poscontrol();




private:
	std::string in;

};

#endif /* POSCONTROL_H */


