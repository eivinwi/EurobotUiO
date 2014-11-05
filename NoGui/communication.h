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
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include <cassert>

//static void* readLoop(void* me);

class Communication {
	public:
	    Communication();
	    ~Communication();
		
		static void* readLoop(void *me) {
			// Prepare our context and socket
			zmq::context_t context (1);
			zmq::socket_t socket (context, ZMQ_REP);
			socket.bind ("tcp://*:5555");
			while (true) {
				zmq::message_t request;
				// Wait for next request from client
				socket.recv (&request);
				std::string rpl = std::string(static_cast<char*>(request.data()), request.size());

				DBPL("READTHREAD: received" << rpl);
				// Do some 'work'
				sleep(1);
				// Send reply back to client
				zmq::message_t reply (5);
				memcpy ((void *) reply.data (), "ok", 2);
				socket.send (reply);
			}
			return 0;
		}	


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
				zmq::message_t reply (9);
				memcpy ((void *) reply.data (), "20 40 50", 8);
				DBPL("WRITETHREAD: replied: " << "20 40 50");
				socket.send (reply);
			}
			return 0;
		}	

	private:
		std::string in;

};

#endif /* COMMUNICATION_H */