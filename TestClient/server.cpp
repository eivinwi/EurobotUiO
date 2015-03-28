#include <zmq.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n) Sleep(n)
#endif

int main () {
	// Prepare our context and socket
	zmq::context_t context (1);
	zmq::socket_t socket (context, ZMQ_REP);
	socket.bind ("tcp://*:5555");

    auto t_start = std::chrono::high_resolution_clock::now();
	int itr = 0;
	while (true) {
		zmq::message_t request;

		// Wait for next request from client
		socket.recv (&request);
		//std::cout << "Received Hello" << std::endl;

		if(itr == 0) {
			auto t_start = std::chrono::high_resolution_clock::now();			
		}
		
		itr++;

		// Do some 'work'
		//sleep(1);

		// Send reply back to client
		zmq::message_t reply (2);
		memcpy ((void *) reply.data (), "ok", 2);
		socket.send (reply);
	
		if(request.size() == 3) {
			break;
		}
	}
	auto t_end = std::chrono::high_resolution_clock::now();    
	double timepassed = std::chrono::duration<double, std::milli>(t_end-t_start).count();	

	std::cout << "Number of packets : " << itr << std::endl;
	std::cout << "Total time 	  : " << timepassed << " ms" << std::endl;
	std::cout << "Average time      : " << (timepassed / itr) << " ms" << std::endl;


	return 0;

}
