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
#include <pthread.h>
#include <unistd.h>
#include <cassert>
#include <vector>
#include <queue>

//static void* readLoop(void* me);

class Communication {
	public:
	    Communication();
	    ~Communication();
		
		static void* readLoop(struct *rd) {
			// Prepare our context and socket
			zmq::context_t context (1);
			zmq::socket_t socket (context, ZMQ_REP);
			socket.bind ("tcp://*:5555");
			while (true) {
				zmq::message_t request;
				// Wait for next request from client
				socket.recv (&request);
				std::string recv_str = std::string(static_cast<char*>(request.data()), request.size());

				PRINTLINE("READTHREAD: received" << recv_str);
				// Do some 'work'
				
				sleep(1);

				//check arguments
				int pos[3];
				bool isPosition = getArguments(recv_str, pos);
				
				zmq::message_t reply(2);
				if(isPosition) {
					//set position in struct

					if(pthread_mutex_trylock(&read_pos_mutex) != 0) {
						usleep(5000);
						if(pthread_mutex_trylock(&read_pos_mutex) != 0) {
							//error: mutex should not still be locked
						} else {
							rd.x = pos[0];
							rd.y = pos[1];
							rd.rot = pos[2];
						}
					} else {
						rd.x = pos[0];
						rd.y = pos[1];
						rd.rot = pos[2];
					}

					//return OK to client
					memcpy ((void *) reply.data (), "ok", 2);
				} else {
					//recv_str is invalid, return negative to client
					memcpy ((void *) reply.data (), "no", 2);
				}
				socket.send (reply);
			}
			return 0;
		}	




		static bool getArguments(std::string input, int *pos) {
			int i = 0;

		    std::istringstream f(input);
		    std::string s;    
		    while(getline(f, s, ',')) {
		        //std::cout << s << endl;
		        pos[i] = atoi(s.c_str());
		        
		        i++;
		        if(i >= 3) break;
		    }

		    if(i != 2) {
		    	return false;
		    } else {
		        if(abs(pos[0]) > 300) return false;
		        else if(abs(pos[1]) > 300) return false;
		        else if(pos[2] < 0 || pos[2] > 360) return false;
		    	else return true;
		    }
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
		static pthread_mutex_t read_pos_mutex;
		static pthread_mutex_t write_pos_mutex;
		pthread_t read_pos_thread;
    	pthread_t write_pos_thread;
};

#endif /* COMMUNICATION_H */