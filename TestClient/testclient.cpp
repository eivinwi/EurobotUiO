//
// Hello World client in C++
// Connects REQ socket to tcp://localhost:5555
// Sends "Hello" to server, expects "World" back
//
#include <zmq.hpp>
#include <string>
#include <sstream>
#include <iostream>

int main (int argc, char *argv[])
{
	//default testing:
	std::string position = "0,0";

	if(argc < 2) {
		std::cout << "Too few arguments: " << argc <<  ". Either <x y> or  <r>. " << std::endl;
		std::cout << "Example usage: <50 0> for position, <90> for rotation " << std::endl; 
		return 0;
	} 
	else if(argc == 2) {
		//rotation
		if(atoi(argv[1]) > 360 || atoi(argv[1]) < 0) {
			std::cout << "argv[1] not an angle: " << argv[1] << "=" << atoi(argv[1]) << std::endl;
			return 0;
		} else {
			position = argv[1];
		}
	} else if(argc == 3) {
		//position
		if(atoi(argv[1]) > 3000) {
			std::cout << "argv[1] too large: " << argv[1] << "=" << atoi(argv[1]) << std::endl;
			return 0;
		}
		if(abs(atoi(argv[2])) > 3000) {
			std::cout << "argv[2] too large: " << argv[2] << "=" << atoi(argv[2]) << std::endl;
			return 0;
		}

		std::stringstream ss;
		ss << argv[1] << "," << argv[2];
		position = ss.str();
		
	} else {
		std::cout << "Too many arguments: " << argc << ". Either <x y> or  <r>. " << std::endl;
		std::cout << "Example usage: <50 0> for position, <90> for rotation " << std::endl; 		
	}


	// Prepare our context and socket
	zmq::context_t context (1);
	zmq::socket_t socket (context, ZMQ_REQ);
	std::cout << "Connecting to hello world server..." << std::endl;
	socket.connect ("tcp://localhost:5555");
	// Do 10 requests, waiting each time for a response
	for (int request_nbr = 0; request_nbr < 1; request_nbr++) {
		
		zmq::message_t request( position.length()+1 );
	
		memcpy ((void *) request.data (), position.c_str(), position.length());
		std::cout << "Sending position: [" << position << "]" << std::endl;
		socket.send (request);
		// Get the reply.
		zmq::message_t reply;
		socket.recv (&reply);
		std::string recv_str = std::string(static_cast<char*>(request.data()), request.size());
		std::cout << "Received: " << recv_str << std::endl;
	}
	return 0;
}