//FILE NOT NEEDED: multithreaded communication moved to main.cpp


/*  
 * File: communication.h
 * Author: Eivind Wikheim 
 *
 * Implements communication between POS and AI parts of the project via Zeromq.
 *
 * TODO:
 * - Send pos only when prompted by main
 * - Thread-safe information sharing (of position)
 *
 * Possibly TODO:
 * Implement a interface for communication between POS <-> SENS

 */

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "printing.h"
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <cassert>
#include <vector>
#include <queue>


class Communication {
	public:
	    Communication();
	    ~Communication();	
		void readLoop();	
		bool getArguments(std::string input, int *pos);



		static void* writeLoop(void *me) {
			// Prepare our context and socket
			zmq::context_t context (1);
			zmq::socket_t socket (context, ZMQ_REP);
			socket.bind ("tcp://*:5556");
			while (true) {
				zmq::message_t request;
				// Wait for next request from client
				socket.recv (&request);
				std::string rpl = std::string(static_cast<char*>(request.data()), request.size());

				DBPL("WRITETHREAD: recieved: " << rpl);
				// Do some 'work'
				sleep(1);
				// Send reply back to client

				//std::string pos = 
				zmq::message_t reply (9);
				memcpy ((void *) reply.data (), "20 40 50", 8);
				DBPL("WRITETHREAD: replied: " << "20 40 50");
				socket.send (reply);
			}
			return 0;
		}	


	private:
		bool addToQueue(std::string);
		void dequeuePos();

		std::string in;
		std::queue<std::string> posQueue;

		bool new_pos_ready;
		//static pthread_mutex_t read_pos_mutex;
		//static pthread_mutex_t write_pos_mutex;
	//	pthread_t read_pos_thread;
    //	pthread_t write_pos_thread;


		std::thread thread1;
};

#endif /* COMMUNICATION_H */