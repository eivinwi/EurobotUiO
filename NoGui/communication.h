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
