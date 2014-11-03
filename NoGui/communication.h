/*  
 * File: communication.h
 * Author: Eivind Wikheim 
 *
 * TODO:
 * Implement a interface for communication between POS <-> AI 
 * 
 * Possibly TODO:
 * Implement a interface for communication between POS <-> SENS
 */

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include <cassert>

void *worker_routine(void *arg);

class Communication {
public:
    Communication();
    ~Communication();




private:
	std::string in;

};

#endif /* COMMUNICATION_H */
